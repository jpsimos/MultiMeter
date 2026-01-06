PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "Multimeter_Alarms" (
	"id"	INTEGER NOT NULL UNIQUE,
	"device_id"	INTEGER NOT NULL,
	"measurement_id"	INTEGER DEFAULT NULL,
	"epoch_occured"	INTEGER(8) NOT NULL DEFAULT 'strftime("%s", "now")',
	"alarm"	INTEGER(1) NOT NULL,
	"alarm_wattage"	REAL NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "Multimeter_Measurements" (
	"id"	INTEGER NOT NULL UNIQUE,
	"device_id"	INTEGER NOT NULL,
	"alarm_id"	INTEGER DEFAULT NULL,
	"epoch_taken"	INTEGER(8) NOT NULL DEFAULT 'strftime("%s", "now")',
	"voltage"	REAL NOT NULL,
	"frequency"	REAL NOT NULL,
	"current"	REAL NOT NULL,
	"power"	REAL NOT NULL,
	"power_factor"	REAL NOT NULL,
	"energy"	REAL NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "Multimeter_Devices" (
	"id"	INTEGER NOT NULL UNIQUE,
	"name"	TEXT NOT NULL UNIQUE,
	"address"	INTEGER(1) NOT NULL UNIQUE,
	"serial_port"	TEXT,
	"alarm_wattage"	REAL NOT NULL,
	"alarm_pin"	INTEGER NOT NULL UNIQUE,
	"output_enable_pin"	INTEGER NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT)
);
INSERT INTO Multimeter_Devices VALUES(1,'primary_transformer',1,'/dev/ttyAMA0',500.0,27,17);
PRAGMA writable_schema=ON;
CREATE TABLE IF NOT EXISTS sqlite_sequence(name,seq);
DELETE FROM sqlite_sequence;
INSERT INTO sqlite_sequence VALUES('Multimeter_Alarms','1');
INSERT INTO sqlite_sequence VALUES('Multimeter_Measurements','1');
INSERT INTO sqlite_sequence VALUES('Multimeter_Devices',1);
CREATE VIEW Measurements_And_Alarms AS SELECT
	m.id,
	m.device_id,
	a.id AS alarm_id,
	m.epoch_taken,
	m.voltage,
	m.frequency,
	m.current,
	m.power,
	m.power_factor,
	m.energy,
	a.epoch_occured,
	a.alarm_wattage
FROM Multimeter_Measurements m
LEFT JOIN Multimeter_Alarms a
ON m.id = a.measurement_id ORDER BY m.id DESC;
PRAGMA writable_schema=OFF;
COMMIT;
