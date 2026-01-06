/*
 * PinController.cpp
 *
 *  Created on: Dec 11, 2025
 *      Author: jacob a psimos
 */

#include <errno.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "PinController.h"
#include "Logger.h"

using namespace std::chrono_literals;

PinController::PinController(
	const uint32_t alarmPin,
	const uint32_t outputEnablePin
)
	: m_alarmPin(alarmPin),
	  m_outputEnablePin(outputEnablePin),
	  m_gpiomem(nullptr),
	  m_misconfigurationEvent(false),
	  m_alarmEvent(false)
{
	bool hadError = true;
	int gpiomemFd;

	errno = 0;
	gpiomemFd = ::open("/dev/gpiomem", O_RDWR | O_SYNC);

	if(-1 != gpiomemFd && errno == 0)
	{
		m_gpiomem = static_cast<volatile uint32_t*>(::mmap(
				0,
				4096,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				gpiomemFd,
				0x3F200000
			)
		);

		::close(gpiomemFd);
	}

	if(nullptr != m_gpiomem && errno == 0)
	{
		if(ConfigureOutputEnablePin() && ConfigureAlarmPin())
		{
			hadError = false;
		}
	}

	if(hadError)
	{
		RUNTIME_ERROR("Could not construct PinController.");
	}
}

PinController::~PinController()
{
	if(nullptr != m_gpiomem)
	{
		SetOutputEnable(false);

		::munmap((void*)m_gpiomem, 4096);
		m_gpiomem = nullptr;
	}
}

bool PinController::SetOutputEnable(const bool enable)
{
	bool hadError;

	if(enable)
	{
		hadError = !ClearGpio(m_outputEnablePin);
	}
	else
	{
		hadError = !SetGpio(m_outputEnablePin);
	}

	return !hadError;
}

bool PinController::Update()
{
	bool hadError;
	bool alarmPinConfiguredProperly;
	bool outputEnablePinConfiguredProperly;
	bool alarmEvent;

	hadError = !(IsOutputEnablePinConfiguredProperly(outputEnablePinConfiguredProperly)
		&& IsAlarmPinConfiguredProperly(alarmPinConfiguredProperly));

	if(!hadError)
	{
		if(!outputEnablePinConfiguredProperly || !alarmPinConfiguredProperly)
		{
			m_misconfigurationEvent = true;
			hadError = true;
		}
		else
		{
			hadError = !GetEventDetectStatus(m_alarmPin, alarmEvent);

			if(!hadError && alarmEvent)
			{
				hadError = !ClearEventDetectStatus(m_alarmPin);
				m_alarmEvent = true;
			}
		}
	}

	return !hadError && outputEnablePinConfiguredProperly && alarmPinConfiguredProperly;
}

void PinController::Print()
{
	bool configuredProperly;
	uint8_t function;
	uint8_t level;
	bool eventStatus;
	bool asyncFallingEdgeEventEnabled;
	bool asyncRisingEdgeEventEnabled;
	bool fallingEdgeEventEnabled;
	bool risingEdgeEventEnabled;
	bool lowLevelEventEnabled;
	bool highLevelEventEnabled;
	bool hadError;

	hadError = !(IsAlarmPinConfiguredProperly(configuredProperly)
		&& GetGpioFunction(m_alarmPin, function)
		&& GetGpio(m_alarmPin, level)
		&& GetEventDetectStatus(m_alarmPin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_alarmPin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_alarmPin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_alarmPin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_alarmPin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_alarmPin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_alarmPin, highLevelEventEnabled)
	);

	std::cout << "alarm_pin\n";
	std::cout << "hadError=" << hadError << "\n"
		<< "configuredProperly=" << configuredProperly << "\n"
		<< "function=" << static_cast<unsigned int>(function) << "\n"
		<< "level=" << static_cast<unsigned int>(level) << "\n"
		<< "eventStatus=" << eventStatus << "\n"
		<< "asyncFallingEdgeEventEnabled=" << asyncFallingEdgeEventEnabled << "\n"
		<< "asyncRisingEdgeEventEnabled=" << asyncRisingEdgeEventEnabled << "\n"
		<< "fallingEdgeEventEnabled=" << fallingEdgeEventEnabled << "\n"
		<< "risingEdgeEventEnabled=" << risingEdgeEventEnabled << "\n"
		<< "lowLevelEventEnabled=" << lowLevelEventEnabled << "\n"
		<< "highLevelEventEnabled=" << highLevelEventEnabled << "\n\n";

	hadError = !(IsOutputEnablePinConfiguredProperly(configuredProperly)
		&& GetGpioFunction(m_outputEnablePin, function)
		&& GetGpio(m_outputEnablePin, level)
		&& GetEventDetectStatus(m_outputEnablePin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_outputEnablePin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_outputEnablePin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_outputEnablePin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_outputEnablePin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_outputEnablePin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_outputEnablePin, highLevelEventEnabled)
	);

	std::cout << "buffer_pin\n";
	std::cout << "hadError=" << hadError << "\n"
		<< "configuredProperly=" << configuredProperly << "\n"
		<< "function=" << static_cast<unsigned int>(function) << "\n"
		<< "level=" << static_cast<unsigned int>(level) << "\n"
		<< "eventStatus=" << eventStatus << "\n"
		<< "asyncFallingEdgeEventEnabled=" << asyncFallingEdgeEventEnabled << "\n"
		<< "asyncRisingEdgeEventEnabled=" << asyncRisingEdgeEventEnabled << "\n"
		<< "fallingEdgeEventEnabled=" << fallingEdgeEventEnabled << "\n"
		<< "risingEdgeEventEnabled=" << risingEdgeEventEnabled << "\n"
		<< "lowLevelEventEnabled=" << lowLevelEventEnabled << "\n"
		<< "highLevelEventEnabled=" << highLevelEventEnabled << "\n\n";

	std::cout << "MisconfigurationEvent=" << m_misconfigurationEvent.load() << "\n"
			<< "AlarmEvent=" << m_alarmEvent.load() << "\n";
}

bool PinController::GetAlarmEvent()
{
	return m_alarmEvent.load();
}

void PinController::ClearAlarmEvent()
{
	m_alarmEvent.store(false);
}

bool PinController::GetMisconfigurationEvent()
{
	return m_misconfigurationEvent.load();
}

void PinController::ClearMisconfigurationEvent()
{
	return m_misconfigurationEvent.store(false);
}

inline bool PinController::ConfigureAlarmPin()
{
	bool configuredProperly;
	uint8_t function;
	uint8_t level;
	bool eventStatus;
	bool asyncFallingEdgeEventEnabled;
	bool asyncRisingEdgeEventEnabled;
	bool fallingEdgeEventEnabled;
	bool risingEdgeEventEnabled;
	bool lowLevelEventEnabled;
	bool highLevelEventEnabled;
	bool hadError;

	hadError = !(GetGpioFunction(m_alarmPin, function)
		&& GetGpio(m_alarmPin, level)
		&& GetEventDetectStatus(m_alarmPin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_alarmPin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_alarmPin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_alarmPin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_alarmPin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_alarmPin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_alarmPin, highLevelEventEnabled)
	);

	if(!hadError)
	{
		hadError = !SetGpioFunction(m_alarmPin, FUNCTION_INPUT);
	}

	if(!hadError && level == LEVEL_LOW)
	{
		m_alarmEvent = true;
	}

	if(!hadError && eventStatus)
	{
		hadError = !ClearEventDetectStatus(m_alarmPin);
	}

	if(!hadError && asyncFallingEdgeEventEnabled)
	{
		hadError = !SetAsyncFallingEdgeEventEnabled(m_alarmPin, false);
	}

	if(!hadError && asyncRisingEdgeEventEnabled)
	{
		hadError = !SetAsyncRisingEdgeEventEnabled(m_alarmPin, false);
	}

	if(!hadError && !fallingEdgeEventEnabled)
	{
		hadError = !SetFallingEdgeEventEnabled(m_alarmPin, true);
	}

	if(!hadError && risingEdgeEventEnabled)
	{
		hadError = !SetRisingEdgeEventEnabled(m_alarmPin, false);
	}

	if(!hadError && lowLevelEventEnabled)
	{
		hadError = !SetLowLevelEventEnabled(m_alarmPin, false);
	}

	if(!hadError && highLevelEventEnabled)
	{
		hadError = !SetHighLevelEventEnabled(m_alarmPin, false);
	}

	if(!hadError)
	{
		hadError = !ClearEventDetectStatus(m_alarmPin);
	}

	if(!hadError)
	{
		hadError = !IsAlarmPinConfiguredProperly(configuredProperly);

		if(!hadError)
		{
			hadError = !configuredProperly;
		}
	}

	return !hadError;
}

inline bool PinController::ConfigureOutputEnablePin()
{
	bool configuredProperly;
	uint8_t function;
	uint8_t level;
	bool eventStatus;
	bool asyncFallingEdgeEventEnabled;
	bool asyncRisingEdgeEventEnabled;
	bool fallingEdgeEventEnabled;
	bool risingEdgeEventEnabled;
	bool lowLevelEventEnabled;
	bool highLevelEventEnabled;
	bool hadError;

	hadError = !(GetGpioFunction(m_outputEnablePin, function)
		&& GetGpio(m_outputEnablePin, level)
		&& GetEventDetectStatus(m_outputEnablePin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_outputEnablePin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_outputEnablePin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_outputEnablePin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_outputEnablePin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_outputEnablePin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_outputEnablePin, highLevelEventEnabled)
	);

	if(!hadError && function != FUNCTION_OUTPUT)
	{
		hadError = !SetGpioFunction(m_outputEnablePin, FUNCTION_OUTPUT);

		if(!hadError)
		{
			hadError = !GetGpio(m_outputEnablePin, level);
		}
	}

	if(!hadError && level == LEVEL_LOW)
	{
		hadError = !SetGpio(m_outputEnablePin);
	}

	if(!hadError && eventStatus)
	{
		hadError = !ClearEventDetectStatus(m_outputEnablePin);
	}

	if(!hadError && asyncFallingEdgeEventEnabled)
	{
		hadError = !SetAsyncFallingEdgeEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError && asyncRisingEdgeEventEnabled)
	{
		hadError = !SetAsyncRisingEdgeEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError && fallingEdgeEventEnabled)
	{
		hadError = !SetFallingEdgeEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError && risingEdgeEventEnabled)
	{
		hadError = !SetRisingEdgeEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError && lowLevelEventEnabled)
	{
		hadError = !SetLowLevelEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError && highLevelEventEnabled)
	{
		hadError = !SetHighLevelEventEnabled(m_outputEnablePin, false);
	}

	if(!hadError)
	{
		hadError = !ClearEventDetectStatus(m_outputEnablePin);
	}

	if(!hadError)
	{
		hadError = !IsOutputEnablePinConfiguredProperly(configuredProperly);

		if(!hadError)
		{
			hadError = !configuredProperly;
		}
	}

	return !hadError;
}

bool PinController::IsAlarmPinConfiguredProperly(bool& configuredProperly)
{
	uint8_t function;
	uint8_t level;
	bool eventStatus;
	bool asyncFallingEdgeEventEnabled;
	bool asyncRisingEdgeEventEnabled;
	bool fallingEdgeEventEnabled;
	bool risingEdgeEventEnabled;
	bool lowLevelEventEnabled;
	bool highLevelEventEnabled;
	bool hadError;

	hadError = !(GetGpioFunction(m_alarmPin, function)
		&& GetGpio(m_alarmPin, level)
		&& GetEventDetectStatus(m_alarmPin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_alarmPin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_alarmPin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_alarmPin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_alarmPin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_alarmPin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_alarmPin, highLevelEventEnabled)
	);

	if(!hadError)
	{
		configuredProperly = (function == FUNCTION_INPUT
			&& !asyncFallingEdgeEventEnabled
			&& !asyncRisingEdgeEventEnabled
			&& fallingEdgeEventEnabled
			&& !risingEdgeEventEnabled
			&& !lowLevelEventEnabled
			&& !highLevelEventEnabled
		);
	}
	else
	{
		configuredProperly = false;
	}

	return !hadError;
}

bool PinController::IsOutputEnablePinConfiguredProperly(bool& configuredProperly)
{
	uint8_t function;
	uint8_t level;
	bool eventStatus;
	bool asyncFallingEdgeEventEnabled;
	bool asyncRisingEdgeEventEnabled;
	bool fallingEdgeEventEnabled;
	bool risingEdgeEventEnabled;
	bool lowLevelEventEnabled;
	bool highLevelEventEnabled;
	bool hadError;

	hadError = !(GetGpioFunction(m_outputEnablePin, function)
		&& GetGpio(m_outputEnablePin, level)
		&& GetEventDetectStatus(m_outputEnablePin, eventStatus)
		&& GetAsyncFallingEdgeEventEnabled(m_outputEnablePin, asyncFallingEdgeEventEnabled)
		&& GetAsyncRisingEdgeEventEnabled(m_outputEnablePin, asyncRisingEdgeEventEnabled)
		&& GetFallingEdgeEventEnabled(m_outputEnablePin, fallingEdgeEventEnabled)
		&& GetRisingEdgeEventEnabled(m_outputEnablePin, risingEdgeEventEnabled)
		&& GetLowLevelEventEnabled(m_outputEnablePin, lowLevelEventEnabled)
		&& GetHighLevelEventEnabled(m_outputEnablePin, highLevelEventEnabled)
	);

	if(!hadError)
	{
		configuredProperly = (function == FUNCTION_OUTPUT
			&& !eventStatus
			&& !asyncFallingEdgeEventEnabled
			&& !asyncRisingEdgeEventEnabled
			&& !fallingEdgeEventEnabled
			&& !risingEdgeEventEnabled
			&& !lowLevelEventEnabled
			&& !highLevelEventEnabled
		);
	}
	else
	{
		configuredProperly = false;
	}

	return !hadError;
}

inline bool PinController::IsValidRegister(uint32_t reg)
{
	return (
			!( 0x18 == reg
			|| 0x24 == reg
			|| 0x30 == reg
			|| 0x3C == reg
			|| 0x48 == reg
			|| 0x54 == reg
			|| 0x60 == reg
			|| 0x6C == reg
			|| 0x78 == reg
			|| 0x84 == reg
			|| 0x90 == reg
			|| 0xA0 == reg
			|| 0xB0 == reg)
		&& reg >= 0 && reg <= 0x9C
	);
}

inline bool PinController::IsValidPin(const uint32_t pin)
{
	return pin >= 0 && pin <= 53;
}

inline bool PinController::IsValidFunction(const uint8_t function)
{
	return function <= 0b111 && (function & 0b111) == function;
}

inline bool PinController::IsValidLevel(const uint8_t level)
{
	return level == LEVEL_HIGH || level == LEVEL_LOW;
}

inline bool PinController::IsValidPull(const uint8_t pull)
{
	return pull == PULL_UP || pull == PULL_DOWN || pull == PULL_NONE;
}

bool PinController::SetGpioFunction(const uint32_t pin, const uint8_t function, const uint8_t pull)
{
	bool hadError = true;
	uint32_t reg;
	uint32_t mask;
	uint32_t shift;
	uint32_t value;

	reg = GPFSEL0 + ((pin * 3) / 30);
	shift = (pin * 3) % 30;
	mask = 0b111 << shift;
	value = static_cast<uint32_t>(function) << shift;

	if(IsValidPin(pin) && IsValidFunction(function)
		&& IsValidRegister(reg))
	{
		hadError = !ModifyRegister(reg, mask, value);
	}

	if(!hadError && IsValidPull(pull) && function == FUNCTION_INPUT)
	{
		reg = pin / 32 == 0 ? GPPUDCLK0 : GPPUDCLK1;
		shift = pin % 32;
		mask = 1 << shift;
		value = mask;
		hadError = !WriteRegister(GPPUD, pull);

		if(!hadError)
		{
			std::this_thread::sleep_for(0.6us);
			hadError = !WriteRegister(reg, value);

			if(!hadError)
			{
				std::this_thread::sleep_for(0.6us);
				hadError = !WriteRegister(GPPUD, 0);

				if(!hadError)
				{
					std::this_thread::sleep_for(0.6us);
					hadError = !WriteRegister(reg, 0);
				}
			}
		}
	}

	return !hadError;
}

bool PinController::GetGpioFunction(const uint32_t pin, uint8_t& function)
{
	bool hadError = true;
	uint32_t reg;
	uint32_t value;
	uint32_t shift;

	reg = GPFSEL0 + (((pin * 3) / 30) << 2);
	shift = (pin * 3) % 30;

	if(IsValidPin(pin) && IsValidRegister(reg))
	{
		hadError = !ReadRegister(reg, value);

		if(!hadError)
		{
			value = (value >> shift) & 0b111;

			if(IsValidFunction(static_cast<uint8_t>(value)))
			{
				function = static_cast<uint8_t>(value);
				hadError = false;
			}
		}
	}

	return !hadError;
}

bool PinController::SetGpio(const uint32_t pin)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		value = 1 << (pin % 32);
		hadError = !WriteRegister(pin / 32 == 0 ? GPSET0 : GPSET1, value);
	}

	return !hadError;
}

bool PinController::ClearGpio(const uint32_t pin)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		value = 1 << (pin % 32);
		hadError = !WriteRegister(pin / 32 == 0 ? GPCLR0 : GPCLR1, value);
	}

	return !hadError;
}

bool PinController::GetGpio(const uint32_t pin, uint8_t& level)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPLEV0 : GPLEV1, value);

		if(!hadError)
		{
			level = static_cast<uint8_t>(!!(value & (1 << (pin % 32))));
		}
	}

	return !hadError;
}

bool PinController::GetEventDetectStatus(const uint32_t pin, bool& status)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPEDS0 : GPEDS1, value);
		if(!hadError)
		{
			status = static_cast<bool>(value & (1 << (pin % 32)));
		}
	}

	return !hadError;
}

bool PinController::ClearEventDetectStatus(const uint32_t pin)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = mask;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPEDS0 : GPEDS1, mask, value);
	}

	return !hadError;
}

bool PinController::SetFallingEdgeEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPFEN0 : GPFEN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetFallingEdgeEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPFEN0 : GPFEN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::SetRisingEdgeEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPREN0 : GPREN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetRisingEdgeEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPREN0 : GPREN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::SetAsyncFallingEdgeEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPAFEN0 : GPAFEN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetAsyncFallingEdgeEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPAFEN0 : GPAFEN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::SetAsyncRisingEdgeEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPAREN0 : GPAREN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetAsyncRisingEdgeEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPAREN0 : GPAREN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::SetLowLevelEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPLEN0 : GPLEN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetLowLevelEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPLEN0 : GPLEN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::SetHighLevelEventEnabled(const uint32_t pin, const bool enable)
{
	bool hadError = true;
	uint32_t mask;
	uint32_t value;

	if(IsValidPin(pin))
	{
		mask = 1 << (pin % 32);
		value = enable ? mask : 0;

		hadError = !ModifyRegister(pin / 32 == 0 ? GPHEN0 : GPHEN1, mask, value);
	}

	return !hadError;
}

bool PinController::GetHighLevelEventEnabled(const uint32_t pin, bool& enabled)
{
	bool hadError = true;
	uint32_t value;

	if(IsValidPin(pin))
	{
		hadError = !ReadRegister(pin / 32 == 0 ? GPHEN0 : GPHEN1, value);

		if(!hadError)
		{
			enabled = static_cast<bool>((value >> (pin % 32)) & 1);
		}
	}

	return !hadError;
}

bool PinController::WriteRegister(const uint32_t reg, const uint32_t val)
{
	bool hadError = true;

	if(nullptr != m_gpiomem && IsValidRegister(reg))
	{
		m_gpiomem[reg / 4] = val;
		hadError = false;
	}

	return !hadError;
}

bool PinController::ReadRegister(const uint32_t reg, uint32_t& val)
{
	bool hadError = true;

	if(nullptr != m_gpiomem && IsValidRegister(reg))
	{
		val = m_gpiomem[reg / 4];
		hadError = false;
	}

	return !hadError;
}

bool PinController::ModifyRegister(const uint32_t reg, const uint32_t mask, const uint32_t val)
{
	bool hadError = true;

	if(nullptr != m_gpiomem && IsValidRegister(reg))
	{
		m_gpiomem[reg / 4] = (m_gpiomem[reg / 4] & ~mask) | (val & mask);
		hadError = false;
	}

	return !hadError;
}

