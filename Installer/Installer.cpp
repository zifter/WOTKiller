// Installer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//-------------------------------------------------------------------------
#include <iostream>
#include <windows.h>
//-------------------------------------------------------------------------
bool IsInstalled();
bool CheckPassword();
void CreatePassword();
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
	return true;
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

}

