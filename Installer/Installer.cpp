// Installer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//-------------------------------------------------------------------------
#include <Shlobj.h>
#include <iostream>
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <atlstr.h>
#include <ctime>
//-------------------------------------------------------------------------
bool IsInstalled();
bool CheckPassword();
void CreatePassword();
//-------------------------------------------------------------------------
const char s_RegisterName[] = "SOFTWARE\\WOTKiller";
//-------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	if(IsInstalled())
	{
		while(CheckPassword())
		{
			Sleep(1000);
		}
	}
	else
	{
		CreatePassword();
	}
	return 0;
}
//-------------------------------------------------------------------------
bool IsInstalled()
{
	HKEY hKey = NULL;
	bool retValue = true;

	long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, s_RegisterName, 0, KEY_READ, &hKey);

	return !(ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts) && (ERROR_SUCCESS != sts);
}
//-------------------------------------------------------------------------
bool CheckPassword()
{
	char pass[1024];

	std::cout << "Password:\n";

	std::cin.getline(pass, 1024, '\n');

	return false;
}
//-------------------------------------------------------------------------
void CreatePassword()
{
	char pass[1024];

	std::cout << "Input password:\n";

	std::cin.getline(pass, 1024, '\n');
}
//-------------------------------------------------------------------------
bool CreateRegister() 
{	
	HKEY hKey = NULL;
	bool retValue = true;

	//Step 1: Open the key
	long sts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, s_RegisterName, 0, KEY_READ, &hKey);

	//Step 2: If failed, create the key
	if (ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts)
	{
		long retError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, s_RegisterName, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL );

		if ( retError != ERROR_SUCCESS)
		{
			std::cout << "Could not create registry key. Error code " << retError << std::endl;
		}
		else
		{
			std::cout << "Key created" << std::endl;
		}
		retValue = false;
	}
	else if (ERROR_SUCCESS != sts)
	{
		std::cout << "Cannot open registry key. Error code " << sts << std::endl;
		retValue = false;
	}
	else //If it already existed, get the value from the key.
	{
		std::cout <<  "Registry key already exist.";
	}

	RegCloseKey(hKey);

	return retValue;
}