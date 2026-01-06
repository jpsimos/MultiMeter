/*
 * Logger.h
 *
 *  Created on: Dec 13, 2025
 *      Author: jacob a psimos
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <ostream>
#include <streambuf>
#include <string>
#include <memory>
#include <cstdarg>
#include <syslog.h>

// ⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖  Logging Macros ⧖⧖⧖⧖⧖⧖⧖⧖⧖⧖
#define LOG(lvl, fmt, ...) { \
		Logger::GetInstance().SetLevel(lvl); \
		Logger::GetInstance().Printf("%s:%u " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
	}

class Logger
{
private:
	inline static Logger* sm_loggerSingletonPtr = nullptr;

public:
	static Logger& CreateInstance();
	static Logger& GetInstance();

private:
	Logger();
	~Logger();

public:
	void Printf(const char* __restrict__ const fmt, ...);
	void SetLevel(const int level);
	Logger& operator<<(const int level);
	Logger& operator<<(const std::string& str);

private:
	int m_level;

};

#endif /* LOGGER_H_ */
