// Installer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//-------------------------------------------------------------------------
#include <Shlobj.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
//-------------------------------------------------------------------------
#include "Sloth.h"
bool CheckPassword();
void CreatePassword();
//-------------------------------------------------------------------------
int main()
{
	if(Sloth::IsRegExist(Sloth::RegisterName))
	{
		while(CheckPassword())
		{
			Sleep(1000);
		}
	}
	else
	{
		Sloth::CreateRegisterDefault();
		CreatePassword();
	}

	_getch();
	return 0;
}
//-------------------------------------------------------------------------
bool CheckPassword()
{
	char pass[1024];

	std::cout << "Password:\n";

	std::cin.getline(pass, 1024, '\n');

	char data[128];

	Sloth::ReadRegInfo(Sloth::RegisterName, "stash", data);

	return false;
}
//-------------------------------------------------------------------------
void CreatePassword()
{
	char pass[128];

	std::cout << "Create password:" << std::endl;

	std::cin.getline(pass, 128, '\n');

	if(Sloth::WriteRegInfo(Sloth::RegisterName, "stash", pass))
	{
		std::cout << "Thank you!" << std::endl;
	}
	else
	{
		std::cout << "Fuck you! :)" << std::endl;
	}
}
