// WOTKiller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Shlobj.h>
#include <iostream>
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <atlstr.h>
#include <ctime>

#include "Aclapi.h"
#include "SLogger.h"

#pragma comment(lib, "psapi.lib")
using std::cout; using std::endl;

char RegisterName[] = "SOFTWARE\\WOTKiller";

typedef struct _SRegInfo
{
	int		timeout;
	int		timeLimit;
	int		day;
	bool	enable;

	bool	valid;
} SRegInfo;

static SRegInfo s_GlobalInfo;

bool ConverToChar( SRegInfo& info, char* data );
bool ConverToInfo( char* data, SRegInfo& info );

void PrintProcessNameAndID( DWORD processID );
void ErrorExit(LPTSTR lpszFunction);

HANDLE Find(TCHAR* proc);
void Kill(HANDLE hProc);
void ShowErrorMessage(LPTSTR text);
void LogLastErrorMessage(LPCTSTR text);
bool CreateRegister(const char strKeyName[]);

bool ReadRegInfo(SRegInfo& info);
bool WriteRegInfo(SRegInfo& info);

int main( void )
{
	srand(time(NULL));
	CreateRegister(RegisterName);

	SRegInfo info = {123, 1000, 4, false};
	
	size_t s = sizeof(unsigned long long);
	s = sizeof(double);

	WriteRegInfo(info);
	ReadRegInfo(info);

	HANDLE hProcess = Find(TEXT("AIMP3.exe"));
	if(hProcess)
	{
		SLOG_DEBUG("Process %s founded!", "AIMP3.exe");
		Kill(hProcess);
		CloseHandle(hProcess);
	}
	else
	{
		SLOG_DEBUG("Process not founded.");
	}
	int a;
	std::cin >> a;
	return 0;
}
//-------------------------------------------------------------------------
HANDLE Find( TCHAR* proc )
{
	// Get the list of process identifiers.
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		cout << "Enum Processes Failed.  Error code: " << GetLastError() << endl;
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
void Kill( HANDLE hProc )
{
	DWORD fdwExit = 0;

	if(!GetExitCodeProcess(hProc, &fdwExit))
	{
		LogLastErrorMessage(TEXT("GetExitCodeProcess"));
	}

	//if(!TerminateProcess(hProc, fdwExit))
	//{
	//	LogLastErrorMessage(TEXT("Cannot kill process"));
	//	return;
	//}

	SLOG_TRACE("Process are killed");
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

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)text)+40)*sizeof(TCHAR)); 

	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s. Error code %d: %s"), 
		text, dw, lpMsgBuf);

	USES_CONVERSION;
	char* message = T2A((LPTSTR)lpDisplayBuf);

	SLOG_ERROR(message);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw); 
}
//-------------------------------------------------------------------------
void ShowErrorMessage( LPTSTR text )
{
	MessageBox(NULL, text, TEXT("Error"), MB_OK); 
}
//-------------------------------------------------------------------------
bool ReadRegInfo( SRegInfo& info )
{
	HKEY hKey;

	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegisterName, 0, KEY_ALL_ACCESS , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success opening key.");
	} 
	else 
	{
		SLOG_TRACE("Error opening key. Error code: %i", openRes);
	}

	LPCTSTR value = TEXT("data");

	char data[128] = {0};
	DWORD Type;

	DWORD size = 63;
	LONG setRes = RegQueryValueEx( hKey, value, NULL, &Type, (LPBYTE)data, &size);

	ConverToInfo(data, info);

	if (setRes == ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success reading to Registry.");
	} 
	else 
	{
		SLOG_TRACE("Error reading from Registry. Error code: %i", openRes);
	}

	LONG closeOut = RegCloseKey(hKey);

	if (closeOut == ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success closing key.");
	} 
	else 
	{
		SLOG_TRACE("Error closing key. Error code: %i", openRes);
	}

	return true;
}
//-------------------------------------------------------------------------
bool WriteRegInfo( SRegInfo& info )
{
	HKEY hKey;

	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegisterName, 0, KEY_ALL_ACCESS , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success opening key.");
	} 
	else 
	{
		SLOG_TRACE("Error opening key. Error code: %i", openRes);
	}

	LPCTSTR value = TEXT("data");
	char str[128] = {0};
	ConverToChar(info, str);
	LPCTSTR data = TEXT(str) ;

	LONG setRes = RegSetValueEx (hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data)+1);

	if (setRes == ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success writing to Registry.");
	} 
	else 
	{
		SLOG_TRACE("Error writing to Registry. Error code: %i", openRes);
	}

	LONG closeOut = RegCloseKey(hKey);

	if (closeOut == ERROR_SUCCESS) 
	{
		SLOG_TRACE("Success closing key.");
	} 
	else 
	{
		SLOG_TRACE("Error closing key. Error code: %i", openRes);
	}

	return true;
}
//-------------------------------------------------------------------------
bool CreateRegister(const char strKeyName[]) 
{	
	HKEY hKey = NULL;
	bool retValue = true;

	//Step 1: Open the key
	long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_READ, &hKey);

	//Step 2: If failed, create the key
	if (ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts)
	{
		SLOG_TRACE("Creating registry key %s", strKeyName);
		
		long retError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, strKeyName, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );

		if ( retError != ERROR_SUCCESS)
		{
			SLOG_ERROR("Could not create registry key %. Error code %i", strKeyName, retError);
		}
		else
		{
			SLOG_TRACE("Key created");
		}
		retValue = false;
	}
	else if (ERROR_SUCCESS != sts)
	{
		SLOG_ERROR("Cannot open registry key %s. Error code %i", strKeyName, sts);
		retValue = false;
	}
	else //If it already existed, get the value from the key.
	{
		SLOG_WARNING("Registry key already exist %s.", strKeyName);
	}

	RegCloseKey(hKey);

	return retValue;
}
//-------------------------------------------------------------------------
bool ConverToChar( SRegInfo& info, char* data )
{
	int seed = rand()%7345734;
	int hash = (info.timeout << 4) + (info.timeLimit << 3)  + (info.day << 2) + (info.enable << 1);
	
	srand(seed);

	int r1 = (rand()%11);
	int r2 = (rand()%11);
	int r3 = (rand()%11);
	int r4 = (rand()%11);

	sprintf_s(data, 128, "%x:%o:%o:%o:%x:%d", seed, 
		info.timeout	<< r1, 
		info.timeLimit	<< r2, 
		info.day		<< r3, 
		info.enable		<< r4, hash);
	return true;
}
//-------------------------------------------------------------------------
bool ConverToInfo( char* data, SRegInfo& info )
{
	int seed;
	int testHash;
	
	sscanf_s(data, "%x:%o:%o:%o:%x:%d", &seed, &info.timeout, &info.timeLimit, &info.day, &info.enable, &testHash );
	
	srand(seed);

	int r1 = (rand()%11);
	int r2 = (rand()%11);
	int r3 = (rand()%11);
	int r4 = (rand()%11);

	info.timeout	= info.timeout		>> r1;
	info.timeLimit	= info.timeLimit	>> r2;
	info.day		= info.day			>> r3;
	info.enable		= info.enable		>> r4;

	int hash = (info.timeout << 4) + (info.timeLimit << 3)  + (info.day << 2) + (info.enable << 1);

	info.valid = (testHash == hash);

	return true;
}
