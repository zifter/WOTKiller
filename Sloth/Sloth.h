// Sloth.h

#pragma once

#include <windows.h>

#define  STR_SIZE	256

namespace Sloth {

	static const char RegisterName[] = "SOFTWARE\\WOTKiller";

	typedef struct _SRegInfo
	{
		/* How long has it.	*/
		int		timeout; 
		/* How long you can play.	*/
		int		timeLimit;
		/* Is service enabled.	*/
		int		timeCheck;
		/* Current number of day.	*/
		int		day;
		/* Is service enabled.	*/
		bool	enable;

		/* This reg info is valid.	*/
		bool	valid;
	} SRegInfo;

	static SRegInfo DefaultRegInfo = {0, 0, 60, -1, true, true};

	bool ConverToChar( SRegInfo& info, char data[STR_SIZE] );
	bool ConverToInfo( char data[STR_SIZE], SRegInfo& info );

	//process
	HANDLE Find(TCHAR* proc);
	void Kill(HANDLE hProc);

	bool GetRegInfo(SRegInfo& info);
	bool SetRegInfo(SRegInfo& info);
	void CreateRegisterDefault();


	// registry
	bool IsRegExist(const char regName[]);
	bool CreateRegister(const char regName[]); 
	bool ReadRegInfo(const char regName[], const char field[], char data[STR_SIZE]);
	bool WriteRegInfo(const char regName[], const char field[] , const char data[STR_SIZE]);

	// For logger
	void LogLastErrorMessage(LPCTSTR text);
}
