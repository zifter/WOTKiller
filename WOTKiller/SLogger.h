#ifndef __SLOGGER_H_
#define __SLOGGER_H_
//-------------------------------------------------------------------------
#include <fstream>
//-------------------------------------------------------------------------
enum LogLevel
{
	Debug,
	Trace,
	Warning,
	Error,
};
#define ENABLE_MIN_LABEL Debug
//-------------------------------------------------------------------------
class SLogger
{
public:
	SLogger();
	~SLogger();

	static std::ofstream	stream;
	static TCHAR			path[1024];
	static char				buffer[1024];
};
//-------------------------------------------------------------------------
void LoggerImpl(LogLevel level, const char* str, ...);
//-------------------------------------------------------------------------
#define SLOG_DEBUG(str, ...)	LoggerImpl(Debug,	str, __VA_ARGS__)
#define SLOG_TRACE(str, ...)	LoggerImpl(Trace,	str, __VA_ARGS__)
#define SLOG_WARNING(str, ...)	LoggerImpl(Warning, str, __VA_ARGS__)
#define SLOG_ERROR(str, ...)	LoggerImpl(Error,	str, __VA_ARGS__)
//-------------------------------------------------------------------------
#endif // __SLOGGER_H_
