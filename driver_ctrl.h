#pragma once
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <winternl.h>
#include <strsafe.h>
#pragma comment(lib,"ntdll")
#include "options.h"

void ErrorExit(const char* lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

namespace Bypass
{
	namespace Installer
	{
		NTSTATUS NTAPI InstallService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpPath)
		{

			NTSTATUS result = NULL;
			SC_HANDLE hSC = NULL, hService = NULL;
			SERVICE_STATUS ss;
			LPCSTR Args[1];
			ZeroMemory(&ss, sizeof(SERVICE_STATUS));
			ZeroMemory(Args, 1);
			hSC = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
			if (!hSC)
			{
				std::cout << "!hSC" << std::endl;
				ErrorExit("OpenSCManagerA");
				result = 1;
				goto END;
			}
			hService = OpenServiceA(hSC, lpServiceName, SERVICE_ALL_ACCESS);
			if (hService == NULL)
			{
				std::cout << "hService == NULL" << std::endl;
				hService = CreateServiceA(hSC, lpServiceName, lpDisplayName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, lpPath, NULL, NULL, NULL, NULL, NULL);
				if (hService == NULL)
				{
					std::cout << "!hSC" << std::endl;
					ErrorExit("CreateServiceA");
					result = 2;
					goto END;
				}
			}

			if (!QueryServiceStatus(hService, &ss))
			{
				std::cout << "!QueryServiceStatus(hService, &ss)" << std::endl;
				ErrorExit("QueryServiceStatus");
				result = 3;
				goto END;
			}
			if (ss.dwCurrentState == SERVICE_STOPPED)
			{
				//std::cout << "ss.dwCurrentState == SERVICE_STOPPED" << std::endl;
				//std::cout << hService << std::endl;
				if (!StartServiceA(hService, 0, 0))
				{
					ErrorExit("StartServiceA");
					result = 4;
					goto END;
				}
			}
		END:
			if (hSC)
				CloseServiceHandle(hSC);
			if (hService)
				CloseServiceHandle(hService);

			return result;
		}
		NTSTATUS NTAPI UninstallService(LPCSTR lpServiceName)
		{

			BOOL Status = false;
			SC_HANDLE hService = 0, hSCM = 0;
			DWORD cbNeeded = 0;
			SERVICE_STATUS_PROCESS ss = { 0,0,0,0,0,0,0,0,0 };
			SERVICE_STATUS ssc = { 0,0,0,0,0,0,0 };

			hSCM = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
			if (hSCM == 0)
				goto EXIT;

			hService = OpenServiceA(hSCM, lpServiceName, SERVICE_ALL_ACCESS);

			if (hService != 0)
			{
				if (QueryServiceStatusEx(hService, static_cast<SC_STATUS_TYPE>(0), reinterpret_cast<LPBYTE>(&ss), sizeof(ss), &cbNeeded))
				{
					if (ss.dwCurrentState == SERVICE_RUNNING)
					{
						if (ControlService(hService, SERVICE_CONTROL_STOP, &ssc) == FALSE)
							goto EXIT;
						while (ss.dwCurrentState != SERVICE_STOPPED)
						{
							if (!QueryServiceStatusEx(hService, static_cast<SC_STATUS_TYPE>(0), reinterpret_cast<LPBYTE>(&ss), sizeof(ss), &cbNeeded))
								goto EXIT;
						}
					}
				}

				if (DeleteService(hService) == FALSE)
					goto EXIT;
				CloseServiceHandle(hService);
				while (true)
				{
					Sleep(120);
					hService = OpenServiceA(hSCM, lpServiceName, SERVICE_ALL_ACCESS);
					if (hService == 0)
						break;
				}
			}
			Status = true;
		EXIT:
			if (hService != 0)
				CloseServiceHandle(hService);
			if (hSCM != 0)
				CloseServiceHandle(hSCM);

			return Status;
		}

	}
	namespace Driver
	{
		HANDLE hFile = 0;

		bool OpenDriver(void)
		{

			HANDLE hDevice = CreateFileA("\\\\.\\Aspect", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				ErrorExit("CreateFileA");
				return false;
			}
			hFile = hDevice;

			return true;
		}

		bool CloseDriver()
		{
			return CloseHandle(hFile);
		}



		bool SuspendEACThreads(DWORD ProcessId)
		{

			auto GetModuleFromAddr = [](DWORD_PTR dwAddr, DWORD dwPid)->DWORD
			{
				HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
				DWORD dwModuleBaseAddress = 0;
				if (hSnapshot != INVALID_HANDLE_VALUE)
				{
					MODULEENTRY32 ModuleEntry32 = { 0 };
					ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
					if (Module32First(hSnapshot, &ModuleEntry32))
					{
						do
						{
							if (dwAddr >= reinterpret_cast<DWORD_PTR>(ModuleEntry32.modBaseAddr) &&
								(reinterpret_cast<DWORD_PTR>(ModuleEntry32.modBaseAddr) + ModuleEntry32.modBaseSize) > dwAddr)
								dwModuleBaseAddress = reinterpret_cast<DWORD_PTR>(ModuleEntry32.modBaseAddr);
						} while (Module32Next(hSnapshot, &ModuleEntry32));
					}
					CloseHandle(hSnapshot);
				}
				//std::cout << "Base Address: " << dwModuleBaseAddress << std::endl;
				return dwModuleBaseAddress;
			};

			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, ProcessId);
			if (hSnapshot == INVALID_HANDLE_VALUE)
				return FALSE;
			THREADENTRY32 te32 = { 0,0,0,0,0,0,0 };
			te32.dwSize = sizeof(THREADENTRY32);
			if (!Thread32First(hSnapshot, &te32))
			{
				CloseHandle(hSnapshot);
				return FALSE;
			}

			do
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, false, te32.th32ThreadID);
				if (hThread)
				{
					std::cout << "Thread opened : " << te32.th32ThreadID << std::endl;
					DWORD_PTR dwStartAddress = 0;
					if (NT_SUCCESS(NtQueryInformationThread(hThread, static_cast<THREADINFOCLASS>(9), &dwStartAddress, sizeof(DWORD_PTR), NULL)))
					{
						std::cout << "Thread Addr : " << reinterpret_cast<void*>(dwStartAddress) << " : " << te32.th32ThreadID << std::endl;
						if (GetModuleFromAddr(dwStartAddress, ProcessId) == 0)
							SuspendThread(hThread);
						CloseHandle(hSnapshot);
					}
					CloseHandle(hThread);
				}
			} while (Thread32Next(hSnapshot, &te32));

			CloseHandle(hSnapshot);

			return false;
		}

		DWORD GetProtectedId()
		{

			typedef struct GETPROC_STRUCT
			{
				ULONG pId;
			}GETPROC_STRUCT, * PGETPROC_STRUCT;
			GETPROC_STRUCT GP;
			GP.pId = 0;
			if (!DeviceIoControl(hFile, CTL_CODE(FILE_DEVICE_UNKNOWN, 0xad139, METHOD_BUFFERED, FILE_SPECIAL_ACCESS), &GP, sizeof(GP), 0, 0, NULL, NULL))
				return 0;

			return GP.pId;
		}

		bool ProtectProcess(DWORD ProcessId)
		{

			typedef struct HIDEPROC_STRUCT
			{
				ULONG pId;
			}HIDEPROC_STRUCT, * PHIDEPROC_STRUCT;
			HIDEPROC_STRUCT HP;
			HP.pId = ProcessId;
			if (!DeviceIoControl(hFile, CTL_CODE(FILE_DEVICE_UNKNOWN, 0xad138, METHOD_BUFFERED, FILE_SPECIAL_ACCESS), &HP, sizeof(HP), 0, 0, NULL, NULL))
				return false;

			return true;
		}

		bool IsBypassed()
		{

			typedef struct IS_ENABLED_STRUCT
			{
				BOOLEAN IsEnabled;
			}IS_ENABLED_STRUCT, * PIS_ENABLED_STRUCT;
			IS_ENABLED_STRUCT IS;
			IS.IsEnabled = false;
			if (!DeviceIoControl(hFile, CTL_CODE(FILE_DEVICE_UNKNOWN, 0xad136, METHOD_BUFFERED, FILE_SPECIAL_ACCESS), &IS, sizeof(IS), &IS, sizeof(IS), NULL, NULL))
				return false;

			return IS.IsEnabled;
		}
	}
}