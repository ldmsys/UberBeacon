#pragma once
#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>

#ifndef NDEBUG
#define SHARE_MODE FILE_SHARE_READ
#else
#defien SHARE_MODE 0
#endif


NTSTATUS NTAPI PsLookupProcessByProcessId(IN HANDLE 	ProcessId,
	OUT PEPROCESS* Process
);
LPSTR NTAPI PsGetProcessImageFileName(PEPROCESS Process); // Undocumented API

BOOLEAN isCreated = FALSE;
PHANDLE thatFile = NULL;
NTSTATUS NTAPI UnloadHandler(_In_ PDRIVER_OBJECT DriverObject);
VOID ProcessNotifyCallback(
	IN HANDLE ParentId,
	IN HANDLE ProcessId,
	IN BOOLEAN Create
);
char* day[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char* mon[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };