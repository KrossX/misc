/* Copyright (c) 2018 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include <windows.h>
#include <stdio.h>

typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#define PATH_LEN 255

u32 num_files;
u32 total_files;
u32 cluster_total;
u32 cluster_size;
u32 buffer_length;
u32 buffer_size;
u32 *buffer;

void fill_buffer(void)
{
	u32 i, pattern = 0xAAAAAAAA;
	
	for(i = 0; i < buffer_length; i++) {
		buffer[i] = pattern;
		pattern = ~pattern;
	}
}

int check_buffer(void)
{
	u32 i, pattern = 0xAAAAAAAA;
	
	for(i = 0; i < buffer_length; i++) {
		if(buffer[i] != pattern) return 1;
		pattern = ~pattern;
	}

	return 0;
}

#define BASESTR filename, (num_files*100)/total_files

int test_file(char *filename)
{
	DWORD nbytes;
	u32 i;
	
	HANDLE file = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_FLAG_NO_BUFFERING, NULL);
	if(file == INVALID_HANDLE_VALUE) return 1;
	
	fill_buffer();
	
	printf("%s (%02d%%): Writing...    ", BASESTR);
	
	for(i = 0; i < cluster_total; i++) {
		if(!WriteFile(file, &buffer[i*(cluster_size>>2)], cluster_size, &nbytes, NULL) || nbytes != cluster_size) {
			printf("\r%s (%02d%%): Error writing file at %02d.\n", BASESTR, (i * 100) / cluster_total);
			CloseHandle(file);
			return 1;
		} else {
			printf("\r%s (%02d%%): Writing... %02d%%", BASESTR, (i * 100) / cluster_total);
		}
	}
	
	SetFilePointer(file, 0, NULL, FILE_BEGIN);

	printf("\r%s (%02d%%): Reading...    ", BASESTR);
	
	for(i = 0; i < cluster_total; i++) {
		if(!ReadFile(file, &buffer[i*(cluster_size>>2)], cluster_size, &nbytes, NULL) || nbytes != cluster_size) {
			printf("\r%s (%02d%%): Error reading file at %02d.\n", BASESTR, (i * 100) / cluster_total);
			CloseHandle(file);
			return 1;
		} else {
			printf("\r%s (%02d%%): Reading... %02d%%", BASESTR, (i * 100) / cluster_total);
		}
	}

	printf("\r%s (%02d%%): Verifying...  ", BASESTR);
		
	if(check_buffer()) {
		printf("\r%s (%02d%%): Error verifying file.\n", BASESTR);
		CloseHandle(file);
		return 1;
	} else {
		printf("\r%s (%02d%%): Verifying... OK!\n", BASESTR);
		CloseHandle(file);
		return 0;
	}
}

void check_path_string(char *path)
{
	DWORD attribs;
	u32 len = 0, last = 0;
	char *str = path;
	
	while(*str)
	{
		if(*str == '/') { *str = '\\'; last = len; }
		else if(*str == '\\') { last = len; }
		str++; len++;
	}
	
	if(len < 1 || last < 1) return;

	if(path[len-1] == '\\') path[len-1] = 0;
	if(path[0] >= 'a' && path[0] <= 'z') path[0] = path[0] - 'a' + 'A';

	attribs = GetFileAttributes(path);
	if(attribs == 0xFFFFFFFF || (attribs&FILE_ATTRIBUTE_DIRECTORY == 0)) {
		path[last] = 0;
	}
}

int check_path(char *path)
{
	u32 i, last = 0;
	
	ULARGE_INTEGER fb;  // free bytes available
	ULARGE_INTEGER tb;  // total bytes
	ULARGE_INTEGER tfb; // total free bytes
	
	DWORD cs; // cluster sectors
	DWORD sb; // sector bytes
	DWORD fc; // free clusters
	DWORD tc; // total clusters
	
	check_path_string(path);

	if(!GetDiskFreeSpace(path, &cs, &sb, &fc, &tc))
		return 1;
	
	if(!GetDiskFreeSpaceEx(path, &fb, &tb, &tfb))
		return 1;
	
	cluster_size  = cs * sb;
	buffer_size   = fb.QuadPart >> 15;
	buffer_size   = buffer_size < 8388608 ? 8388608 : buffer_size;
	buffer_size   = (buffer_size / cluster_size) * cluster_size;
	buffer_length = buffer_size >> 2;
	cluster_total = buffer_size / cluster_size;
	total_files   = fb.QuadPart / buffer_size;

	printf("Path: %s\n", path);
	printf("Cluster Size: %uB, Test File Size: %uMiB, Free Space: %uMiB\n",
		cluster_size, buffer_size >> 20, (u32)(fb.QuadPart>>20));
	
	return 0;
}

char path[PATH_LEN];
char namebuf[PATH_LEN * 2];

int main(int argc, char **args) 
{
	u32  index = 0;
	
	if(argc == 2) {
		strncpy(path, args[1], PATH_LEN);
	} else {
		printf("Set path: ");
		scanf("%255s", path);
	}
	
	if(check_path(path)) {
		printf("Failed to get free space information.\n");
		system("pause");
		return 0;
	}
	
	buffer = VirtualAlloc(NULL, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	if(!buffer) {
		printf("Error creating buffer of size: %u bytes.\n", buffer_size);
		system("pause");
		return 0;
	}

	while(1) {
		sprintf(namebuf, "%s\\%016d.test", path, index++);
		
		if(test_file(namebuf)) {
			DWORD err = GetLastError();
			if(err == ERROR_FILE_EXISTS) continue;
			else {
				char *msg;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&msg, 0, NULL);
				printf("Error: (%d) %s", err, msg);
				LocalFree(msg);
				break;
			}
		}

		num_files++;
	}

	printf("Total verified files: %u, %u MiB\n", num_files, num_files * (buffer_size>>20));
	system("pause");
	
	return 0;
}