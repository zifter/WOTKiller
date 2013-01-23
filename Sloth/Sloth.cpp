// This is the main DLL file.

#include "stdafx.h"

#include "Sloth.h"

#include <Shlobj.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <psapi.h>
#include <atlstr.h>
#include <ctime>

#include "Aclapi.h"
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
}