// This is the main DLL file.

#include "stdafx.h"

#include "Sloth.h"

#include <tchar.h>
#include <strsafe.h>
#include <psapi.h>
#include <atlstr.h>

#include "SLogger.h"

namespace Sloth
{
	bool ReadRegInfo( const char regName[], const char field[], char* data)
	{
		HKEY hKey;

		LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS , &hKey);

		if (openRes==ERROR_SUCCESS) 
		{
			SLOG_TRACE("Success opening key.");
		} 
		else 
		{
			SLOG_TRACE("Error opening key. Error code: %i", openRes);
		}

		LPCTSTR value = TEXT(field);

		DWORD Type;
		DWORD size = 128;
		LONG setRes = RegQueryValueEx( hKey, value, NULL, &Type, (LPBYTE)data, &size);

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
	bool WriteRegInfo( const char regName[], const char field[], const char data[] )
	{
		HKEY hKey;

		LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS , &hKey);

		if (openRes==ERROR_SUCCESS) 
		{
			SLOG_TRACE("Success opening key.");
		} 
		else 
		{
			SLOG_TRACE("Error opening key. Error code: %i", openRes);
		}

		LPCTSTR value = TEXT(field);
		LPCTSTR dataValue = TEXT(data) ;

		LONG setRes = RegSetValueEx (hKey, value, 0, REG_SZ, (LPBYTE)dataValue, strlen(dataValue)+1);

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
	HANDLE Find( TCHAR* proc )
	{
		// Get the list of process identifiers.
		DWORD aProcesses[1024], cbNeeded, cProcesses;
		unsigned int i;

		if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		{
			SLOG_ERROR( "Enum Processes Failed.  Error code: %d", GetLastError());
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
}