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
#include "Aclapi.h"

#pragma comment(lib, "psapi.lib")
using std::cout; using std::endl;

void PrintProcessNameAndID( DWORD processID );
void ErrorExit(LPTSTR lpszFunction);

int main( void )
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

	// Print the name and process identifier for each process.

	for ( i = 0; i < cProcesses; i++ )
	{
		if( aProcesses[i] != 0 )
		{
			PrintProcessNameAndID( aProcesses[i] );
		}
	}

	int a;
	std::cin >> a;
	return 0;
}

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void PrintProcessNameAndID( DWORD processID )
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE |
		PROCESS_VM_READ,
		FALSE, processID );

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

	// Print the process name and identifier.
	if(_tcscmp(szProcessName, TEXT("<unknown>")))
	{
		_tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, processID );
		if(!_tcscmp(szProcessName, TEXT("AIMP3.exe")))
		{
			DWORD fdwExit = 0;
			GetExitCodeProcess(hProcess, &fdwExit);
			BOOL flag = TerminateProcess(hProcess, fdwExit);

			if(!flag)
			{
				ErrorExit(TEXT("PrintProcessNameAndID"));
			}
		}
	}


	// Release the handle to the process.

	CloseHandle( hProcess );
}

void ErrorExit(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

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
		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"), 
		lpszFunction, dw, lpMsgBuf); 
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw); 
}