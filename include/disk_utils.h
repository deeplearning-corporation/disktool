#ifndef DISK_UTILS_H
#define DISK_UTILS_H

#include <windows.h>
#include <stdio.h>
#include "disk_info.h"

// 磁盘健康状态
typedef struct {
    BOOL is_healthy;
    DWORD last_error;
    char error_message[256];
    DWORD media_type;
    ULARGE_INTEGER bytes_per_sector;
    ULARGE_INTEGER total_sectors;
} DiskHealthInfo;

// 磁盘性能统计
typedef struct {
    DWORD read_speed;      // MB/s
    DWORD write_speed;     // MB/s
    DWORD avg_read_time;   // ms
    DWORD avg_write_time;  // ms
    DWORD queue_length;
} DiskPerformanceInfo;

// 函数声明
BOOL IsDriveExist(const char* drive_path);
HANDLE OpenDiskHandle(const char* drive_path, BOOL read_only);
BOOL GetDiskGeometry(const char* drive_path, DISK_GEOMETRY* geometry);
BOOL CheckDiskHealth(const char* drive_path, DiskHealthInfo* health_info);
BOOL TestDiskPerformance(const char* drive_path, DiskPerformanceInfo* perf_info, DWORD test_size_mb);
BOOL GetDiskSerialNumber(const char* drive_path, char* serial, DWORD serial_size);
BOOL GetDiskModel(const char* drive_path, char* model, DWORD model_size);
BOOL TestReadWriteAccess(const char* drive_path);
BOOL EjectRemovableDisk(const char* drive_path);

#endif // DISK_UTILS_H
