# MultiMeter

PZEM-004x sensor reading for the Raspberry Pi. The project objectives are to provide a daemon utility
that is capable of reading from multiple PZEM-004x devices while maintaining a record of the measurements and alarms.
A measurement does not need an alarm condition.

**Author:** Jacob A. Psimos

## Features

- Continuous or one-shot reading of one or more PZEM-004T / PZEM-004x devices over UART
- Hardware alarm detection via the active-low alarm pin (Broadcom GPIO edge events) so serial traffic is not required for alarm checks
- SQLite3 storage of device configuration, measurements, and alarms with a ready-made view for joined queries
- Configurable alarm wattage threshold written to the sensor
- Optional output-enable pin control for shared bus or power gating
- Simple command-line interface suitable for daemon or cron use

## Hardware

| Item | Notes |
|------|--------|
| Raspberry Pi | Tested with Broadcom GPIO (edge events on alarm pin) |
| PZEM-004T (or compatible PZEM-004x) | UART Modbus-style protocol; alarm output pin (active-low) |
| Serial | Default `/dev/ttyAMA0` (or any character device) |
| GPIO | Default alarm pin 27, output-enable pin 17 (BCM numbering) |

Not every PZEM-004x board exposes the alarm pin. If yours does not, the alarm-related GPIO code can be disabled or stubbed.

## SQLITE database

The tool uses an SQLITE3 database to store details about each PZEM-004x necessary to communicate.
Additionally, the measurement data and alarms are stored in their own tables and cross referenced for summarization.
A working schema can be found in `Database.sql` in the repository.

Key tables:

- `Multimeter_Devices` – name, Modbus address, serial port, alarm wattage, GPIO pins
- `Multimeter_Measurements` – voltage, current, power, energy, frequency, power factor, timestamp
- `Multimeter_Alarms` – alarm events linked to a measurement and the wattage threshold that was active
- `Measurements_And_Alarms` view – convenient LEFT JOIN for reporting or charting

Default database path: `/var/db/multimeter.db` (override with `--db`).

## Alarm

The PZEM-004x supplies an alarm out pin that enables the Raspberry Pi to detect an alarm condition
without needing to use serial. Not all PZEM-004x boards have it. Code modifications are needed if it does not.

The PZEM-004x alarm pin is active low so the most efficient way to detect an alarm without serial is to
use the Broadcom's edge event register (`PinController`).

## Building

Dependencies:

- g++ with C++20 support
- `libsqlite3-dev`

```bash
make
# produces the MultiMeter binary
```

Clean:

```bash
make clean
```

## Usage

```text
MultiMeter [options]
```

Common options (see `Main.cpp` for the full set):

| Option | Description | Default |
|--------|-------------|---------|
| `--name <name>` | Device name in the database | `default_device` |
| `--serial <path>` | Serial device | `/dev/ttyAMA0` |
| `--db <path>` | SQLite database | `/var/db/multimeter.db` |
| `--address <1-247>` | Modbus address | `1` |
| `--alarm-pin <bcm>` | Alarm input GPIO | `27` |
| `--oe-pin <bcm>` | Output-enable GPIO | `17` |
| `--interval <seconds>` | Polling interval (0 = single shot) | `0` |
| `--alarm <watts>` | Alarm threshold | `1000` |
| `--create` | Insert the device into the DB if missing | |
| `--no-db` | Do not open or write the database | |
| `--verbose` | Print readings to stdout | |
| `--help` | Show help | |

Examples:

```bash
# Create a device entry and take one reading
./MultiMeter --create --name kitchen --address 1 --alarm 1500 --verbose

# Run as a daemon, sampling every 10 seconds
./MultiMeter --name kitchen --interval 10

# Single reading without touching the database
./MultiMeter --name kitchen --no-db --verbose
```

Exit status is a bit-mask of error conditions defined in `Main.cpp` (bad arguments, serial failure, DB errors, signal, etc.).

## LineChart.js — Custom Lightweight Visualization

The measurement history stored in SQLite is intended to be visualized with a purpose-built **LineChart.js**.

This is a deliberately minimal, dependency-free line-chart implementation written for the MultiMeter data model. It is custom rather than a wrapper around Chart.js, D3, or Highcharts for two reasons:

1. **Lightweight** – No external libraries, no CSS frameworks, and a tiny footprint. It can be served from a static directory on the Pi or embedded in a small status page without pulling in hundreds of kilobytes of third-party code.
2. **Simplicity** – The API is reduced to the essentials needed for time-series power data: an array of `{t, v}` points (epoch + value), a canvas element, and a handful of options (color, fill, min/max, label formatter). There are no plugins, no animation system, and no configuration objects that require reading a manual.

Typical usage with data exported from the `Measurements_And_Alarms` view:

```javascript
const chart = new LineChart(canvas, {
  color: "#1e88e5",
  fill: "rgba(30,136,229,0.15)",
  yLabel: "W",
  formatX: (epoch) => new Date(epoch * 1000).toLocaleTimeString()
});

chart.setData([
  { t: 1697000000, v: 42.3 },
  { t: 1697000060, v: 45.1 },
  // … rows from SQLite
]);
```

The chart draws directly to a `<canvas>`, handles high-DPI scaling, and only re-renders when `setData` is called. Because the MultiMeter daemon already timestamps every sample, the x-axis is a simple linear scale of Unix epochs; no date library is required.

This design keeps the visualization layer as easy to maintain as the C++ daemon itself—small, obvious, and free of unnecessary abstractions.

## Project Layout

```text
Main.cpp              entry point, argument parsing, timer loop
MultiMeter.cpp/.h     serial protocol, measurement parsing, alarm set/read
Database.cpp/.h       SQLite wrappers and schema access
PinController.cpp/.h  BCM GPIO edge detection for the alarm pin
Logger.cpp/.h         simple logging
Helpers.cpp/.h        CRC, threading timer, utilities
Database.sql          reference schema
Makefile
```

## License / Contact

Author: Jacob A. Psimos  
Contact: jpsimos@gmail.com
