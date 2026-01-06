/*
 * PinController.h
 *
 *  Created on: Dec 11, 2025
 *      Author: jacob a psimos
 */

#ifndef PINCONTROLLER_H_
#define PINCONTROLLER_H_

#include <atomic>
#include <cstdint>
#include "Helpers.h"

using namespace Helpers;

// Gpio Level Defines
#define LEVEL_HIGH   0x01
#define LEVEL_LOW    0x00

// Pullup/Pulldown Resistor Defines
#define PULL_UP         0x02
#define PULL_DOWN       0x01
#define PULL_NONE       0x00
#define PULL_NOCHANGE   0xFF

// Gpio Function Defines (some of them)
#define FUNCTION_OUTPUT   0x01
#define FUNCTION_INPUT    0x00

// Gpio Register Offset Defines
#define GPFSEL0       0x00
#define GPSET0        0x1C
#define GPSET1        0x20
#define GPCLR0        0x28
#define GPCLR1        0x2C
#define GPLEV0        0x34
#define GPLEV1        0x38
#define GPEDS0        0x40
#define GPEDS1        0x41
#define GPREN0        0x4C
#define GPREN1        0x50
#define GPFEN0        0x58
#define GPFEN1        0x5C
#define GPHEN0        0x64
#define GPHEN1        0x68
#define GPLEN0        0x70
#define GPLEN1        0x74
#define GPAREN0       0x7C
#define GPAREN1       0x80
#define GPAFEN0       0x88
#define GPAFEN1       0x8C
#define GPPUD         0x94
#define GPPUDCLK0     0x98
#define GPPUDCLK1     0x9C

class PinController
{
public:
	PinController(
		const uint32_t alarmPin,
		const uint32_t outputEnablePin
	);
	~PinController();
	bool SetOutputEnable(const bool enable);
	bool Update();
	bool GetAlarmEvent();
	void ClearAlarmEvent();
	bool GetMisconfigurationEvent();
	void ClearMisconfigurationEvent();
	void Print();

private:
	inline bool ConfigureAlarmPin();
	inline bool ConfigureOutputEnablePin();
	bool IsAlarmPinConfiguredProperly(bool& configuredProperly);
	bool IsOutputEnablePinConfiguredProperly(bool& configuredProperly);
	inline bool IsValidRegister(uint32_t reg);
	inline bool IsValidPin(const uint32_t pin);
	inline bool IsValidFunction(const uint8_t function);
	inline bool IsValidLevel(const uint8_t level);
	inline bool IsValidPull(const uint8_t pull);
	bool SetGpioFunction(const uint32_t pin, const uint8_t function, const uint8_t pull = PULL_NOCHANGE);
	bool GetGpioFunction(const uint32_t pin, uint8_t& function);
	bool SetGpio(const uint32_t pin);
	bool ClearGpio(const uint32_t pin);
	bool GetGpio(const uint32_t pin, uint8_t& level);
	bool GetEventDetectStatus(const uint32_t pin, bool& status);
	bool ClearEventDetectStatus(const uint32_t pin);
	bool SetFallingEdgeEventEnabled(const uint32_t pin, const bool enable);
	bool GetFallingEdgeEventEnabled(const uint32_t pin, bool& enabled);
	bool SetRisingEdgeEventEnabled(const uint32_t pin, const bool enable);
	bool GetRisingEdgeEventEnabled(const uint32_t pin, bool& enabled);
	bool SetAsyncFallingEdgeEventEnabled(const uint32_t pin, const bool enable);
	bool GetAsyncFallingEdgeEventEnabled(const uint32_t pin, bool& enabled);
	bool SetAsyncRisingEdgeEventEnabled(const uint32_t pin, const bool enable);
	bool GetAsyncRisingEdgeEventEnabled(const uint32_t pin, bool& enabled);
	bool SetLowLevelEventEnabled(const uint32_t pin, const bool enable);
	bool GetLowLevelEventEnabled(const uint32_t pin, bool& enabled);
	bool SetHighLevelEventEnabled(const uint32_t pin, const bool enable);
	bool GetHighLevelEventEnabled(const uint32_t pin, bool& enabled);
	bool WriteRegister(const uint32_t reg, const uint32_t val);
	bool ReadRegister(const uint32_t reg, uint32_t& val);
	bool ModifyRegister(const uint32_t reg, const uint32_t mask, const uint32_t val);

private:
	const uint32_t m_alarmPin;
	const uint32_t m_outputEnablePin;
	volatile uint32_t* m_gpiomem;
	std::atomic_bool m_misconfigurationEvent;
	std::atomic_bool m_alarmEvent;
};

#endif /* PINCONTROLLER_H_ */
