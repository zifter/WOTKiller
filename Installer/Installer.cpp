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
#include "SLogger.h"
//-------------------------------------------------------------------------
bool CheckPassword();
void CreatePassword();
void CreateNewRegInfo();
//-------------------------------------------------------------------------
int main()
{
	if(!Sloth::IsRegExist(Sloth::RegisterName))
	{
		Sloth::CreateRegisterDefault();
	}

	if(!Sloth::IsPassExist())
	{
		CreatePassword();
	}

	if(!CheckPassword())
	{
		std::cout << "Fuck you! :)" << std::endl;
		return 0;
	}

	CreateNewRegInfo();
	std::cout << "Good buy!" << std::endl;

	_getch();
	return 0;
}
//-------------------------------------------------------------------------
bool CheckPassword()
{
	SLOG_TRACE("Check pass");

	char pass[STR_SIZE];

	std::cout << "Password:\n";

	std::cin.getline(pass, STR_SIZE, '\n');

	char data[STR_SIZE];

	Sloth::GetPass(data);

	return (strcmp(data, pass) == 0);
}
//-------------------------------------------------------------------------
void CreatePassword()
{
	char pass[STR_SIZE];

	std::cout << "Create password:" << std::endl;

	std::cin.getline(pass, STR_SIZE, '\n');

	SLOG_TRACE("Set new pass");

	Sloth::SetPass(pass);
}
//-------------------------------------------------------------------------
void CreateNewRegInfo()
{
	SLOG_TRACE("Create new registrt info");

	std::cout << "Are you want to enable? (y)es/(n)o" << std::endl;

	char enabled = true;
	char enabledStr[128];
	std::cin.getline(enabledStr, 128, '\n');

	if(strcmp(enabledStr, "n")==0)
	{
		enabled = false;
	}

	std::cout << "\nHow long are you allowed to play? Please, enter time in minutes." << std::endl;

	int time;
	std::cin >> time;

	if(time > 12440)
	{
		time = 12440;
	}
	if(time < 0)
	{
		time = 0;
	}

	Sloth::SRegInfo info = Sloth::DefaultRegInfo;
	info.timeLimit = time*60;
	info.enable = enabled;

	Sloth::LogRegInfo(info);
	Sloth::SetRegInfo(info);
}
