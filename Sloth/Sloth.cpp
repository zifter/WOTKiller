// This is the main DLL file.

#include "stdafx.h"

#include "Sloth.h"
#include <psapi.h>

#include <tchar.h>
#include <strsafe.h>

#include <atlstr.h>
#include <ctime>

#include "SLogger.h"

namespace Sloth
{
//-------------------------------------------------------------------------
bool IsRegExist(const char regName[])
{
	SLOG_DEBUG("Is register exist.");

	HKEY hKey = NULL;
	bool retValue = true;

	long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_READ, &hKey);

	return !(ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts);
}
//-------------------------------------------------------------------------
bool ReadRegInfo( const char regName[], const char field[], char data[STR_SIZE])
{
	SLOG_DEBUG("Read register info.");

	HKEY hKey;
	bool bRet;

	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success opening key.");
		bRet = true;
	} 
	else 
	{
		LogLastErrorMessage("Error opening key.");
		bRet = false;
	}

	LPCTSTR value = TEXT(field);

	DWORD Type;
	DWORD size = 128;
	LONG setRes = RegQueryValueEx( hKey, value, NULL, &Type, (LPBYTE)data, &size);

	if (setRes == ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success reading to Registry. %s", data);
		bRet = true;
	} 
	else 
	{
		LogLastErrorMessage("Error reading from Registry.");
		bRet = false;
	}

	LONG closeOut = RegCloseKey(hKey);

	if (closeOut == ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success closing key.");
	} 
	else 
	{
		LogLastErrorMessage("Error closing key.");
		bRet = false;
	}

	return bRet;
}
//-------------------------------------------------------------------------
bool WriteRegInfo( const char regName[], const char field[], const char data[STR_SIZE] )
{
	SLOG_TRACE("Write register info. %s", data);

	HKEY hKey;
	bool bRet;
	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success opening key.");
		bRet = true;
	} 
	else 
	{
		LogLastErrorMessage("Error opening key.");
		bRet = false;
	}

	LPCTSTR value = TEXT(field);
	LPCTSTR dataValue = TEXT(data) ;

	LONG setRes = RegSetValueEx (hKey, value, 0, REG_SZ, (LPBYTE)dataValue, strlen(dataValue)+1);

	if (setRes == ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success writing to Registry.");
		bRet = true;
	} 
	else 
	{
		LogLastErrorMessage("Error writing to Registry.");
		bRet = false;
	}

	LONG closeOut = RegCloseKey(hKey);

	if (closeOut == ERROR_SUCCESS) 
	{
		SLOG_DEBUG("Success closing key.");
	} 
	else 
	{
		LogLastErrorMessage("Error closing key.");
		bRet = false;
	}

	return bRet;
}
//-------------------------------------------------------------------------
bool ConverToChar( SRegInfo& info, char data[STR_SIZE] )
{
	int seed = rand()%7345734;
	int hash = (info.timeout << 5) + (info.timeLimit << 4)  + (info.timeCheck << 3) + (info.day << 2);

	srand(seed);

	int r1 = (rand()%11) + 1;
	int r2 = (rand()%11) + 1;
	int r3 = (rand()%11) + 1;
	int r4 = (rand()%11) + 1;
	int r5 = (rand()%11) + 1;

	sprintf_s(data, 128, "%x.%o:%o:%o.%x:%x:%d", seed, 
		info.timeout	<< r1, 
		info.timeLimit	<< r2, 
		info.timeCheck	<< r3, 
		info.day		<< r4, 
		int(info.enable)<< r5, hash);
	return true;
}
//-------------------------------------------------------------------------
bool ConverToInfo( char data[STR_SIZE], SRegInfo& info )
{
	int seed;
	int testHash;
	int enable;

	sscanf_s(data, "%x.%o:%o:%o.%x:%x:%d", &seed, &info.timeout, &info.timeLimit, &info.timeCheck, &info.day, &enable, &testHash );

	srand(seed);

	int r1 = (rand()%11) + 1;
	int r2 = (rand()%11) + 1;
	int r3 = (rand()%11) + 1;
	int r4 = (rand()%11) + 1;
	int r5 = (rand()%11) + 1;

	info.timeout	= info.timeout	>> r1;
	info.timeLimit	= info.timeLimit>> r2;
	info.timeCheck	= info.timeCheck>> r3;
	info.day		= info.day		>> r4;
	enable			= enable		>> r5;

	if(enable == 0)
	{
		info.enable = false;
	}
	else if (enable == 1)
	{
		info.enable = true;
	}
	else
	{
		info.valid = false;
		return true;
	}

	int hash = (info.timeout << 5) + (info.timeLimit << 4)  + (info.timeCheck << 3) + (info.day << 2);

	info.valid = (testHash == hash);

	return true;
}

//-------------------------------------------------------------------------
void Kill( HANDLE hProc )
{
	SLOG_DEBUG("Kill process");

	DWORD fdwExit = 0;

	if(!GetExitCodeProcess(hProc, &fdwExit))
	{
		LogLastErrorMessage(TEXT("GetExitCodeProcess"));
	}

	if(!TerminateProcess(hProc, fdwExit))
	{
		LogLastErrorMessage(TEXT("Cannot kill process"));
		return;
	}

	SLOG_TRACE("Process are killed");
}
//-------------------------------------------------------------------------
HANDLE Find( TCHAR* proc )
{     
	SLOG_DEBUG("Find");

	// Get the list of process identifiers.
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		LogLastErrorMessage("Enum Processes Failed.");
	}

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Find process
	for ( i = 0; i < cProcesses; i++ )
	{
		if( aProcesses[i] != 0 )
		{
			// Get a handle to the process.
			HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE |
				PROCESS_VM_READ,
				FALSE, aProcesses[i] );

			// Get the process name.
			if (NULL != hProcess )
			{
				HMODULE hMod;
				DWORD cbNeeded;

				if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
					&cbNeeded) )
				{
					GetModuleBaseName( hProcess, hMod, szProcessName, 
						sizeof(szProcessName)/sizeof(TCHAR) );
				}
			}

			if(!_tcscmp(szProcessName, proc))
			{
				return hProcess;
			}

			CloseHandle(hProcess);
		}
	}

	return NULL;
}
//-------------------------------------------------------------------------
void LogLastErrorMessage(LPCTSTR text) 
{ 
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)text)+40)*sizeof(TCHAR)); 

	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s\n Error code %d: %s"), 
		text, dw, lpMsgBuf);

	USES_CONVERSION;
	char* message = T2A((LPTSTR)lpDisplayBuf);

	SLOG_ERROR(message);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
//-------------------------------------------------------------------------
bool CreateRegister(const char regName[]) 
{	
	SLOG_TRACE("Creating register.");
	HKEY hKey = NULL;
	bool retValue = true;

	//Step 1: Open the key
	long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_READ, &hKey);

	//Step 2: If failed, create the key
	if (ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts)
	{
		long retError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, regName, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );

		if ( retError != ERROR_SUCCESS)
		{
			LogLastErrorMessage("Could not create registry key.");
			retValue = false;
		}
		else
		{
			SLOG_TRACE("Key created.");
		}
	}
	else if (ERROR_SUCCESS != sts)
	{
		LogLastErrorMessage("Cannot open registry key.");
		retValue = false;
	}
	else //If it already existed, get the value from the key.
	{
		SLOG_WARNING("Registry key already exist.");
	}

	RegCloseKey(hKey);

	return retValue;
}
//-------------------------------------------------------------------------
void CreateRegisterDefault()
{
	CreateRegister(RegisterName);
	
	SetRegInfo(DefaultRegInfo);

	Encrypt(DefaultPass);

	WriteRegInfo(RegisterName, RegisterPass, DefaultPass);

	Dencrypt(DefaultPass);
}
//-------------------------------------------------------------------------
bool GetRegInfo( SRegInfo& info )
{
	char data[STR_SIZE];
	if(ReadRegInfo(RegisterName, RegisterData, data))
	{
		ConverToInfo(data, info);

		return true;
	}
	return false;
}
//-------------------------------------------------------------------------
bool SetRegInfo( SRegInfo& info )
{
	char data[STR_SIZE];

	ConverToChar(info, data);

	return WriteRegInfo(RegisterName, RegisterData, data);
}

//-------------------------------------------------------------------------
void SRegInfo::Update()
{
	if(!valid)
	{
		timeout		= DefaultRegInfo.timeout;
		timeLimit	= DefaultRegInfo.timeLimit;
		timeCheck	= DefaultRegInfo.timeCheck;
		day			= DefaultRegInfo.day;
		enable		= DefaultRegInfo.enable;
	}

	time_t rawtime;
	struct tm * timeinfo = new struct tm;

	time ( &rawtime );
	localtime_s(timeinfo, &rawtime );

	timeout += timeCheck;

	if(timeinfo->tm_mday != day)
	{
		day = timeinfo->tm_mday;
		timeout = 0;
	}
}
//-------------------------------------------------------------------------
bool SRegInfo::IsNeedToKill()
{
	return !valid || ((timeout > timeLimit) && enable);
}
//-------------------------------------------------------------------------
void Encrypt( char data[STR_SIZE] )
{
	for (int i = 0; i < STR_SIZE ; i++)
	{
		if(data[i] != 0)
		{
			data[i] = sizeof(char) - data[i];
		}
	}
}
//-------------------------------------------------------------------------
void Dencrypt( char data[STR_SIZE] )
{
	Encrypt(data);
}
//-------------------------------------------------------------------------
bool IsPassExist()
{
	char data[STR_SIZE];

	ReadRegInfo(RegisterName, RegisterPass, data);

	Dencrypt(data);

	return (strcmp(data, DefaultPass) != 0);
}
//-------------------------------------------------------------------------
void SetPass( char data[STR_SIZE] )
{
	Encrypt(data);

	WriteRegInfo(RegisterName, RegisterPass, data);
}
//-------------------------------------------------------------------------
void GetPass( char data[STR_SIZE] )
{
	ReadRegInfo(RegisterName, RegisterPass, data);

	Dencrypt(data);
}
//-------------------------------------------------------------------------
void LogRegInfo( SRegInfo& info )
{
	SLOG_DEBUG("timeout: %d, time limit: %d, timeCheck: %d, day: %d; is enabled: %s %d, valid %d", info.timeout, info.timeLimit, info.timeCheck, info.day, (info.enable ? "true" : "false"), info.enable, info.valid );
}

}