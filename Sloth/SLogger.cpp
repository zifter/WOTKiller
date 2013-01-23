#include "StdAfx.h"
#include "SLogger.h"
//-------------------------------------------------------------------------
#include <shlwapi.h>
#include <windows.h>
#include <shlobj.h>
#include <ctime>
#include <tchar.h>
#include <strsafe.h>
//-------------------------------------------------------------------------
#pragma comment(lib,"shlwapi.lib")
//-------------------------------------------------------------------------
std::ofstream	SLogger::stream;
TCHAR			SLogger::path[1024] = {0};
//-------------------------------------------------------------------------
char* LogLevelName[] = 
{	
	"[DEBUG]",
	"[TRACE]",
	"[WARN]",
	"[ERROR]",
};
//-------------------------------------------------------------------------
#define MAX_LENGTH_FOR_FORMATED_STRING 64
//-------------------------------------------------------------------------
void LoggerImpl( LogLevel level, const char* str )
{
	if(level < MIN_LEVEL)
	{
		return;
	}

	SLogger log;

	time_t rawtime;
	struct tm * timeinfo = new struct tm;

	time ( &rawtime );
	localtime_s(timeinfo, &rawtime );

	// level
	log.stream 
		<< LogLevelName[level] << "\t";

	// time
	// asctime_s gen a lot of data and insert \n
	log.stream << timeinfo->tm_year - 100 << ".";

	if(timeinfo->tm_mon - 9 < 0)	{	log.stream << "0";	}
	log.stream << timeinfo->tm_mon + 1 << ".";

	if(timeinfo->tm_mday - 9 < 0)	{	log.stream << "0";	}
	log.stream << timeinfo->tm_mday + 1 << " ";

	if(timeinfo->tm_hour - 10 < 0)	{	log.stream << "0";	}
	log.stream << timeinfo->tm_hour << ":";
		
	if(timeinfo->tm_min - 10 < 0)	{	log.stream << "0";	}
	log.stream << timeinfo->tm_min << ":";

	if(timeinfo->tm_sec - 10 < 0)	{	log.stream << "0";	}
	log.stream << timeinfo->tm_sec << "\t\t";

	// text
	log.stream 
		<< str << "\n";

	delete timeinfo;
}
//-------------------------------------------------------------------------
SLogger::SLogger()
{
	if(path[0] == 0)
	{
		if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path ) ) )
		{
			PathAppend( path, _T("\\WOTKiller") );
		}

		if (CreateDirectory(path, NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			PathAppend( path, _T("\\log") );
		}
	}

	stream.open(path, std::ios_base::app);
}
//-------------------------------------------------------------------------
SLogger::~SLogger()
{
	stream.close();
}
