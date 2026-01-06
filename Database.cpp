/*
 * Database.cpp
 *
 *  Created on: Oct 14, 2025
 *      Author: jacob a psimos
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sqlite3.h>
#include "Helpers.h"
#include "Database.h"
#include "Logger.h"

using namespace Helpers;

Database& Database::CreateInstance(const std::string& dbPath)
{
	if(nullptr == Database::sm_databaseSingletonPtr)
	{
		Database::sm_databaseSingletonPtr = new Database(dbPath);
	}
	return *Database::sm_databaseSingletonPtr;
}

Database& Database::GetInstance()
{
	if(nullptr == Database::sm_databaseSingletonPtr)
	{
		RUNTIME_ERROR("Must call CreateInstance first.");
	}
	return *Database::sm_databaseSingletonPtr;
}

Database::Database(const std::string& dbPath) :
	m_db(nullptr),
	m_addDeviceStmt(nullptr),
	m_updateDeviceStmt(nullptr),
	m_getDeviceByNameStmt(nullptr),
	m_getDeviceByAddressStmt(nullptr),
	m_recordAlarmStmt(nullptr),
	m_updateAlarmStmt(nullptr),
	m_recordMeasurementStmt(nullptr),
	m_updateMeasurementStmt(nullptr)
{
	int result;

	result = ::sqlite3_open(dbPath.c_str(), &m_db);

	if(SQLITE_OK == result && m_db != nullptr)
	{
		result = ::sqlite3_exec(m_db, "PRAGMA journal_mode = WAL", nullptr, nullptr, nullptr);
		result |= ::sqlite3_busy_timeout(m_db, 1000);

		if(SQLITE_OK != result)
		{
			RUNTIME_ERROR("sqlite_exec/busy_timeout = %d", result);
		}
	}

	if(!m_db)
	{
		RUNTIME_ERROR("sqlite_open = %d", result);
	}
}

Database::~Database()
{
	if(this == Database::sm_databaseSingletonPtr)
	{
		Database::sm_databaseSingletonPtr = nullptr;
	}

	if(m_addDeviceStmt != nullptr)
	{
		::sqlite3_finalize(m_addDeviceStmt);
	}

	if(m_updateDeviceStmt != nullptr)
	{
		::sqlite3_finalize(m_updateDeviceStmt);
	}

	if(m_getDeviceByAddressStmt != nullptr)
	{
		::sqlite3_finalize(m_getDeviceByAddressStmt);
	}

	if(m_recordAlarmStmt != nullptr)
	{
		::sqlite3_finalize(m_recordAlarmStmt);
	}

	if(m_updateAlarmStmt != nullptr)
	{
		::sqlite3_finalize(m_updateAlarmStmt);
	}

	if(m_recordMeasurementStmt != nullptr)
	{
		::sqlite3_finalize(m_recordMeasurementStmt);
	}

	if(m_updateMeasurementStmt != nullptr)
	{
		::sqlite3_finalize(m_updateMeasurementStmt);
	}

	if(m_db != nullptr)
	{
		::sqlite3_close(m_db);
	}
}

bool Database::AddDevice(
	const std::string& name,
	const unsigned char address,
	const double alarm_wattage,
	const uint32_t alarm_pin,
	const uint32_t output_enable_pin,
	unsigned long& new_device_id
)
{
	bool hadError = false;
	bool first = false;
	int result;

	if(nullptr == m_addDeviceStmt)
	{
		first = true;

		result = ::sqlite3_prepare_v2(
			m_db,
			"INSERT INTO Multimeter_Devices(name,address,alarm_wattage,alarm_pin,buffer_pin) "
				"VALUES(?,?,?,?,?)",
			-1,
			&m_addDeviceStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		if(!first)
		{
			result = ::sqlite3_clear_bindings(m_addDeviceStmt);
			result = ::sqlite3_reset(m_addDeviceStmt);
		}

		if(SQLITE_OK != result)
		{
			result = ::sqlite3_bind_text(m_addDeviceStmt, 1, name.c_str(), name.length(), nullptr);
			result |= ::sqlite3_bind_int(m_addDeviceStmt, 2, static_cast<int>(address));
			result |= ::sqlite3_bind_double(m_addDeviceStmt, 3, alarm_wattage);
			result |= ::sqlite3_bind_int(m_addDeviceStmt, 4, static_cast<int>(alarm_pin));
			result |= ::sqlite3_bind_int(m_addDeviceStmt, 5, static_cast<int>(output_enable_pin));

			if(SQLITE_OK == result)
			{
				result = ::sqlite3_step(m_addDeviceStmt);

				if(SQLITE_DONE == result)
				{
					new_device_id = static_cast<unsigned long>(::sqlite3_last_insert_rowid(m_db));
				}
				else
				{
					hadError = true;
				}
			}
			else
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}

bool Database::UpdateDevice(
	const unsigned long device_id,
	const double alarm_wattage
)
{
	bool hadError = false;
	int result;

	if(m_updateDeviceStmt == nullptr)
	{
		result = ::sqlite3_prepare_v2(m_db,
			"UPDATE Multimeter_Devices SET alarm_wattage = ? WHERE id = ?",
			-1,
			&m_updateDeviceStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}
	else
	{
		result = ::sqlite3_clear_bindings(m_updateDeviceStmt);
		result |= ::sqlite3_reset(m_updateDeviceStmt);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		result = ::sqlite3_bind_int64(m_updateDeviceStmt, 1, static_cast<sqlite3_int64>(alarm_wattage));
		result |= ::sqlite3_bind_int64(m_updateDeviceStmt, 2, static_cast<sqlite3_int64>(device_id));

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_step(m_updateDeviceStmt);

			if(SQLITE_DONE != result)
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}

bool Database::GetDeviceByName(const std::string& name, Database::DevicesTableRow& deviceTableRow)
{
	bool hadError = false;
	bool first = false;
	int result;

	if(nullptr == m_getDeviceByNameStmt)
	{
		first = true;

		result = ::sqlite3_prepare_v2(
			m_db,
			"SELECT * FROM Multimeter_Devices WHERE name = ?",
			-1,
			&m_getDeviceByNameStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		if(!first)
		{
			result = ::sqlite3_clear_bindings(m_getDeviceByNameStmt);
			result = ::sqlite3_reset(m_getDeviceByNameStmt);
		}

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_bind_text(m_getDeviceByNameStmt, 1, name.c_str(), name.length(), nullptr);

			if(SQLITE_OK == result)
			{
				result = ::sqlite3_step(m_getDeviceByNameStmt);

				if(SQLITE_ROW == result)
				{
					deviceTableRow.id = static_cast<unsigned int>(::sqlite3_column_int64(m_getDeviceByNameStmt, 0));
					deviceTableRow.name.reserve(::sqlite3_column_bytes(m_getDeviceByNameStmt, 1));
					deviceTableRow.name = reinterpret_cast<const char*>(::sqlite3_column_text(m_getDeviceByNameStmt, 1));
					deviceTableRow.address = static_cast<unsigned char>(::sqlite3_column_int(m_getDeviceByNameStmt, 2));
					deviceTableRow.serial_port.reserve(::sqlite3_column_bytes(m_getDeviceByNameStmt, 3));
					deviceTableRow.serial_port = reinterpret_cast<const char*>(::sqlite3_column_text(m_getDeviceByNameStmt, 3));
					deviceTableRow.alarm_wattage = ::sqlite3_column_double(m_getDeviceByNameStmt, 4);
					deviceTableRow.alarm_pin = static_cast<uint32_t>(::sqlite3_column_int(m_getDeviceByNameStmt, 5));
					deviceTableRow.output_enable_pin = static_cast<uint32_t>(::sqlite3_column_int(m_getDeviceByNameStmt, 6));
					deviceTableRow.m_null = false;

					result = ::sqlite3_step(m_getDeviceByNameStmt);

					if(SQLITE_DONE != result)
					{
						hadError = true;
					}
				}
				else
				{
					deviceTableRow.m_null = true;
				}
			}
			else
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}

bool Database::RecordAlarm(
	const unsigned long device_id,
	const unsigned long measurement_id,
	const time_t epoch_occured,
	const bool alarm,
	const double alarm_wattage,
	unsigned long& row_id
)
{
	bool hadError = false;
	int result;

	if(nullptr == m_recordAlarmStmt)
	{
		result = ::sqlite3_prepare_v2(
			m_db,
			"INSERT INTO Multimeter_Alarms(device_id,measurement_id,epoch_occured,alarm,alarm_wattage) "
				"VALUES(?,?,?,?,?)",
			-1,
			&m_recordAlarmStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}
	else
	{
		result = ::sqlite3_clear_bindings(m_recordAlarmStmt);
		result |= ::sqlite3_reset(m_recordAlarmStmt);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		result = ::sqlite3_bind_int64(m_recordAlarmStmt, 1, static_cast<sqlite3_int64>(device_id));
		result |= measurement_id != 0
			? ::sqlite3_bind_int64(m_recordAlarmStmt, 2, static_cast<sqlite3_int64>(measurement_id))
					: ::sqlite3_bind_null(m_recordAlarmStmt, 2);
		result |= ::sqlite3_bind_int64(m_recordAlarmStmt, 3, static_cast<sqlite3_int64>(epoch_occured));
		result |= ::sqlite3_bind_int(m_recordAlarmStmt, 4, static_cast<int>(alarm));
		result |= ::sqlite3_bind_double(m_recordAlarmStmt, 5, alarm_wattage);

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_step(m_recordAlarmStmt);

			if(SQLITE_DONE == result)
			{
				row_id = static_cast<unsigned int>(::sqlite3_last_insert_rowid(m_db));
			}
			else
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}

bool Database::UpdateAlarm(
	const unsigned long alarm_id,
	const unsigned long measurement_id
)
{
	bool hadError = false;
	int result;

	if(m_updateAlarmStmt == nullptr)
	{
		result = ::sqlite3_prepare_v2(m_db,
			"UPDATE Multimeter_Alarms SET measurement_id = ? WHERE id = ?",
			-1,
			&m_updateAlarmStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}
	else
	{
		result = ::sqlite3_clear_bindings(m_updateAlarmStmt);
		result |= ::sqlite3_reset(m_updateAlarmStmt);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		result = ::sqlite3_bind_int64(m_updateAlarmStmt, 1, static_cast<sqlite3_int64>(measurement_id));
		result |= ::sqlite3_bind_int64(m_updateAlarmStmt, 2, static_cast<sqlite3_int64>(alarm_id));

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_step(m_updateAlarmStmt);

			if(SQLITE_DONE != result)
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}


bool Database::RecordMeasurement(
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
)
{
	bool hadError = false;
	int result;

	if(m_recordMeasurementStmt == nullptr)
	{
		result = ::sqlite3_prepare_v2(m_db,
			"INSERT INTO Multimeter_Measurements"
				"(device_id,alarm_id,epoch_taken,voltage,frequency,current,power,power_factor,energy) "
				"VALUES(?,?,?,?,?,?,?,?,?)",
			-1,
			&m_recordMeasurementStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}
	else
	{
		result = ::sqlite3_clear_bindings(m_recordMeasurementStmt);
		result |= ::sqlite3_reset(m_recordMeasurementStmt);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		result = ::sqlite3_bind_int64(m_recordMeasurementStmt, 1, static_cast<sqlite3_int64>(device_id));
		result |= alarm_id != 0
			? ::sqlite3_bind_int64(m_recordMeasurementStmt, 2, static_cast<sqlite3_int64>(alarm_id))
					: ::sqlite3_bind_null(m_recordMeasurementStmt, 2);
		result |= ::sqlite3_bind_int64(m_recordMeasurementStmt, 3, static_cast<sqlite3_int64>(epoch_taken));
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 4, voltage);
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 5, frequency);
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 6, current);
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 7, power);
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 8, power_factor);
		result |= ::sqlite3_bind_double(m_recordMeasurementStmt, 9, energy);

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_step(m_recordMeasurementStmt);

			if(SQLITE_DONE == result)
			{
				row_id = static_cast<unsigned long>(::sqlite3_last_insert_rowid(m_db));
			}
			else
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}

bool Database::UpdateMeasurement(
	const unsigned long measurement_id,
	const unsigned long alarm_id
)
{
	bool hadError = false;
	int result;

	if(m_updateMeasurementStmt == nullptr)
	{
		result = ::sqlite3_prepare_v2(m_db,
			"UPDATE Multimeter_Measurements SET alarm_id = ? WHERE id = ?",
			-1,
			&m_updateMeasurementStmt,
			nullptr
		);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}
	else
	{
		result = ::sqlite3_clear_bindings(m_updateMeasurementStmt);
		result |= ::sqlite3_reset(m_updateMeasurementStmt);

		if(SQLITE_OK != result)
		{
			hadError = true;
		}
	}

	if(!hadError)
	{
		result = ::sqlite3_bind_int64(m_updateMeasurementStmt, 1, static_cast<sqlite3_int64>(alarm_id));
		result |= ::sqlite3_bind_int64(m_updateMeasurementStmt, 2, static_cast<sqlite3_int64>(measurement_id));

		if(SQLITE_OK == result)
		{
			result = ::sqlite3_step(m_updateMeasurementStmt);

			if(SQLITE_DONE != result)
			{
				hadError = true;
			}
		}
		else
		{
			hadError = true;
		}
	}

	return !hadError;
}


