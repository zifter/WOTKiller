// This is the main DLL file.

#include "stdafx.h"

#include "Sloth.h"
#include <psapi.h>

#include <tchar.h>
#include <strsafe.h>

#include <atlstr.h>

#include "SLogger.h"

namespace Sloth
{
	//-------------------------------------------------------------------------
	bool IsRegExist(const char regName[])
	{
		SLOG_TRACE("Is register exist.");

		HKEY hKey = NULL;
		bool retValue = true;

		long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_READ, &hKey);

		SLOG_DEBUG("%d %d %d %d", sts, ERROR_NO_MATCH, ERROR_FILE_NOT_FOUND, ERROR_SUCCESS);

		return !(ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts);
	}
	//-------------------------------------------------------------------------
	bool ReadRegInfo( const char regName[], const char field[], char* data)
	{
		SLOG_TRACE("Read register info.");

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
			SLOG_DEBUG("Success reading to Registry.");
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
			SLOG_TRACE("Success closing key.");
		} 
		else 
		{
			LogLastErrorMessage("Error closing key.");
			bRet = false;
		}

		return bRet;
	}
	//-------------------------------------------------------------------------
	bool WriteRegInfo( const char regName[], const char field[], const char data[] )
	{
		SLOG_TRACE("Write register info.");

		HKEY hKey;
		bool bRet;
		LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regName, 0, KEY_ALL_ACCESS , &hKey);

		if (openRes==ERROR_SUCCESS) 
		{
			SLOG_TRACE("Success opening key.");
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
			SLOG_TRACE("Success writing to Registry.");
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
			SLOG_TRACE("Success closing key.");
		} 
		else 
		{
			LogLastErrorMessage("Error closing key.");
			bRet = false;
		}

		return bRet;
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
			TEXT("%s\n Error code %d: %s"), 
			text, dw, lpMsgBuf);

		USES_CONVERSION;
		char* message = T2A((LPTSTR)lpDisplayBuf);

		SLOG_ERROR(message);

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
		//ExitProcess(dw); 
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
}