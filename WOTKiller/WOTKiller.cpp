// WOTKiller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>
#include <tchar.h>

#include "SLogger.h"
#include "Sloth.h"

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);
VOID WINAPI GrantShutdownPrivileges();

#define SERVICE_NAME  _T("WifeControl")
#define PROCESS_NAME  ("WorldOfTanks.exe")

int _tmain (int argc, TCHAR *argv[])
{
	SLOG_DEBUG("Service: Main: Entry");

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
		SLOG_ERROR("Service: Main: StartServiceCtrlDispatcher returned error");
		return GetLastError ();
    }

    SLOG_DEBUG("Service: Main: Exit");
    return 0;
}


VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    SLOG_DEBUG("Service: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        SLOG_ERROR("Service: ServiceMain: RegisterServiceCtrlHandler returned error");
        goto EXIT;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        SLOG_ERROR("Service: ServiceMain: SetServiceStatus returned error");
    }

    SLOG_DEBUG("Service: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        SLOG_ERROR("Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
	    {
		    SLOG_ERROR("Service: ServiceMain: SetServiceStatus returned error");
	    }
        goto EXIT; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    SLOG_ERROR("Service: ServiceMain: SetServiceStatus returned error");
    }

 	GrantShutdownPrivileges();

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    SLOG_TRACE("Service: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
    
    SLOG_DEBUG("Service: ServiceMain: Worker Thread Stop Event signaled");

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    SLOG_ERROR("Service: ServiceMain: SetServiceStatus returned error");
    }
    
    EXIT:
    SLOG_DEBUG("Service: ServiceMain: Exit");

    return;
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    SLOG_DEBUG("Service: ServiceCtrlHandler: Entry");

    switch (CtrlCode) 
	{
     case SERVICE_CONTROL_STOP :

        SLOG_DEBUG("Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			SLOG_ERROR("Service: ServiceCtrlHandler: SetServiceStatus returned error");
		}

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    SLOG_DEBUG("Service: ServiceCtrlHandler: Exit");
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    SLOG_DEBUG("Service: ServiceWorkerThread: Entry");

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {       

		if(!Sloth::IsRegExist(Sloth::RegisterName))
		{
			Sloth::CreateRegisterDefault();
		}

		Sloth::SRegInfo cInfo;
		Sloth::GetRegInfo(cInfo);
		Sloth::LogRegInfo(cInfo);

		HANDLE hProcess = Sloth::Find(TEXT(PROCESS_NAME));

		if(hProcess)
		{
			SLOG_TRACE("Process %s founded!", PROCESS_NAME);
			cInfo.Update();
							
			if(cInfo.IsNeedToKill())
			{
				Sloth::Kill(hProcess);

				DWORD status = InitiateShutdown(
					NULL,
					NULL,
					0,
					SHUTDOWN_FORCE_OTHERS |
					SHUTDOWN_NOREBOOT,
					SHTDN_REASON_MAJOR_APPLICATION
					);
				
				if(status == ERROR_SUCCESS)
				{
					SLOG_ERROR("Cannot shutdown! Error code %", GetLastError());
				}

				CloseHandle(hProcess);
			}

			LogRegInfo(cInfo);
			SetRegInfo(cInfo);
		}
		else
		{
			SLOG_TRACE("Process not founded.");
		}

		Sleep(cInfo.timeCheck*1000);
    }

    SLOG_DEBUG("Service: ServiceWorkerThread: Exit");

    return ERROR_SUCCESS;
}
//-------------------------------------------------------------------------
VOID WINAPI GrantShutdownPrivileges()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 

	OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&hToken
		); 

	LookupPrivilegeValue(
		NULL,
		SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid
		); 

	tkp.PrivilegeCount = 1;

	// Get shutdown privilege for this process. 
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(
		hToken,
		FALSE, 
		&tkp, 
		0, 
		(PTOKEN_PRIVILEGES) NULL, 
		0)
		)
	{
		SLOG_ERROR("AdjustTokenPrivileges %d", GetLastError());
	}
}
//-------------------------------------------------------------------------
