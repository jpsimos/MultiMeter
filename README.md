# MultiMeter
PZEM-004x sensor reading for the Raspberry Pi. The project objectives are to provide a daemon utility
that is capable of reading from multiple PZEM-004x devices while maintaining a record of the measurements and alarms.
A measurement does not need an alarm condition.

# SQLITE database
The tool uses an SQLITE3 database to store details about each PZEM-004x necessary to communicate.
Additionally, the measurement data and alarms are stored in their own tables and cross referenced for summarization.
A working schema can be found in Database.sql in the repository.

# Alarm
The PZEM-004x supplies an alarm out pin that enables the Raspberry Pi to detect an alarm condition
without needing to use serial. Not all PZEM-004x boards have it. Code modifications are needed if it does not.

# Raspberry Pi
The PZEM-004x alarm pin is active low so the most efficient way to detect an alarm without serial is to
use the Broadcom's edge event register.

# Author Jacob A. Psimos
