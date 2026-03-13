#ifndef DISK_INFO_H
#define DISK_INFO_H

#include <windows.h>
#include <stdio.h>

// 磁盘信息结构体
typedef struct {
    char device_name[8];
    char volume_name[256];
    char file_system[64];
    ULARGE_INTEGER total_space;
    ULARGE_INTEGER free_space;
    ULARGE_INTEGER available_space;
    DWORD serial_number;
    DWORD max_component_length;
    DWORD file_system_flags;
} DiskInfo;

// 函数声明
BOOL GetLogicalDrivesList(char drives[][8], int* count);
BOOL GetDiskInfo(const char* drive_path, DiskInfo* info);
void FormatSize(ULARGE_INTEGER size, char* buffer, size_t buffer_size);
const char* GetDriveTypeString(const char* drive_path);
BOOL IsRemovableDrive(const char* drive_path);

#endif // DISK_INFO_H
