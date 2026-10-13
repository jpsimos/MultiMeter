# MultiMeter

PZEM-004x sensor reading for the Raspberry Pi. The project provides a daemon utility capable of reading from multiple PZEM-004x devices while maintaining a record of the measurements and alarms. A measurement does not need an alarm condition.

**Author:** Jacob A. Psimos

## Features

- Continuous or one-shot reading of one or more PZEM-004T / PZEM-004x devices over UART
- Hardware alarm detection via the active-low alarm pin (Broadcom GPIO edge events) so serial traffic is not required for alarm checks
- SQLite3 storage of device configuration, measurements, and alarms with a ready-made view for joined queries
- Configurable alarm wattage threshold written to the sensor
- Optional output-enable pin control for shared bus or power gating
- Simple command-line interface suitable for daemon or cron use
- Lightweight web interface (`universe/`) for live viewing of measurements

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

The PZEM-004x supplies an alarm out pin that enables the Raspberry Pi to detect an alarm condition without needing to use serial. Not all PZEM-004x boards have it. Code modifications are needed if it does not.

The PZEM-004x alarm pin is active low so the most efficient way to detect an alarm without serial is to use the Broadcom's edge event register (`PinController`).

## Building the daemon

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

## Web Interface (`universe/`)

The `universe/` directory contains a lightweight PHP + HTML5 + JavaScript web front-end that reads the SQLite database and displays live measurements and charts.

### Requirements

The web interface **must be hosted on a capable HTTP server** such as Apache (with `mod_php` or PHP-FPM), nginx with PHP-FPM, or any other web server that can execute PHP and serve static files. It is not a standalone executable—place the `universe/` tree under the server’s document root (or a subdirectory) and ensure the PHP process can read the MultiMeter SQLite database.

### Overview

- Entry point: `universe/index.php`
- Tab-based UI (Main, Multimeter, System)
- AJAX polling via `universe/invoke/pzemInvoker.php` for live data
- Uses jQuery + Bootstrap for minimal chrome
- Live multi-series chart powered by the custom `LineChart.js`

Typical setup on Apache:

```apache
# Example virtual host snippet
DocumentRoot /var/www/multimeter/universe
<Directory /var/www/multimeter/universe>
    AllowOverride All
    Require all granted
</Directory>
```

Ensure the PHP user has read access to the SQLite database file and that the database path used by the invoker matches the one written by the daemon.

## LineChart.js — Custom Lightweight Visualization

The web interface visualizes measurement history with a purpose-built **`universe/js/LineChart.js`**.

This is a deliberately minimal, dependency-free (beyond the page’s existing jQuery) canvas line-chart written specifically for MultiMeter data. It is custom rather than a wrapper around Chart.js, D3, or Highcharts for two reasons:

1. **Lightweight** – ~4.8 KB of plain JavaScript. No charting library, no plugins, no animation system, and no large dependency tree. It draws directly to a `<canvas>` and only re-renders when data or size changes.
2. **Simplicity** – The public surface is intentionally small:
   - `new LineChart(canvasId)`
   - `SetData(seriesName, numberArray, color)` — stores a series and immediately refreshes
   - Automatic multi-series support (Wattage, Voltage, Current, etc.)
   - Debounced window-resize handling (75 ms) so the chart stays responsive without thrashing the CPU
   - Simple axis drawing and proportional scaling to the observed maximum

Typical use (as seen in the Multimeter tab):

```javascript
this.measurementsChart = new LineChart('measurementsChart');

// later, after fetching rows from the database:
this.measurementsChart.SetData('Wattage', watts, 'blue');
this.measurementsChart.SetData('Voltage', volts, 'red');
this.measurementsChart.SetData('Current', current, 'green');
```

The chart scales each series relative to its own maximum, draws connected line segments, and keeps the drawing code obvious. Because the MultiMeter daemon already timestamps every sample, the x-axis is simply sequential points—no date library or complex scale system is required.

This design keeps the visualization layer as easy to maintain as the C++ daemon itself: small, obvious, and free of unnecessary abstractions.

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

universe/             web interface (host on Apache or equivalent)
  index.php           front-end entry point
  js/LineChart.js     custom lightweight multi-series line chart
  js/                 supporting client scripts (Tab, Clock, MessageBox, …)
  include/            PHP back-end and tab definitions
  invoke/             AJAX endpoints
  stylesheets/        CSS
```

## License / Contact

Author: Jacob A. Psimos  
Contact: jpsimos@gmail.com
