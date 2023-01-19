#include <ntddk.h>
#include <wdf.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <ntstrsafe.h>
#define BUFFER_SIZE 100

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD KmdfHelloWorldEvtDeviceAdd;

HANDLE globProcessId = NULL, idToTerminate = NULL;

void PcreateProcessNotifyRoutineEx(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

VOID
MyDriverUnload(
	_In_
	WDFDRIVER Driver
);

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	// NTSTATUS variable to record success or failure
	NTSTATUS status = STATUS_SUCCESS;

	// Allocate the driver configuration object
	WDF_DRIVER_CONFIG config;

	// Print "Hello World" for DriverEntry
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Alexander Marjin\n"));

	// Initialize the driver configuration object to register the
	// entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
	WDF_DRIVER_CONFIG_INIT(&config,
		KmdfHelloWorldEvtDeviceAdd
	);

	config.EvtDriverUnload = MyDriverUnload;
	// Finally, create the driver object

	status = WdfDriverCreate(DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE
	);

	DbgPrint("Driver loaded!\n");

	NTSTATUS result;
	result = PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, FALSE);
	if (STATUS_SUCCESS == result) {
		DbgPrint("Added routine!\n");
	}
	else {
		DbgPrint("Failed to add routine! Error: %i\n", result);
	}

	return status;
}

void PcreateProcessNotifyRoutineEx(
	PEPROCESS				Process,
	HANDLE					ProcessId,
	PPS_CREATE_NOTIFY_INFO	CreateInfo
)
{
	PCUNICODE_STRING procName;
	UNREFERENCED_PARAMETER(Process);
	OBJECT_ATTRIBUTES fileAttributes;
	OBJECT_ATTRIBUTES sourceProjectAttributes;
	IO_STATUS_BLOCK ioStatusBlock;
	CLIENT_ID clientID;
	DWORD pidFromFile;
	NTSTATUS ntstatus;
	LARGE_INTEGER byteOffset;
	HANDLE fileHandle;
	HANDLE processToTerminate;
	CHAR bufferToWrite[BUFFER_SIZE], bufferToRead[BUFFER_SIZE];
	size_t cbLen;
	UNICODE_STRING sourceProcessName;

	if (CreateInfo == NULL)
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "One more process EXITED\n"));
		if (globProcessId == ProcessId) {
			DbgPrint("Our USERMODE process exited\n");

			RtlInitUnicodeString(&sourceProcessName, L"\\??\\E:\\check.txt");
			InitializeObjectAttributes(&fileAttributes, &sourceProcessName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
			ntstatus = ZwCreateFile(&fileHandle, GENERIC_READ, &fileAttributes, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
			if (NT_SUCCESS(ntstatus)) {
				byteOffset.LowPart = byteOffset.HighPart = 0;
				ntstatus = ZwReadFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, bufferToRead, BUFFER_SIZE, &byteOffset, NULL);
				if (NT_SUCCESS(ntstatus)) {
					bufferToRead[BUFFER_SIZE - 1] = '\0';
					pidFromFile = atoi(bufferToRead);
					InitializeObjectAttributes(&sourceProjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
					clientID.UniqueProcess = (HANDLE)pidFromFile;
					clientID.UniqueThread = NULL;
					ntstatus = ZwOpenProcess(&processToTerminate, DELETE, &sourceProjectAttributes, &clientID);
					if (NT_SUCCESS(ntstatus)) {
						ntstatus = ZwTerminateProcess(processToTerminate, 0);
						if (NT_SUCCESS(ntstatus)) {
							ZwClose(processToTerminate);
							DbgPrint("Process notepad.exe terminated\n");
						}
						else {
							DbgPrint("Error terminating target process\n");
						}
					}
					else {
						DbgPrint("Error opening target process\n");
					}
				}
				else {
					DbgPrint("Error reading file\n");
				}
				ZwClose(fileHandle);
			}
			else {
				DbgPrint("Error opening file\n");
			}
		}
	}
	//process created
	else
	{
		procName = CreateInfo->ImageFileName;
		DbgPrint("Process %S created\n", procName->Buffer);
		if (wcsstr(procName->Buffer, L"notepad.exe"))
			idToTerminate = ProcessId;

		if (wcsstr(procName->Buffer, L"UserModeProject.exe"))
		{
			globProcessId = ProcessId;
			RtlInitUnicodeString(&sourceProcessName, L"\\??\\E:\\check.txt");
			InitializeObjectAttributes(&fileAttributes, &sourceProcessName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
			ntstatus = ZwCreateFile(&fileHandle, GENERIC_WRITE, &fileAttributes, &ioStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
			if (NT_SUCCESS(ntstatus)) {
				ntstatus = RtlStringCbPrintfA(bufferToWrite, sizeof(bufferToWrite), "n");
				if (NT_SUCCESS(ntstatus)) {
					ntstatus = RtlStringCbLengthA(bufferToWrite, sizeof(bufferToWrite), &cbLen);
					if (NT_SUCCESS(ntstatus)) {
						ntstatus = ZwWriteFile(fileHandle, NULL, NULL, NULL, &ioStatusBlock, bufferToWrite, (ULONG)cbLen, NULL, NULL);
						if (!NT_SUCCESS(ntstatus)) {
							DbgPrint("Error writing to file\n");
						}
						DbgPrint("Our USERMODE opened\n");
						DbgPrint("Process calc.exe started\n");
						ZwClose(fileHandle);
					}
					else {
						DbgPrint("Error RtlStringCbLengthA\n");
					}
				}
				else {
					DbgPrint("Error RtlStringCbPrintfA\n");
				}
			}
			else {
				DbgPrint("Error opening file \n");
			}
		}
	}
}


VOID
MyDriverUnload(
	_In_
	WDFDRIVER Driver
)
{
	UNREFERENCED_PARAMETER(Driver);
	if (STATUS_SUCCESS == (PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, TRUE))) {
		DbgPrint("Removed routine!\n");
	}
	else {
		DbgPrint("Failed to remove routine!\n");
	}

	DbgPrint("Driver unloaded!\n");
}


NTSTATUS
KmdfHelloWorldEvtDeviceAdd(
	_In_    WDFDRIVER       Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
{
	// We're not using the driver object,
	// so we need to mark it as unreferenced
	UNREFERENCED_PARAMETER(Driver);

	NTSTATUS status;

	// Allocate the device object
	WDFDEVICE hDevice;

	// Print "Hello World"
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: KmdfHelloWorldEvtDeviceAdd\n"));

	// Create the device object
	status = WdfDeviceCreate(&DeviceInit,
		WDF_NO_OBJECT_ATTRIBUTES,
		&hDevice
	);
	return status;
}
