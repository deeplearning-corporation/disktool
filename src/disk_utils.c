#include "disk_utils.h"
#include <winioctl.h>

// 检查磁盘是否存在
BOOL IsDriveExist(const char* drive_path) {
    char root_path[MAX_PATH];
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    return (GetDriveTypeA(root_path) != DRIVE_NO_ROOT_DIR);
}

// 打开磁盘句柄
HANDLE OpenDiskHandle(const char* drive_path, BOOL read_only) {
    char physical_path[MAX_PATH];
    HANDLE hDevice;
    DWORD access = read_only ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
    DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    
    // 尝试打开物理磁盘
    snprintf(physical_path, sizeof(physical_path), "\\\\.\\%c:", drive_path[0]);
    
    hDevice = CreateFileA(
        physical_path,
        access,
        share,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    
    return hDevice;
}

// 获取磁盘几何信息
BOOL GetDiskGeometry(const char* drive_path, DISK_GEOMETRY* geometry) {
    HANDLE hDevice = OpenDiskHandle(drive_path, TRUE);
    DWORD bytesReturned;
    BOOL result;
    
    if (hDevice == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    result = DeviceIoControl(
        hDevice,
        IOCTL_DISK_GET_DRIVE_GEOMETRY,
        NULL,
        0,
        geometry,
        sizeof(DISK_GEOMETRY),
        &bytesReturned,
        NULL);
    
    CloseHandle(hDevice);
    return result;
}

// 检查磁盘健康状态
BOOL CheckDiskHealth(const char* drive_path, DiskHealthInfo* health_info) {
    char root_path[MAX_PATH];
    HANDLE hDevice;
    DISK_GEOMETRY geometry;
    DWORD bytesReturned;
    
    if (!health_info) return FALSE;
    
    memset(health_info, 0, sizeof(DiskHealthInfo));
    
    // 构造根路径
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    // 检查驱动器是否存在
    if (GetDriveTypeA(root_path) == DRIVE_NO_ROOT_DIR) {
        health_info->is_healthy = FALSE;
        health_info->last_error = ERROR_PATH_NOT_FOUND;
        strcpy(health_info->error_message, "驱动器不存在");
        return FALSE;
    }
    
    // 尝试打开物理磁盘
    char physical_path[MAX_PATH];
    snprintf(physical_path, sizeof(physical_path), "\\\\.\\%c:", drive_path[0]);
    
    hDevice = CreateFileA(
        physical_path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    
    if (hDevice == INVALID_HANDLE_VALUE) {
        health_info->is_healthy = FALSE;
        health_info->last_error = GetLastError();
        strcpy(health_info->error_message, "无法打开设备");
        return FALSE;
    }
    
    // 获取磁盘几何信息
    if (DeviceIoControl(
            hDevice,
            IOCTL_DISK_GET_DRIVE_GEOMETRY,
            NULL,
            0,
            &geometry,
            sizeof(geometry),
            &bytesReturned,
            NULL)) {
        health_info->media_type = geometry.MediaType;
        health_info->bytes_per_sector.QuadPart = geometry.BytesPerSector;
        health_info->total_sectors.QuadPart = geometry.Cylinders.QuadPart * 
                                              geometry.TracksPerCylinder * 
                                              geometry.SectorsPerTrack;
        health_info->is_healthy = TRUE;
    } else {
        health_info->is_healthy = FALSE;
        health_info->last_error = GetLastError();
        strcpy(health_info->error_message, "无法获取磁盘几何信息");
    }
    
    CloseHandle(hDevice);
    return health_info->is_healthy;
}

// 简单的磁盘性能测试
BOOL TestDiskPerformance(const char* drive_path, DiskPerformanceInfo* perf_info, DWORD test_size_mb) {
    char test_file[MAX_PATH];
    HANDLE hFile;
    DWORD bytesWritten, bytesRead;
    LARGE_INTEGER freq, start, end, write_end, read_end;
    char* buffer;
    DWORD buffer_size = test_size_mb * 1024 * 1024; // 转换为字节
    BOOL result = TRUE;
    
    if (!perf_info) return FALSE;
    
    // 初始化性能信息
    memset(perf_info, 0, sizeof(DiskPerformanceInfo));
    
    // 创建测试文件路径
    snprintf(test_file, sizeof(test_file), "%s\\disktool_test.tmp", drive_path);
    
    // 分配测试缓冲区
    buffer = (char*)VirtualAlloc(NULL, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer) {
        return FALSE;
    }
    
    // 填充测试数据
    memset(buffer, 0xAA, buffer_size);
    
    // 创建测试文件
    hFile = CreateFileA(
        test_file,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
        NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        VirtualFree(buffer, 0, MEM_RELEASE);
        return FALSE;
    }
    
    QueryPerformanceFrequency(&freq);
    
    // 测试写入性能
    QueryPerformanceCounter(&start);
    if (!WriteFile(hFile, buffer, buffer_size, &bytesWritten, NULL) || bytesWritten != buffer_size) {
        result = FALSE;
        goto cleanup;
    }
    QueryPerformanceCounter(&write_end);
    
    // 刷新到磁盘
    FlushFileBuffers(hFile);
    
    // 计算写入速度
    double write_time = (double)(write_end.QuadPart - start.QuadPart) / freq.QuadPart;
    if (write_time > 0) {
        perf_info->write_speed = (DWORD)((test_size_mb) / write_time);
    }
    perf_info->avg_write_time = (DWORD)(write_time * 1000 / test_size_mb);
    
    // 设置文件指针到开始
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    
    // 测试读取性能
    QueryPerformanceCounter(&start);
    if (!ReadFile(hFile, buffer, buffer_size, &bytesRead, NULL) || bytesRead != buffer_size) {
        result = FALSE;
        goto cleanup;
    }
    QueryPerformanceCounter(&read_end);
    
    // 计算读取速度
    double read_time = (double)(read_end.QuadPart - start.QuadPart) / freq.QuadPart;
    if (read_time > 0) {
        perf_info->read_speed = (DWORD)((test_size_mb) / read_time);
    }
    perf_info->avg_read_time = (DWORD)(read_time * 1000 / test_size_mb);
    perf_info->queue_length = 1; // 简化版本，默认队列长度为1
    
cleanup:
    CloseHandle(hFile);
    DeleteFileA(test_file);
    VirtualFree(buffer, 0, MEM_RELEASE);
    
    return result;
}

// 获取磁盘序列号（简化版）
BOOL GetDiskSerialNumber(const char* drive_path, char* serial, DWORD serial_size) {
    char root_path[MAX_PATH];
    DWORD vsn, max_len, flags;
    char fs_name[MAX_PATH];
    
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    if (GetVolumeInformationA(root_path, NULL, 0, &vsn, &max_len, &flags, fs_name, sizeof(fs_name))) {
        snprintf(serial, serial_size, "%08X-%08X", HIWORD(vsn), LOWORD(vsn));
        return TRUE;
    }
    
    strcpy(serial, "未知");
    return FALSE;
}

// 获取磁盘型号（简化版）- 修正版
BOOL GetDiskModel(const char* drive_path, char* model, DWORD model_size) {
    char root_path[MAX_PATH];
    
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    // 根据驱动器类型返回描述
    UINT driveType = GetDriveTypeA(root_path);
    
    switch (driveType) {
        case DRIVE_REMOVABLE:
            strncpy(model, "可移动磁盘", model_size - 1);
            break;
        case DRIVE_FIXED:
            strncpy(model, "固定磁盘", model_size - 1);
            break;
        case DRIVE_REMOTE:
            strncpy(model, "网络磁盘", model_size - 1);
            break;
        case DRIVE_CDROM:
            strncpy(model, "光盘驱动器", model_size - 1);
            break;
        case DRIVE_RAMDISK:
            strncpy(model, "RAM磁盘", model_size - 1);
            break;
        default:
            strncpy(model, "未知型号", model_size - 1);
            break;
    }
    
    model[model_size - 1] = '\0';
    return TRUE;
}

// 创建临时文件测试读写
BOOL TestReadWriteAccess(const char* drive_path) {
    char test_file[MAX_PATH];
    HANDLE hFile;
    BOOL result = TRUE;
    
    snprintf(test_file, sizeof(test_file), "%s\\disktool_access.tmp", drive_path);
    
    hFile = CreateFileA(
        test_file,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    CloseHandle(hFile);
    DeleteFileA(test_file);
    
    return TRUE;
}

// 安全弹出可移动磁盘
BOOL EjectRemovableDisk(const char* drive_path) {
    char root_path[MAX_PATH];
    char volume_path[MAX_PATH];
    HANDLE hVolume;
    DWORD bytesReturned;
    BOOL result = FALSE;
    
    // 构造根路径
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    // 检查是否可移动
    if (GetDriveTypeA(root_path) != DRIVE_REMOVABLE) {
        return FALSE;
    }
    
    // 构造卷设备路径
    snprintf(volume_path, sizeof(volume_path), "\\\\.\\%c:", drive_path[0]);
    
    // 打开卷
    hVolume = CreateFileA(
        volume_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    
    if (hVolume == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    // 锁定卷
    if (DeviceIoControl(
            hVolume,
            FSCTL_LOCK_VOLUME,
            NULL,
            0,
            NULL,
            0,
            &bytesReturned,
            NULL)) {
        
        // 卸载卷
        if (DeviceIoControl(
                hVolume,
                FSCTL_DISMOUNT_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &bytesReturned,
                NULL)) {
            
            // 弹出媒体
            result = DeviceIoControl(
                hVolume,
                IOCTL_STORAGE_EJECT_MEDIA,
                NULL,
                0,
                NULL,
                0,
                &bytesReturned,
                NULL);
        }
        
        // 如果弹出失败，解锁卷
        if (!result) {
            DeviceIoControl(
                hVolume,
                FSCTL_UNLOCK_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &bytesReturned,
                NULL);
        }
    }
    
    CloseHandle(hVolume);
    return result;
}
