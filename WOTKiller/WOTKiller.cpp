// WOTKiller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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
#include "Sloth.h"

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

void ShowErrorMessage(LPTSTR text);

int main( void )
{
	srand(time(NULL));

	SRegInfo info = {123, 1000, 4, false};
	
	char data1[128];
	ConverToChar(info, data1);
	Sloth::WriteRegInfo(RegisterName, "data", data1);
	
	char data2[128];
	Sloth::ReadRegInfo(RegisterName, "data", data2);
	ConverToInfo(data2, info);

	HANDLE hProcess = Sloth::Find(TEXT("AIMP3.exe"));
	if(hProcess)
	{
		SLOG_DEBUG("Process %s founded!", "AIMP3.exe");
		Sloth::Kill(hProcess);
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
void ShowErrorMessage( LPTSTR text )
{
	MessageBox(NULL, text, TEXT("Error"), MB_OK); 
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
