// Sloth.h

#pragma once

#include <windows.h>

namespace Sloth {

	static const char RegisterName[] = "SOFTWARE\\WOTKiller";

	//process
	HANDLE Find(TCHAR* proc);
	void Kill(HANDLE hProc);

	// registry
	bool IsRegExist(const char regName[]);
	bool CreateRegister(const char regName[]); 
	bool ReadRegInfo(const char regName[], const char field[], char* data);
	bool WriteRegInfo(const char regName[], const char field[] , const char data[]);

	// For logger
	void LogLastErrorMessage(LPCTSTR text);
}
