#include "main.h"

// \SystemRoot\Temp\UberBeacon.log

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK ISB;
	OBJECT_ATTRIBUTES p;
	UNICODE_STRING fileName;

	RtlInitUnicodeString(&fileName, L"\\SystemRoot\\Temp\\UberBeacon.log");

	InitializeObjectAttributes(&p, &fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	// 1st try
	
	thatFile = ExAllocatePoolZero(NonPagedPool, sizeof(HANDLE), 'enim');
	status = ZwCreateFile(thatFile, GENERIC_ALL, &p, &ISB, NULL, FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN, SHARE_MODE, FILE_OPEN_IF, FILE_WRITE_THROUGH, NULL, 0);
	if (!NT_SUCCESS(status)) {
		ExFreePoolWithTag(thatFile, 'enim');
		thatFile = NULL;
		return status;
	}

	isCreated = ISB.Information == FILE_CREATED;

	status = PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, FALSE);
	DriverObject->DriverUnload = UnloadHandler;
	return status;
}

NTSTATUS NTAPI UnloadHandler(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	if(thatFile) {
		PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, TRUE);
		ZwClose(*thatFile);
		ExFreePoolWithTag(thatFile, 'enim');
	}
	
	return STATUS_SUCCESS;
}

VOID ProcessNotifyCallback(
	IN HANDLE ParentId,
	IN HANDLE ProcessId,
	IN BOOLEAN Create
)
{
	UNREFERENCED_PARAMETER(ParentId);
	PEPROCESS processObject = NULL;
	NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &processObject);

	if (NT_SUCCESS(status) && processObject)
	{
		// Get image name (short version, typically the file name only)
		char* imageName = PsGetProcessImageFileName(processObject);

		if (Create)
		{
			DbgPrint("Process created: %p, Image name: %s\n", ProcessId, imageName);
			if (!strcmp(imageName, "userinit.exe")) {
				char* strTemp;
				strTemp = ExAllocatePoolZero(NonPagedPool, 128, 'enim');
				if (!strTemp) return;
				LARGE_INTEGER time100, appender;
				TIME_FIELDS easyTime;
				IO_STATUS_BLOCK ISB;
				appender.HighPart = -1; appender.LowPart = FILE_WRITE_TO_END_OF_FILE;
				KeQuerySystemTime(&time100);
				RtlTimeToTimeFields(&time100, &easyTime);
				status = RtlStringCbPrintfA(strTemp, 128, "%s %s %02d %04d %02d:%02d:%02d GMT+0000 (Coordinated Universal Time) - wininit.exe executed!\r\n",
					day[(easyTime.Weekday >= 0 && easyTime.Weekday <= 6) ? easyTime.Weekday : 0], mon[(easyTime.Month >= 1 && easyTime.Month <= 12)?easyTime.Month-1:0],
					easyTime.Day, easyTime.Year, easyTime.Hour, easyTime.Minute, easyTime.Second);
				DbgPrint(strTemp);
				if (NT_SUCCESS(status) && thatFile) {
					size_t buflen;// = (ULONG)strlen(strTemp);
					status = RtlStringCbLengthA(strTemp, 128, &buflen);

					status = ZwWriteFile(*thatFile, NULL, NULL, NULL, &ISB, strTemp, (ULONG)buflen, &appender, NULL);
					DbgPrint("ZwWriteFile: %0x", status);
				}

				ExFreePoolWithTag(strTemp, 'enim');
				if (isCreated) {
					if (thatFile) {
						ZwClose(*thatFile);
						ExFreePoolWithTag(thatFile, 'enim');
					}
					KeBugCheckEx(/*0xD4C0FFEE*/0x29, 0, 0, 0, 0);
					return;
				}
			}
		}
		else
		{
			DbgPrint("Process terminated: %p, Image name: %s\n", ProcessId, imageName);
		}

		// Dereference the process object after using it
		ObDereferenceObject(processObject);
	}
}
