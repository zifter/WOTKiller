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

#include "Aclapi.h"
#include "SLogger.h"

#pragma comment(lib, "psapi.lib")
using std::cout; using std::endl;

typedef struct _SRegInfo
{
	float timeout;
} SRegInfo;

void PrintProcessNameAndID( DWORD processID );
void ErrorExit(LPTSTR lpszFunction);

HANDLE Find(TCHAR* proc);
void Kill(HANDLE hProc);
void ShowErrorMessage(LPTSTR text);
void LogLastErrorMessage(LPCTSTR text);

bool ReadRegInfo(SRegInfo& info);
bool WriteRegInfo(SRegInfo& info);

int main( void )
{
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

	if(!TerminateProcess(hProc, fdwExit))
	{
		LogLastErrorMessage(TEXT("Cannot kill process"));
		return;
	}

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
	return true;
}
//-------------------------------------------------------------------------
bool WriteRegInfo( SRegInfo& info )
{
	return true;
}