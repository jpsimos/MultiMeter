/*
 * Database.h
 *
 *  Created on: Oct 14, 2025
 *      Author: jacob a psimos
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <unordered_map>
#include <string>
#include <limits>
#include <sqlite3.h>

// Defines
#define SQL_NULL 0

class Database {
public:

// ⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖
	class TableRow
	{
	public:
		TableRow();
		TableRow(const TableRow& copyFrom);
		bool m_null;
		bool m_changed;
	};

	class AlarmsTableRow : public TableRow
	{
		public:
			AlarmsTableRow();
			AlarmsTableRow(const AlarmsTableRow& copyFrom);
			unsigned long id;
			unsigned long device_id;
			unsigned long measurement_id;
			time_t epoch_occured;
			bool alarm;
			double alarm_wattage;
	};

	class DevicesTableRow : public TableRow
	{
		public:
			DevicesTableRow();
			DevicesTableRow(const DevicesTableRow& copyFrom);
			unsigned long id;
			std::string name;
			unsigned char address;
			std::string serial_port;
			double alarm_wattage;
			uint32_t alarm_pin;
			uint32_t output_enable_pin;
	};

	class MeasurementsTableRow : public TableRow
	{
	public:
		MeasurementsTableRow();
		MeasurementsTableRow(const MeasurementsTableRow& copyFrom);
		unsigned long id;
		unsigned long device_id;
		unsigned long alarm_id;
		time_t epoch_taken;
		double voltage;
		double frequency;
		double current;
		double power;
		double power_factor;
		double energy;
	};

// ⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖

private:
	inline static Database* sm_databaseSingletonPtr = nullptr;

public:
	static Database& CreateInstance(const std::string& dbPath);
	static Database& GetInstance();

private:
	Database(const std::string& dbPath);
	~Database();

public:
	bool AddDevice(
		const std::string& name,
		const unsigned char address,
		const double alarm_wattage,
		const uint32_t alarm_pin,
		const uint32_t output_enable_pin,
		unsigned long& new_device_id
	);
	bool UpdateDevice(
		const unsigned long device_id,
		const double alarm_wattage
	);
	bool GetDeviceByName(const std::string& name, DevicesTableRow& deviceTableRow);
	bool RecordAlarm(
		const unsigned long device_id,
		const unsigned long measurement_id,
		const time_t epoch_occured,
		const bool alarm,
		const double alarm_wattage,
		unsigned long& row_id
	);
	bool UpdateAlarm(
		const unsigned long alarm_id,
		const unsigned long measurement_id
	);
	bool RecordMeasurement(
		const unsigned long device_id,
		const unsigned long alarm_id,
		const time_t epoch_taken,
		const double voltage,
		const double frequency,
		const double current,
		const double power,
		const double power_factor,
		const double energy,
		unsigned long& row_id
	);
	bool UpdateMeasurement(
		const unsigned long measurement_id,
		const unsigned long alarm_id
	);

private:
	sqlite3* m_db;
	sqlite3_stmt* m_addDeviceStmt;
	sqlite3_stmt* m_updateDeviceStmt;
	sqlite3_stmt* m_getDeviceByNameStmt;
	sqlite3_stmt* m_getDeviceByAddressStmt;
	sqlite3_stmt* m_recordAlarmStmt;
	sqlite3_stmt* m_updateAlarmStmt;
	sqlite3_stmt* m_recordMeasurementStmt;
	sqlite3_stmt* m_updateMeasurementStmt;
};

// ⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖

inline Database::TableRow::TableRow()
	: m_null(true),
	  m_changed(false)
{
}

inline Database::TableRow::TableRow(const Database::TableRow& copyFrom)
{
	m_null = copyFrom.m_null;
	m_changed = copyFrom.m_changed;
}

inline Database::AlarmsTableRow::AlarmsTableRow()
	: Database::TableRow(),
	  id(SQL_NULL),
	  device_id(SQL_NULL),
	  measurement_id(SQL_NULL),
	  epoch_occured(0),
	  alarm(false),
	  alarm_wattage(NAN)
{
}

inline Database::AlarmsTableRow::AlarmsTableRow(const Database::AlarmsTableRow& copyFrom)
	: Database::TableRow(copyFrom)
{
	id = copyFrom.id;
	device_id = copyFrom.device_id;
	measurement_id = copyFrom.measurement_id;
	epoch_occured = copyFrom.epoch_occured;
	alarm = copyFrom.alarm;
	alarm_wattage = copyFrom.alarm_wattage;
}

inline Database::DevicesTableRow::DevicesTableRow()
	: Database::TableRow(),
	  id(SQL_NULL),
	  name(),
	  address(0),
	  serial_port(),
	  alarm_wattage(NAN),
	  alarm_pin(0),
	  output_enable_pin(0)
{
}

inline Database::DevicesTableRow::DevicesTableRow(const Database::DevicesTableRow& copyFrom)
	: Database::TableRow(copyFrom)
{
	id = copyFrom.id;
	name = copyFrom.name;
	address = copyFrom.address;
	serial_port = copyFrom.serial_port;
	alarm_wattage = copyFrom.alarm_wattage;
	alarm_pin = copyFrom.alarm_pin;
	output_enable_pin = copyFrom.output_enable_pin;
}

inline Database::MeasurementsTableRow::MeasurementsTableRow()
	: Database::TableRow(),
	  id(SQL_NULL),
	  device_id(SQL_NULL),
	  alarm_id(SQL_NULL),
	  epoch_taken(0),
	  voltage(NAN),
	  frequency(NAN),
	  current(NAN),
	  power(NAN),
	  power_factor(NAN),
	  energy(NAN)
{
}

inline Database::MeasurementsTableRow::MeasurementsTableRow(const Database::MeasurementsTableRow& copyFrom)
	: Database::TableRow(copyFrom)
{
	id = copyFrom.id;
	device_id = copyFrom.device_id;
	alarm_id = copyFrom.alarm_id;
	epoch_taken = copyFrom.epoch_taken;
	voltage = copyFrom.voltage;
	frequency = copyFrom.frequency;
	current = copyFrom.current;
	power = copyFrom.power;
	power_factor = copyFrom.power_factor;
	energy = copyFrom.energy;
}

#endif /* DATABASE_H_ */
