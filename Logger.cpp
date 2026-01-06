/*
 * LoggerBuffer.cpp
 *
 *  Created on: Dec 13, 2025
 *      Author: jacob a psimos
 */

#include <iostream>
#include <ostream>
#include <streambuf>
#include <string>
#include <memory>
#include <cstdarg>
#include <syslog.h>
#include "Logger.h"
#include "Helpers.h"

using namespace Helpers;

Logger& Logger::CreateInstance()
{
	if(nullptr == Logger::sm_loggerSingletonPtr)
	{
		Logger::sm_loggerSingletonPtr = new Logger();
	}
	return *Logger::sm_loggerSingletonPtr;
}

Logger& Logger::GetInstance()
{
	if(nullptr == Logger::sm_loggerSingletonPtr)
	{
		RUNTIME_ERROR("Must call CreateInstance first.");
	}

	return *Logger::sm_loggerSingletonPtr;
}

Logger::Logger()
	: m_level(LOG_ERR)
{
	::openlog(nullptr, LOG_PID | LOG_NDELAY, LOG_USER);
}

Logger::~Logger()
{
	::closelog();
}

void Logger::SetLevel(const int level)
{
	m_level = level;
}

void Logger::Printf(const char* __restrict__ const fmt, ...)
{
	std::va_list args;
	va_start(args, fmt);
	::vsyslog(m_level, fmt, args);
	va_end(args);
}

Logger& Logger::operator<<(const int level)
{
	m_level = level;
	return *this;
}

Logger& Logger::operator<<(const std::string& str)
{
	::syslog(m_level, "%s", str.c_str());
	return *this;
}


