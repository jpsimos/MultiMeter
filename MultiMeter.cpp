/*
 * MultiMeter.cpp
 *
 *      Author: jacob a psimos
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <mutex>
#include <string>
#include <sstream>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MultiMeter.h"
#include "PinController.h"
#include "Logger.h"
#include "Helpers.h"

using namespace Helpers;

MultiMeter::MultiMeter(
	const std::string& name
) :
	m_deviceTableRow(),
	m_measurementTableRow(),
	m_alarmTableRow(),
	m_pinControllerPtr(),
	m_mutex(),
	m_serialFd(-1),
	m_hadError(false)
{
	bool hadError = false;

	hadError = !Database::GetInstance().GetDeviceByName(name, m_deviceTableRow);

	if(!hadError)
	{
		if(!ConfigureSerialPort())
		{
			RUNTIME_ERROR("Unable to configure serial port %s", m_deviceTableRow.serial_port.c_str());
			hadError = true;
		}

		if(!hadError)
		{
			m_pinControllerPtr.reset(new PinController(
				m_deviceTableRow.alarm_pin,
				m_deviceTableRow.output_enable_pin
			));
		}

		if(!hadError)
		{
			m_pinControllerPtr->SetOutputEnable(true);

			hadError = !SetPowerAlarm(
				static_cast<unsigned short>(std::round(m_deviceTableRow.alarm_wattage)));

			m_pinControllerPtr->SetOutputEnable(false);
		}
	}
	else
	{
		RUNTIME_ERROR("Unable to locate device by name.");
		hadError = true;
	}
}

MultiMeter::~MultiMeter()
{
	if(m_serialFd != -1)
	{
		::close(m_serialFd);
	}
}

bool MultiMeter::HadError()
{
	return m_hadError;
}

bool MultiMeter::SetPowerAlarm(const double wattage)
{
	bool hadError = true;

	if(wattage >= ALARM_MIN && wattage <= ALARM_MAX)
	{
		hadError = !Command(CMD_WSR, WREG_ALARM_THR, wattage, true);

		if(!hadError)
		{
			hadError = !Database::GetInstance().UpdateDevice(
				m_deviceTableRow.id,
				wattage
			);

			if(!hadError)
			{
				m_deviceTableRow.alarm_wattage = wattage;
			}
		}
	}

	return !hadError;
}

bool MultiMeter::SetAddress(const unsigned char address)
{
	bool hadError = true;

	if(address >= ADDRESS_MIN && address <= ADDRESS_MAX)
	{
		hadError = !Command(CMD_WSR, WREG_ADDR, address, true);

		if(!hadError)
		{
			RUNTIME_ERROR("Not implemented.");
		}
	}

	return !hadError;
}

bool MultiMeter::ReadAddress(unsigned char& address)
{
	bool hadError = false;
	unsigned char response[7];
	unsigned int address32;

	std::memset(&response[0], 0, sizeof(response));

	hadError = !Command(CMD_RHR, WREG_ADDR, 0x01, false);

	if(!hadError)
	{
		hadError = !ReadResponse(&response[0], sizeof(response));

		if(!hadError)
		{
			address32 = (static_cast<unsigned int>(
				response[3]) << 8) | static_cast<unsigned int>(response[4]);

			address = static_cast<unsigned char>(address32 & 0xFF);
		}
	}

	return !hadError;
}

bool MultiMeter::ResetEnergy()
{
	bool hadError = false;
	unsigned char buffer[4];
	unsigned char reply[5];
	uint16_t value;

	std::memset(&buffer[0], 0, sizeof(buffer));
	std::memset(&reply[0], 0, sizeof(reply));

	buffer[0] = m_deviceTableRow.address;
	buffer[1] = CMD_REST;
	value = Checksum::CRC16::Calculate(buffer, 4);
	buffer[2] = static_cast<unsigned char>(value & 0xFF);
	buffer[3] = static_cast<unsigned char>((value >> 8) & 0xFF);

	if(sizeof(buffer) == SerialWrite(&buffer[0], sizeof(buffer)))
	{
		hadError = !ReadResponse(&reply[0], sizeof(reply));
	}
	else
	{
		hadError = true;
	}

	if(hadError)
	{
		m_hadError = true;
	}

	return !hadError;
}

bool MultiMeter::Update()
{
	bool hadError;
	unsigned char in[25];
	unsigned int conv;
	time_t epochTaken;

	std::memset(&in[0], 0, sizeof(in));
	hadError = !m_pinControllerPtr->Update();

	if(!hadError)
	{
		if(m_pinControllerPtr->GetMisconfigurationEvent())
		{
			LOG(LOG_WARNING, "GPIO misconfiguration event.");
			m_pinControllerPtr->ClearMisconfigurationEvent();
		}
	}
	else
	{
		LOG(LOG_ERR, "Error updating pin controller.");
	}

	if(!hadError)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_pinControllerPtr->SetOutputEnable(true);
		hadError = !Command(CMD_RIR, 0x00, 0x0A, false);

		if(!hadError)
		{
			hadError = !ReadResponse(in, sizeof(in));
			m_pinControllerPtr->SetOutputEnable(false);

			if(!hadError)
			{
				epochTaken = std::time(nullptr);

				conv = ((unsigned int)in[3] << 8) | (unsigned int)in[4];
				m_measurementTableRow.voltage = (double)conv / 10.0;
				hadError = hadError || std::isnan(m_measurementTableRow.voltage);

				conv = (unsigned int)in[5] << 8;
				conv |= (unsigned int)in[6];
				conv |= (unsigned int)in[7] << 24;
				conv |= (unsigned int)in[8] << 16;
				m_measurementTableRow.current = (double)conv / 1000.0;
				hadError = hadError || std::isnan(m_measurementTableRow.current);

				conv = (unsigned int)in[9] << 8;
				conv |= (unsigned int)in[10];
				conv |= (unsigned int)in[11] << 24;
				conv |= (unsigned int)in[12] << 16;
				m_measurementTableRow.power = (double)conv / 10.0;
				hadError = hadError || std::isnan(m_measurementTableRow.power);

				conv = (unsigned int)in[13] << 8;
				conv |= (unsigned int)in[14];
				conv |= (unsigned int)in[15] << 24;
				conv |= (unsigned int)in[16] << 16;
				m_measurementTableRow.energy = (double)conv / 1000.0;
				hadError = hadError || std::isnan(m_measurementTableRow.energy);

				conv = ((unsigned int)in[17] << 8) | (unsigned int)in[18];
				m_measurementTableRow.frequency = (double)conv / 10.0;
				hadError = hadError || std::isnan(m_measurementTableRow.frequency);

				conv = ((unsigned int)in[19] << 8) | (unsigned int)in[20];
				m_measurementTableRow.power_factor = (double)conv / 100.0;
				hadError = hadError || std::isnan(m_measurementTableRow.power_factor);

				conv = ((unsigned int)in[21] << 8) | (unsigned int)in[22];

				if(!hadError)
				{
					if(conv != 0 || m_pinControllerPtr->GetAlarmEvent())
					{
						std::printf("AlarmEvent=%d, Alarm=%d\n", static_cast<int>(!!conv), static_cast<int>(m_pinControllerPtr->GetAlarmEvent()));

						m_alarmTableRow.alarm = static_cast<bool>(conv);
						m_alarmTableRow.alarm_wattage = m_deviceTableRow.alarm_wattage;
						m_alarmTableRow.epoch_occured = epochTaken;
						m_alarmTableRow.m_changed = true;
						m_alarmTableRow.m_null = false;
						m_pinControllerPtr->ClearAlarmEvent();
					}

					m_measurementTableRow.epoch_taken = epochTaken;
					m_measurementTableRow.m_changed = true;
					m_measurementTableRow.m_null = false;
				}
				else
				{
					LOG(LOG_ERR, "Error parsing response.");
				}
			}
		}
		else
		{
			LOG(LOG_ERR, "Error processing command.");
		}
	}

	m_hadError = hadError;

	return !hadError;
}

bool MultiMeter::Record()
{
	bool hadError = false;
	unsigned long measurementRowId;
	unsigned long alarmRowId;

	hadError = !Database::GetInstance().RecordMeasurement(
		m_deviceTableRow.id,
		SQL_NULL,
		m_measurementTableRow.epoch_taken,
		m_measurementTableRow.voltage,
		m_measurementTableRow.frequency,
		m_measurementTableRow.current,
		m_measurementTableRow.power,
		m_measurementTableRow.power_factor,
		m_measurementTableRow.energy,
		measurementRowId
	);

	if(!hadError)
	{
		if(m_alarmTableRow.alarm != 0)
		{
			hadError = !Database::GetInstance().RecordAlarm(
				m_deviceTableRow.id,
				measurementRowId,
				m_measurementTableRow.epoch_taken,
				m_alarmTableRow.alarm,
				m_alarmTableRow.alarm_wattage,
				alarmRowId
			);

			if(!hadError)
			{
				hadError = !Database::GetInstance().UpdateMeasurement(measurementRowId, alarmRowId);
				m_pinControllerPtr->ClearAlarmEvent();
			}
		}
	}

	return !hadError;
}

double MultiMeter::GetVoltage()
{
	return m_measurementTableRow.voltage;
}

double MultiMeter::GetFrequency()
{
	return m_measurementTableRow.frequency;
}

double MultiMeter::GetCurrent()
{
	return m_measurementTableRow.current;
}

double MultiMeter::GetPower()
{
	return m_measurementTableRow.power;
}

double MultiMeter::GetPowerFactor()
{
	return m_measurementTableRow.power_factor;
}

double MultiMeter::GetEnergy()
{
	return m_measurementTableRow.energy;
}

std::string MultiMeter::ToJsonString()
{
	std::ostringstream ostr;

	if(!m_hadError)
	{
		ostr << "{" << std::setprecision(5) <<
			"\"voltage\":" << m_measurementTableRow.voltage << ","
			"\"frequency\":" << m_measurementTableRow.frequency << ","
			"\"current\":" << m_measurementTableRow.current << ","
			"\"power\":" << m_measurementTableRow.power << ","
			"\"power_factor\":" << m_measurementTableRow.power_factor << ","
			"\"energy\":" << m_measurementTableRow.energy << ","
			"\"alarm\":" << m_alarmTableRow.alarm << ","
			"\"alarm_wattage\":" << m_alarmTableRow.alarm_wattage
		<< "}";
	}

	return ostr.str();
}

bool MultiMeter::Command(
	const unsigned char command,
	const unsigned short regAddress,
	unsigned short value,
	const bool verify
)
{
	bool hadError = false;
	unsigned char out[8];
	unsigned char in[8];
	size_t numSent;

	out[0] = m_deviceTableRow.address;
	out[1] = command;
	out[2] = (unsigned char)((regAddress >> 8) & 0xFF);
	out[3] = (unsigned char)(regAddress & 0xFF);
	out[4] = (unsigned char)((value >> 8) & 0xFF);
	out[5] = (unsigned char)(value & 0xFF);
	value = Checksum::CRC16::Calculate(out, 6);
	out[6] = (unsigned char)(value & 0xFF);
	out[7] = (unsigned char)((value >> 8) & 0xFF);

	std::memset(&in[0], 0, sizeof(in));

	numSent = SerialWrite(out, 8);

	if(8 != numSent)
	{
		hadError = true;
	}

	if(!hadError && verify)
	{
		hadError = !ReadResponse(in, 8);
	}

	return !hadError;
}

bool MultiMeter::ReadResponse(unsigned char response[], const size_t length)
{
	bool hadError = true;
	size_t numRead;
	uint16_t checksum;
	uint16_t calculatedChecksum;

	if(length > 2)
	{
		numRead = SerialRead(response, length);

		if(length == numRead)
		{
			checksum = (uint16_t)response[numRead - 2] | ((uint16_t)response[numRead - 1] << 8);
			calculatedChecksum = Checksum::CRC16::Calculate(response, numRead - 2);

			if(checksum == calculatedChecksum)
			{
				hadError = false;
			}
		}
	}

	return !hadError;
}

size_t MultiMeter::SerialWrite(const unsigned char data[], const size_t length)
{
	size_t numWrote = 0;

	if(m_serialFd != -1)
	{
		numWrote = ::write(m_serialFd, data, length);

		if(length == numWrote)
		{
			::tcdrain(m_serialFd);
		}
	}

	return numWrote;
}

size_t MultiMeter::SerialRead(unsigned char data[], const size_t length)
{
	size_t numRead = 0;
	size_t chunkSize;
	size_t numJustRead;
	Time::Stopwatch timeout;

	if(m_serialFd != -1)
	{
		do
		{
			if(numRead < length)
			{
				chunkSize = std::min(static_cast<size_t>(1024), length - numRead);
				numJustRead = ::read(m_serialFd, &data[numRead], chunkSize);

				if(numJustRead > 0)
				{
					numRead += numJustRead;
					timeout = 0;
				}
			}

			if(!(numRead < length))
			{
				break;
			}
		}
		while(timeout < 0.15);
	}

	return numRead;
}

bool MultiMeter::IsSerialAvailable()
{
	struct pollfd fds[1];
	int result;

	fds[0].fd = m_serialFd;
	fds[0].events = POLLIN | POLLPRI;
	fds[0].revents = 0;

	result = ::poll(&fds[0], 1, 10);

	return static_cast<bool>(
		result > 0
			&& ((fds[0].revents & (POLLIN | POLLPRI)) != 0)
	);
}

bool MultiMeter::ConfigureSerialPort()
{
	struct termios tty;

	errno = 0;

	if(m_serialFd != -1)
	{
		::close(m_serialFd);
		m_serialFd = -1;
	}

	m_serialFd = ::open(m_deviceTableRow.serial_port.c_str(), O_RDWR | O_NOCTTY);

	if(m_serialFd != -1)
	{
		::tcgetattr(m_serialFd, &tty);
		::cfmakeraw(&tty);
		::cfsetospeed(&tty, B9600);
		::cfsetispeed(&tty, B9600);
		tty.c_cflag &= ~CRTSCTS;
		tty.c_cflag |= CS8 | CREAD | CLOCAL;
		tty.c_cc[VMIN] = 0;
		tty.c_cc[VTIME] = 10;
		::tcsetattr(m_serialFd, TCSANOW, &tty);
		::tcflush(m_serialFd, TCIFLUSH);
	}

	return static_cast<bool>(m_serialFd != -1);
}

