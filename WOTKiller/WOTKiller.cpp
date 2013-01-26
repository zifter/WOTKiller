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

LPTSTR ServiceName = TEXT("WOTKiller");
SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus;

void StartService();
void RemoveService();
void InstallService(LPTSTR path);
void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);

bool InitService();
bool ServiceProceed();

void ShowErrorMessage(LPTSTR text);
void KillProcess();
void UpdateInfo(Sloth::SRegInfo& info);
bool IsNeedToKill(Sloth::SRegInfo& info);

int _tmain(int argc, _TCHAR* argv[]) 
{
	SLOG_TRACE("_tmain");

	if(argc - 1 == 0) 
	{
		SERVICE_TABLE_ENTRY ServiceTable[1];
		ServiceTable[0].lpServiceName = ServiceName;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

		if(!StartServiceCtrlDispatcher(ServiceTable)) 
		{
			SLOG_ERROR("Error: StartServiceCtrlDispatcher");
		}
	} 
	else if( _tcscmp(argv[argc-1], TEXT("install")) == 0) 
	{
		InstallService(argv[0]);
	} 
	else if( _tcscmp(argv[argc-1], TEXT("remove")) == 0) 
	{
		RemoveService();
	} 
	else if( _tcscmp(argv[argc-1], TEXT("start")) == 0 )
	{
		StartService();
	}
}

void ServiceMain(int argc, char** argv) 
{
	SLOG_DEBUG("ServiceMain");

	int error; 
	int i = 0;

	ServiceStatus.dwServiceType		= SERVICE_WIN32_OWN_PROCESS; 
	ServiceStatus.dwCurrentState    = SERVICE_START_PENDING; 
	ServiceStatus.dwControlsAccepted= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode   = 0; 
	ServiceStatus.dwServiceSpecificExitCode = 0; 
	ServiceStatus.dwCheckPoint		= 0; 
	ServiceStatus.dwWaitHint		= 0; 

	hStatus = RegisterServiceCtrlHandler(ServiceName, (LPHANDLER_FUNCTION)ControlHandler); 
	if (hStatus == (SERVICE_STATUS_HANDLE)0) 
	{ 
		return; 
	} 

	error = InitService(); 
	if (error) 
	{
		ServiceStatus.dwCurrentState    = SERVICE_STOPPED; 
		ServiceStatus.dwWin32ExitCode   = -1; 
		SetServiceStatus(hStatus, &ServiceStatus); 
		return; 
	} 

	ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
	SetServiceStatus (hStatus, &ServiceStatus);

	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		if (ServiceProceed())  
		{
			ServiceStatus.dwCurrentState    = SERVICE_STOPPED; 
			ServiceStatus.dwWin32ExitCode   = -1; 
			SetServiceStatus(hStatus, &ServiceStatus);
			return;
		}
	}
}
//-------------------------------------------------------------------------
void ControlHandler(DWORD request) 
{ 
	SLOG_TRACE("ControlHandler");

	switch(request) 
	{ 
	case SERVICE_CONTROL_STOP: 
		SLOG_TRACE("Stopped.");

		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
		SetServiceStatus (hStatus, &ServiceStatus);
		return; 

	case SERVICE_CONTROL_SHUTDOWN: 
		SLOG_TRACE("Shutdown.");

		ServiceStatus.dwWin32ExitCode = 0; 
		ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
		SetServiceStatus (hStatus, &ServiceStatus);
		return; 

	default:
		break;
	} 

	SetServiceStatus (hStatus, &ServiceStatus);

	return; 
} 
//-------------------------------------------------------------------------
void StartService( void )
{
	SLOG_TRACE("Start service");

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_START);
	if(!StartService(hService, 0, NULL)) 
	{
		CloseServiceHandle(hSCManager);
		SLOG_ERROR("Error: Can't start service");
		return;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return;
}
//-------------------------------------------------------------------------
void RemoveService()
{
	SLOG_TRACE("Remove service");

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager) 
	{
		SLOG_ERROR("Error: Can't open Service Control Manager");
		return;
	}

	SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_STOP | DELETE);
	if(!hService) 
	{
		SLOG_ERROR("Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	SLOG_TRACE("Success remove service!");

	return;
}
//-------------------------------------------------------------------------
void InstallService(LPTSTR path)
{
	SLOG_TRACE("Install service");

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if(!hSCManager) 
	{
		SLOG_ERROR("Error: Can't open Service Control Manager");
		return;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		ServiceName,
		ServiceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		path,
		NULL, NULL, NULL, NULL, NULL
		);

	if(!hService) 
	{
		int err = GetLastError();
		switch(err) 
		{
		case ERROR_ACCESS_DENIED: 
			SLOG_ERROR("Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			SLOG_ERROR("Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			SLOG_ERROR("Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			SLOG_ERROR("Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			SLOG_ERROR("Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			SLOG_ERROR("Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			SLOG_ERROR("Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			SLOG_ERROR("Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			SLOG_ERROR("Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return;
	}
	CloseServiceHandle(hService);

	CloseServiceHandle(hSCManager);
	SLOG_TRACE("Success install service!");
	return;
}
//-------------------------------------------------------------------------
void ShowErrorMessage( LPTSTR text )
{
	MessageBox(NULL, text, TEXT("Error"), MB_OK); 
}
//-------------------------------------------------------------------------
void KillProcess()
{
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
}
//-------------------------------------------------------------------------
bool IsNeedToKill( Sloth::SRegInfo& info )
{
	return info.valid || ((info.timeout > info.timeCheck) && info.enable);
}
//-------------------------------------------------------------------------
void UpdateInfo(Sloth::SRegInfo& info)
{
	if(!info.valid)
	{
		info = Sloth::DefaultRegInfo;
	}

	time_t rawtime;
	struct tm * timeinfo = new struct tm;

	time ( &rawtime );
	localtime_s(timeinfo, &rawtime );

	if(timeinfo->tm_mday != info.day)
	{
		info.day = timeinfo->tm_mday;
		info.timeout = 0;
	}
	info.timeout += info.timeCheck;
}
//-------------------------------------------------------------------------
bool InitService()
{
	SLOG_TRACE("Init Service");
	return true;
}
//-------------------------------------------------------------------------
bool ServiceProceed()
{
	SLOG_TRACE("ServiceProceed");

	srand(time(NULL));
	
	char data[STR_SIZE];

	if(!Sloth::IsRegExist(Sloth::RegisterName))
	{
		Sloth::CreateRegisterDefault();
	}

	Sloth::SRegInfo info;
	Sloth::GetRegInfo(info);

	UpdateInfo(info);

	if(IsNeedToKill(info))
	{
		KillProcess();
	}

	SetRegInfo(info);

	Sleep(info.timeCheck);

	return true;
}
