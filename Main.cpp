/*
 * Main.cpp
 *
 * Multimeter tool main source file.
 * Is an interface to PZEM-004T sensor with sqlite database record keeping.
 *
 * Author Jacob A. Psimos
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Helpers.h"
#include "Logger.h"
#include "MultiMeter.h"
#include "Database.h"
#include "PinController.h"

using namespace Helpers;

// Result code bitmask definitions
#define RESULT_BAD_ARGS                       (0x01)
#define RESULT_ADD_DEVICE_FAILED              (0x02)
#define RESULT_NO_DEVICE_FOUND                (0x04)
#define RESULT_SIG_FAILED                     (0x20)
#define RESULT_DB_CTOR_FAILED                 (0x40)
#define RESULT_MULTIMETER_CTOR_FAILED         (0x80)
#define RESULT_SET_ALARM_FAILED               (0x100)
#define RESULT_TIMER_EXCEPTION                (0x200)
#define RESULT_SIGQUIT                        (0x400)
#define RESULT_PROCESS_MULTIMETER_FAILED      (0x800)
#define RESULT_PROCESS_MULTIMETER_EXCEPTION   (0x1000)
#define RESULT_RECORD_MEASUREMENT_FAILED      (0x2000)
#define RESULT_BAD_OR_NO_MEASUREMENT          (0x4000)
#define RESULT_SET_ADDRESS_FAILED             (0x8000)

// Defaults
static const char* const defaultDeviceName = "default_device";
static const char* const defaultSerialDevicePath = "/dev/ttyAMA0";
static const char* const defaultDatabasePath = "/var/db/multimeter.db";
static const unsigned char defaultDeviceAddress = 1;
static const unsigned int defaultAlarmPin = 27;
static const unsigned int defaultBufferPin = 17;
static const time_t defaultInterval = 0;
static const double defaultAlarmWattage = 1000.0;
static const bool defaultVerbose = false;

// Prototypes
struct TimerArguments
{
	int* m_result;
	std::shared_ptr<MultiMeter> m_multiMeterPtr;
	bool m_noDatabase;
	bool m_verbose;
	bool(*m_processMultimeterProc)(TimerArguments* timerArgsPtr);
};

// Globals
static sig_atomic_t receivedQuitSignal;

// Prototypes
bool SetupQuitSignal();
void HandleQuitSignal(int quitSignal);
bool ProcessMultimeter(TimerArguments* timerArgsPtr) noexcept;
void PrintHelp();
bool FileExists(const char* const file, int flags);
bool ParseArguments(
	int argc,
	char* argv[],
	bool& createDevice,
	char*& serialDevicePath,
	char*& databasePath,
	char*& deviceName,
	unsigned char& deviceAddress,
	uint32_t& alarmPin,
	uint32_t& bufferPin,
	time_t& interval,
	double& alarmWattage,
	bool& verbose,
	bool& noDatabase
);

int main(int argc, char* argv[], char* envp[])
{
	int result = EXIT_SUCCESS;
	bool createDevice;
	char* serialDevicePath;
	char* databasePath;
	char* deviceName;
	unsigned char deviceAddress;
	uint32_t alarmPin;
	uint32_t outputEnablePin;
	time_t interval;
	double alarmWattage;
	bool verbose;
	bool noDatabase;
	std::shared_ptr<MultiMeter> multiMeterPtr;
	TimerArguments timerArgs;
	Threading::Timer multiMeterTimer;

	result = EXIT_SUCCESS;

	Logger::CreateInstance();

	if(!ParseArguments(
			argc,
			argv,
			createDevice,
			serialDevicePath,
			databasePath,
			deviceName,
			deviceAddress,
			alarmPin,
			outputEnablePin,
			interval,
			alarmWattage,
			verbose,
			noDatabase
		)
	)
	{
		if(!FileExists(serialDevicePath, S_IFCHR))
		{
			std::cerr << "Serial device not accessible.\n";
		}

		if(!noDatabase && !FileExists(databasePath, S_IFREG))
		{
			std::cerr << "Database file not accessible.\n";
		}

		result |= RESULT_BAD_ARGS;
	}
	
	if(EXIT_SUCCESS == result)
	{
		if(!SetupQuitSignal())
		{
			result |= RESULT_SIG_FAILED;
		}
	}

	if(EXIT_SUCCESS == result && !noDatabase)
	{
		try
		{
			Database::CreateInstance(databasePath);
		}
		catch(const std::exception& e)
		{
			result |= RESULT_DB_CTOR_FAILED;
		}
	}

	// Create the device if arguments supplied.
	if(EXIT_SUCCESS == result)
	{
		Database::DevicesTableRow deviceTableRow;

		if(Database::GetInstance().GetDeviceByName(deviceName, deviceTableRow))
		{
			if(createDevice && deviceTableRow.m_null)
			{
				deviceTableRow.id = 0;

				if(!Database::GetInstance().AddDevice(
					deviceName,
					deviceAddress,
					alarmWattage,
					alarmPin,
					outputEnablePin,
					deviceTableRow.id)
				)
				{
					if(deviceTableRow.id == 0)
					{
						result |= RESULT_ADD_DEVICE_FAILED;
					}
				}
			}
			else if(!createDevice && deviceTableRow.m_null)
			{
				result |= RESULT_NO_DEVICE_FOUND;
			}
		}
	}

	// C'tor the MultiMeter device by name.
	if(EXIT_SUCCESS == result)
	{
		try
		{
			multiMeterPtr.reset(new MultiMeter(
				deviceName
			));
		}
		catch(const std::exception& e)
		{
			result |= RESULT_MULTIMETER_CTOR_FAILED;
		}
	}
	
	// Begin sampling the sensor.
	if(EXIT_SUCCESS == result)
	{
		timerArgs.m_result = &result;
		timerArgs.m_multiMeterPtr = multiMeterPtr;
		timerArgs.m_noDatabase = noDatabase;
		timerArgs.m_verbose = verbose;
		timerArgs.m_processMultimeterProc = ProcessMultimeter;
	
		if(interval > 0)
		{
			multiMeterTimer.Begin(
				interval,
				false,
				[=](void* arg) -> bool {
					TimerArguments* timerArgsPtr = reinterpret_cast<TimerArguments*>(arg);
					bool continueProcessing = false;

					if(nullptr != timerArgsPtr)
					{
						if(timerArgsPtr->m_processMultimeterProc(timerArgsPtr))
						{
							continueProcessing = true;
						}
						else
						{
							if(nullptr != timerArgsPtr->m_result)
							{
								*(timerArgsPtr->m_result) |= RESULT_PROCESS_MULTIMETER_FAILED;
							}
						}
					}

					return continueProcessing;
				},
				&timerArgs
			);

			while(!receivedQuitSignal && multiMeterTimer && EXIT_SUCCESS == result)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
			}

			if(receivedQuitSignal)
			{
				multiMeterTimer.End();
				result |= RESULT_SIGQUIT;
			}
		}
		else
		{
			if(!ProcessMultimeter(&timerArgs))
			{
				result |= RESULT_PROCESS_MULTIMETER_FAILED;
			}
		}
	}

	return result;
}

bool ProcessMultimeter(TimerArguments* timerArgsPtr) noexcept
{
	bool hadError = true;

	if(nullptr != timerArgsPtr)
	{
		try
		{
			if(timerArgsPtr->m_multiMeterPtr->Update()
				&& !timerArgsPtr->m_multiMeterPtr->HadError())
			{
				if(!timerArgsPtr->m_noDatabase)
				{
					if(timerArgsPtr->m_multiMeterPtr->Record())
					{
						hadError = false;
					}
					else
					{
						if(nullptr != timerArgsPtr->m_result)
						{
							*(timerArgsPtr->m_result) |= RESULT_RECORD_MEASUREMENT_FAILED;
						}
					}
				}
			}
			else
			{
				if(nullptr != timerArgsPtr->m_result)
				{
					*(timerArgsPtr->m_result) |= RESULT_BAD_OR_NO_MEASUREMENT;
				}
				hadError = true;
			}

			hadError = hadError || timerArgsPtr->m_multiMeterPtr->HadError();

			if(!hadError)
			{
				if(timerArgsPtr->m_verbose)
				{
					std::cout << timerArgsPtr->m_multiMeterPtr->ToJsonString() << '\n';
				}
			}
			else
			{
				if(nullptr != timerArgsPtr->m_result)
				{
					*(timerArgsPtr->m_result) |= RESULT_BAD_OR_NO_MEASUREMENT;
				}
			}
		}
		catch(const std::exception& e)
		{
			*(timerArgsPtr->m_result) |= RESULT_PROCESS_MULTIMETER_EXCEPTION;
			hadError = true;
		}
	}

	return !hadError;
}

bool SetupQuitSignal()
{
	bool hadError;
	receivedQuitSignal = 0;
	errno = 0;

	::signal(SIGQUIT, HandleQuitSignal);
	hadError = static_cast<bool>(errno != 0);

	return !hadError;
}

void HandleQuitSignal(int quitSignal)
{
	receivedQuitSignal = 1;
}

bool FileExists(const char* const file, int flags)
{
	bool fileExists = false;
	struct stat st;

	errno = 0;
	st.st_mode = 0;

	if(!::stat(file, &st))
	{
		if(flags == static_cast<int>(st.st_mode & S_IFMT))
		{
			fileExists = true;
		}
	}

	return fileExists;
}

bool ParseArguments(
	int argc,
	char* argv[],
	bool& createDevice,
	char*& serialDevicePath,
	char*& databasePath,
	char*& deviceName,
	unsigned char& deviceAddress,
	uint32_t& alarmPin,
	uint32_t& bufferPin,
	time_t& interval,
	double& alarmWattage,
	bool& verbose,
	bool& noDatabase
)
{
	bool hadError = false;
	bool handled = false;
	char* arg = nullptr;
	char* next = nullptr;

	createDevice = false;
	serialDevicePath = const_cast<char*>(defaultSerialDevicePath);
	databasePath = const_cast<char*>(defaultDatabasePath);
	deviceName = std::strlen(argv[argc - 1]) > 0
		? argv[argc - 1]
			   : const_cast<char*>(defaultDeviceName);
	deviceAddress = defaultDeviceAddress;
	alarmPin = defaultAlarmPin;
	bufferPin = defaultBufferPin;
	interval = defaultInterval;
	alarmWattage = defaultAlarmWattage;
	verbose = defaultVerbose;
	noDatabase = false;

	if(argc == 1)
	{
		PrintHelp();
		std::exit(EXIT_FAILURE);
	}

	for(int argi = 1; argi < argc && !hadError; argi++)
	{
		arg = &argv[argi][0];
		next = argi < argc - 1
				? &argv[argi + 1][0]
					: static_cast<char*>(nullptr);

		if(nullptr != next)
		{
			handled = true;

			if(Text::CStrEq("--interval", arg) || Text::CStrEq("-i", arg))
			{
				errno = 0;
				interval = static_cast<time_t>(std::strtoul(next, nullptr, 0));

				if(errno || interval < 500)
				{
					hadError = true;
				}
			}
			else if(Text::CStrEq("--alarm-pin", arg) || Text::CStrEq("-k", arg))
			{
				errno = 0;
				alarmPin = static_cast<uint32_t>(std::strtoul(next, nullptr, 0));

				if(errno)
				{
					hadError = true;
				}
			}
			else if(Text::CStrEq("--oe-pin", arg) || Text::CStrEq("-b", arg))
			{
				errno = 0;
				bufferPin = static_cast<uint32_t>(std::strtoul(next, nullptr, 0));

				if(errno)
				{
					hadError = true;
				}
			}
			else if(Text::CStrEq("--alarm-wattage", arg) || Text::CStrEq("-a", arg))
			{
				errno = 0;
				alarmWattage = std::strtod(next, nullptr);
				
				if(errno != 0 || std::isnan(alarmWattage) || alarmWattage < ALARM_MIN || alarmWattage > ALARM_MAX)
				{
					alarmWattage = NAN;
					hadError = true;
				}
			}
			else if(Text::CStrEq("--address", arg) || Text::CStrEq("-A", arg))
			{
				errno = 0;
				deviceAddress = static_cast<unsigned char>(std::strtoul(next, nullptr, 0));

				if(errno != 0 || deviceAddress < ADDRESS_MIN || deviceAddress > ADDRESS_MAX)
				{
					deviceAddress = 0;
					hadError = true;
				}
			}
			else if(Text::CStrEq("--database", arg) || Text::CStrEq("-D", arg))
			{
				databasePath = next;
			}
			else
			{
				handled = false;
			}

			if(handled)
			{
				continue;
			}
		}
		
		if(Text::CStrEq("--create-device", arg) || Text::CStrEq("-c", arg))
		{
			createDevice = true;
		}

		if(Text::CStrEq("--help", arg) || Text::CStrEq("-h", arg))
		{
			PrintHelp();
			std::exit(EXIT_SUCCESS);
		}
		else if(Text::CStrEq("--verbose", arg) || Text::CStrEq("-v", arg))
		{
			verbose = true;
		}
		else if(Text::CStrEq("--no-database", arg) || Text::CStrEq("-n", arg))
		{
			noDatabase = true;
		}
	}

	return !hadError;
}

void PrintHelp()
{
	std::cout <<
		"             _              " "\n"
		"   ___  ___ (_)_ _  ___  ___" "\n"
		"  / _ \\(_-</ /  ' \\/ _ \\(_-<" "\n"
		" / .__/___/_/_/_/_/\\___/___/" "\n"
		"/_/                         " "\n"
		"MultiMeter (PZEM-004X) Utility\n"
		"Wrote by Jacob A. Psimos\n"
		"Built " __DATE__ "\n\n"
		"./MultiMeter [OPTIONS] [NAME]\n"
		"\t-c  --create-device      creates a new device (needs defaults or -k, -b, -a, -A, and -s)\n"
		"\t-k  --alarm-pin          sets the alarm pin (default GPIO27)\n"
		"\t-b  --oe-pin             sets the output enable pin (default GPIO17)\n"
		"\t-a  --alarm-wattage      sets the power alarm wattage\n"
		"\t-A  --address            sets the device address\n"
		"\t-s  --serial-device      the serial device to use (default " << defaultSerialDevicePath << ")\n"
		"\t-d  --no-database        do not use a database (requires -k, -b, and -s)\n"
		"\t-D  --database           database path (default " << defaultDatabasePath << ")\n"
		"\t-i  --interval           repeat interval msecs >= 500\n"
		"\t-v  --verbose            enable verbose\n"
		"Signal with SIGQUIT to gracefully terminate when a repeat interval is used.\n"
		<< std::flush;
}
