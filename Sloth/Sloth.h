// Sloth.h

#pragma once

#include <windows.h>

namespace Sloth {

	//process
	HANDLE Find(TCHAR* proc);
	void Kill(HANDLE hProc);

	// registry
	bool ReadRegInfo(const char regName[], const char field[], char* data);
	bool WriteRegInfo(const char regName[], const char field[] , const char data[]);

	// For logger
	void LogLastErrorMessage(LPCTSTR text);
}
