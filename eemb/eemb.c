/* Copyright (c) 2020 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include <windows.h>

void *mem_ptr;
UINT mem_size;

enum {
	EVENT_APPLICATION,
	EVENT_SYSTEM,
	EVENT_COUNT
};

char evt_src[EVENT_COUNT][16] = {
	"Application",
	"System" };

HANDLE evt_wait[EVENT_COUNT];
HANDLE evt_log[EVENT_COUNT];

void err_msg_quit(char *msg)
{
	MessageBox(NULL, msg, "Error!", MB_OK | MB_ICONERROR | MB_TOPMOST);
	ExitProcess(-1);
}

void mem_alloc(DWORD size)
{
	if(size > mem_size) {
		VirtualFree(mem_ptr, 0, MEM_RELEASE);
		mem_ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if(!mem_ptr) err_msg_quit("Could not allocate memory.");
	}
}

int read_evt_log(HANDLE evt, DWORD flags, DWORD record_off)
{
	DWORD byte_count = sizeof(EVENTLOGRECORD);
	DWORD bytes_read;
	DWORD min_bytes;
	
	if(ReadEventLog(evt, flags, record_off, mem_ptr, byte_count, &bytes_read, &min_bytes))
		return 1;

	if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		mem_alloc(min_bytes);
		byte_count = min_bytes;
		
		if(ReadEventLog(evt, flags, record_off, mem_ptr, byte_count, &bytes_read, &min_bytes))
			return 1;
	}

	return 0;
}

void seek_last_evt_log(HANDLE evt)
{
	DWORD oldest, total;

	if(!GetOldestEventLogRecord(evt, &oldest))
		err_msg_quit("GetOldestEventLogRecord failed.");
		
	if(!GetNumberOfEventLogRecords(evt, &total))
		err_msg_quit("GetNumberOfEventLogRecords failed.");
	
	read_evt_log(evt, EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ, oldest + total - 1);
}

void application_error_msg(WORD num, char *evt_strings, char *evt_source)
{
	static char buffer[1024];
	char *str[13];
	int i;
	
	str[0] = evt_strings;
	for(i = 1; i < 13; i++) {
		char *s = str[i-1];
		while(*s++);
		str[i] = s;
	}
	
	wsprintf(buffer,
	"Faulting application:\n\tname: %s\n\tversion: %s\n\ttimestamp: %s\n\n"
	"Faulting module\n\tname: %s\n\tversion: %s\n\ttimestamp: %s\n\n"
	"Exception code: %s\n"
	"Fault offset: %s\n"
	"Faulting process id: %s\n"
	"Faulting application start time: %s\n"
	"Faulting application path: %s\n"
	"Faulting module path: %s\n"
	"Report Id: %s\n",
	str[0],str[1],str[2],str[3],str[4],str[5],str[6],
	str[7],str[8],str[9],str[10],str[11],str[12]);
	
	MessageBox(NULL, buffer, evt_source, MB_OK | MB_ICONERROR | MB_TOPMOST);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	int i;

	mem_alloc(sizeof(EVENTLOGRECORD));
	
	for(i = 0; i < EVENT_COUNT; i++) {
		evt_wait[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(!evt_wait[i]) err_msg_quit("CreateEvent failed.");
		
		evt_log[i] = OpenEventLog(NULL, evt_src[i]);
		if(!evt_log[i]) err_msg_quit("OpenEventLog failed.");
		
		seek_last_evt_log(evt_log[i]);
		
		if(!NotifyChangeEventLog(evt_log[i], evt_wait[i]))
			err_msg_quit("NotifyChangeEventLog failed.");
	}

	while(1) {
		DWORD id = WaitForMultipleObjects(2, evt_wait, FALSE, INFINITE);
		if(id == WAIT_FAILED) err_msg_quit("Waiting failed.");
		id -= WAIT_OBJECT_0;
		
		ResetEvent(evt_wait[id]);

		if(read_evt_log(evt_log[id], EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0)) {
			EVENTLOGRECORD *evt_info = (EVENTLOGRECORD*)mem_ptr;
			char *evt_strings = (char*)mem_ptr + evt_info->StringOffset;
			char *evt_source = (char*)mem_ptr + sizeof(EVENTLOGRECORD);

			if(evt_info->EventType != EVENTLOG_ERROR_TYPE) continue;
			if(!evt_strings || !evt_source) continue;
			
			if(evt_info->EventID == 1000)
				application_error_msg(evt_info->NumStrings, evt_strings, evt_source);
			else
				MessageBox(NULL, evt_strings, evt_source, MB_OK | MB_ICONERROR | MB_TOPMOST);
		}
	}

	return ERROR_SUCCESS;
}