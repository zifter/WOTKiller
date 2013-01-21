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
#define MIN_LEVEL Debug
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
#if MIN_LEVEL == Debug
	#define SLOG_DEBUG(str, ...)	LoggerImpl(Debug,	str, __VA_ARGS__)
	#define SLOG_TRACE(str, ...)	LoggerImpl(Trace,	str, __VA_ARGS__)
	#define SLOG_WARNING(str, ...)	LoggerImpl(Warning, str, __VA_ARGS__)
	#define SLOG_ERROR(str, ...)	LoggerImpl(Error,	str, __VA_ARGS__)
#elif MIN_LEVEL == Trace
	#define SLOG_DEBUG(str, ...)	
	#define SLOG_TRACE(str, ...)	LoggerImpl(Trace,	str, __VA_ARGS__)
	#define SLOG_WARNING(str, ...)	LoggerImpl(Warning, str, __VA_ARGS__)
	#define SLOG_ERROR(str, ...)	LoggerImpl(Error,	str, __VA_ARGS__)
#elif MIN_LEVEL == Warning
	#define SLOG_DEBUG(str, ...)	
	#define SLOG_TRACE(str, ...)	
	#define SLOG_WARNING(str, ...)	LoggerImpl(Warning, str, __VA_ARGS__)
	#define SLOG_ERROR(str, ...)	LoggerImpl(Error,	str, __VA_ARGS__)
#elif MIN_LEVEL == Error
	#define SLOG_DEBUG(str, ...)	
	#define SLOG_TRACE(str, ...)	
	#define SLOG_WARNING(str, ...)	
	#define SLOG_ERROR(str, ...)	LoggerImpl(Error,	str, __VA_ARGS__)
#endif

//-------------------------------------------------------------------------
#endif // __SLOGGER_H_
