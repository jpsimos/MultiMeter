/*
 * MultiMeter.h
 *
 *      Author: jacob a psimos
 */

#ifndef SRC_MULTIMETER_H_
#define SRC_MULTIMETER_H_

#include <mutex>
#include <memory>
#include <string>
#include "PinController.h"
#include "Database.h"
#include "Helpers.h"

using namespace Helpers;

// ⩽ PZEM Registers ⩾
#define REG_VOLTAGE     0x0000
#define REG_CURRENT_L   0x0001
#define REG_CURRENT_H   0X0002
#define REG_POWER_L     0x0003
#define REG_POWER_H     0x0004
#define REG_ENERGY_L    0x0005
#define REG_ENERGY_H    0x0006
#define REG_FREQUENCY   0x0007
#define REG_PF          0x0008
#define REG_ALARM       0x0009
#define CMD_RHR         0x03
#define CMD_RIR         0X04
#define CMD_WSR         0x06
#define CMD_CAL         0x41
#define CMD_REST        0x42
#define WREG_ALARM_THR   0x0001
#define WREG_ADDR        0x0002

// ⩽ PZEM Register Limits ⩾
#define ALARM_MAX      50000
#define ALARM_MIN      1
#define ADDRESS_MAX    0xF7
#define ADDRESS_MIN    1

class MultiMeter
{
public:
	MultiMeter(
		const std::string& name
	);
	~MultiMeter();
	bool HadError();
	bool SetPowerAlarm(const double wattage);
	bool SetAddress(const unsigned char address);
	bool ResetEnergy();
	bool Update();
	bool Record();
	double GetVoltage();
	double GetFrequency();
	double GetCurrent();
	double GetPower();
	double GetPowerFactor();
	double GetEnergy();
	std::string ToJsonString();

private:
	bool ReadAddress(unsigned char& address);
	bool Command(
		const unsigned char command,
		const unsigned short regAddress,
		unsigned short value,
		const bool verify
	);
	bool ReadResponse(unsigned char response[], const size_t length);
	size_t SerialWrite(const unsigned char data[], const size_t length);
	size_t SerialRead(unsigned char data[], const size_t length);
	bool IsSerialAvailable();
	bool ConfigureSerialPort();

private:
	Database::DevicesTableRow m_deviceTableRow;
	Database::MeasurementsTableRow m_measurementTableRow;
	Database::AlarmsTableRow m_alarmTableRow;
	std::unique_ptr<PinController> m_pinControllerPtr;
	std::mutex m_mutex;
	int m_serialFd;
	bool m_hadError;
};

#endif /* SRC_MULTIMETER_H_ */
