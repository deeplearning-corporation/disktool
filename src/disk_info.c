#include "disk_info.h"
#include <string.h>

// 获取所有逻辑驱动器
BOOL GetLogicalDrivesList(char drives[][8], int* count) {
    DWORD drive_mask = GetLogicalDrives();
    char drive[] = "A:\\";
    *count = 0;
    
    if (drive_mask == 0) {
        return FALSE;
    }
    
    for (int i = 0; i < 26; i++) {
        if (drive_mask & (1 << i)) {
            drive[0] = 'A' + i;
            strcpy(drives[*count], drive);
            (*count)++;
        }
    }
    
    return TRUE;
}

// 获取磁盘详细信息
BOOL GetDiskInfo(const char* drive_path, DiskInfo* info) {
    char root_path[MAX_PATH];
    char volume_name[MAX_PATH];
    DWORD vsn, max_len, flags;
    char fs_name[MAX_PATH];
    ULARGE_INTEGER free_avail, total, free_total;
    
    if (!info) return FALSE;
    
    // 构造根路径
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    // 获取卷信息
    if (!GetVolumeInformationA(
            root_path,
            volume_name,
            sizeof(volume_name),
            &vsn,
            &max_len,
            &flags,
            fs_name,
            sizeof(fs_name))) {
        // 如果无法获取卷信息，至少填充一些默认值
        strcpy(info->device_name, drive_path);
        strcpy(info->volume_name, "");
        strcpy(info->file_system, "未知");
        info->total_space.QuadPart = 0;
        info->free_space.QuadPart = 0;
        info->available_space.QuadPart = 0;
        info->serial_number = 0;
        info->max_component_length = 0;
        info->file_system_flags = 0;
        return TRUE;
    }
    
    // 获取磁盘空间信息
    if (!GetDiskFreeSpaceExA(root_path, &free_avail, &total, &free_total)) {
        total.QuadPart = 0;
        free_total.QuadPart = 0;
        free_avail.QuadPart = 0;
    }
    
    // 填充信息结构
    strcpy(info->device_name, drive_path);
    strcpy(info->volume_name, volume_name);
    strcpy(info->file_system, fs_name);
    info->total_space = total;
    info->free_space = free_total;
    info->available_space = free_avail;
    info->serial_number = vsn;
    info->max_component_length = max_len;
    info->file_system_flags = flags;
    
    return TRUE;
}

// 格式化大小为人类可读格式
void FormatSize(ULARGE_INTEGER size, char* buffer, size_t buffer_size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double size_d = (double)size.QuadPart;
    
    if (size_d < 0.1) {
        snprintf(buffer, buffer_size, "0 B");
        return;
    }
    
    while (size_d >= 1024.0 && unit_index < 5) {
        size_d /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%.0f %s", size_d, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size_d, units[unit_index]);
    }
}

// 获取驱动器类型字符串
const char* GetDriveTypeString(const char* drive_path) {
    char root_path[MAX_PATH];
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    switch (GetDriveTypeA(root_path)) {
        case DRIVE_UNKNOWN:     return "未知";
        case DRIVE_NO_ROOT_DIR: return "无效路径";
        case DRIVE_REMOVABLE:   return "可移动";
        case DRIVE_FIXED:       return "本地磁盘";
        case DRIVE_REMOTE:      return "网络驱动器";
        case DRIVE_CDROM:       return "光盘";
        case DRIVE_RAMDISK:     return "RAM磁盘";
        default:                return "其他";
    }
}

// 检查磁盘是否可移动
BOOL IsRemovableDrive(const char* drive_path) {
    char root_path[MAX_PATH];
    strcpy(root_path, drive_path);
    if (root_path[strlen(root_path) - 1] != '\\') {
        strcat(root_path, "\\");
    }
    
    return (GetDriveTypeA(root_path) == DRIVE_REMOVABLE);
}
