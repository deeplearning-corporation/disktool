#include "gui.h"
#include <stdio.h>
#include <string.h>

// 全局实例
HINSTANCE g_hInst;

// 函数声明
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hWnd, MainWindowData* data);

// 初始化GUI
BOOL InitGUI(HINSTANCE hInstance) {
    g_hInst = hInstance;
    
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
    
    return TRUE;
}

// 创建主窗口
HWND CreateMainWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc = {0};
    MainWindowData* data = (MainWindowData*)malloc(sizeof(MainWindowData));
    memset(data, 0, sizeof(MainWindowData));
    
    // 注册窗口类
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(MainWindowData*);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "DiskToolWindow";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        free(data);
        return NULL;
    }
    
    // 创建窗口
    HWND hWnd = CreateWindowEx(
        0,
        "DiskToolWindow",
        "DiskTool - 磁盘管理工具",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 650,
        NULL,
        NULL,
        hInstance,
        data);
    
    if (hWnd) {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
        data->hMainWnd = hWnd;
        
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    } else {
        free(data);
    }
    
    return hWnd;
}

// 创建控件
void CreateControls(HWND hWnd, MainWindowData* data) {
    HFONT hTitleFont, hNormalFont;
    RECT rc;
    GetClientRect(hWnd, &rc);
    
    // 创建字体
    hTitleFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "微软雅黑");
    hNormalFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "微软雅黑");
    
    // 创建标题标签
    HWND hTitle = CreateWindow("STATIC", "磁盘驱动器列表",
                              WS_CHILD | WS_VISIBLE | SS_LEFT,
                              10, 10, 200, 25,
                              hWnd, NULL, g_hInst, NULL);
    SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    
    // 创建驱动器列表（使用ListView）
    data->hDriveList = CreateWindowEx(
        WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        10, 40, 250, 300,
        hWnd, (HMENU)IDC_DRIVE_LIST, g_hInst, NULL);
    
    // 添加列
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 80;
    lvc.pszText = "驱动器";
    ListView_InsertColumn(data->hDriveList, 0, &lvc);
    
    lvc.cx = 100;
    lvc.pszText = "类型";
    ListView_InsertColumn(data->hDriveList, 1, &lvc);
    
    lvc.cx = 60;
    lvc.pszText = "卷标";
    ListView_InsertColumn(data->hDriveList, 2, &lvc);
    
    // 创建信息显示区域
    HWND hInfoTitle = CreateWindow("STATIC", "磁盘信息",
                                   WS_CHILD | WS_VISIBLE | SS_LEFT,
                                   270, 10, 200, 25,
                                   hWnd, NULL, g_hInst, NULL);
    SendMessage(hInfoTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    
    data->hInfoEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, "EDIT", NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | 
        ES_AUTOVSCROLL | WS_VSCROLL,
        270, 40, 600, 280,
        hWnd, (HMENU)IDC_INFO_EDIT, g_hInst, NULL);
    SendMessage(data->hInfoEdit, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    
    // 创建进度条
    data->hProgressBar = CreateWindowEx(
        0, PROGRESS_CLASS, NULL,
        WS_CHILD | WS_VISIBLE,
        270, 330, 400, 20,
        hWnd, (HMENU)IDC_PROGRESS_BAR, g_hInst, NULL);
    
    // 创建状态文本
    data->hStatusText = CreateWindow("STATIC", "就绪",
                                     WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     680, 330, 190, 20,
                                     hWnd, (HMENU)IDC_STATUS_TEXT, g_hInst, NULL);
    SendMessage(data->hStatusText, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    
    // 创建按钮
    data->hRefreshBtn = CreateWindow("BUTTON", "刷新",
                                     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     10, 350, 80, 30,
                                     hWnd, (HMENU)IDC_REFRESH_BTN, g_hInst, NULL);
    
    data->hPropertiesBtn = CreateWindow("BUTTON", "属性",
                                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                        100, 350, 80, 30,
                                        hWnd, (HMENU)IDC_PROPERTIES_BTN, g_hInst, NULL);
    
    data->hPerfBtn = CreateWindow("BUTTON", "性能测试",
                                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  10, 390, 80, 30,
                                  hWnd, (HMENU)IDC_PERF_BTN, g_hInst, NULL);
    
    data->hHealthBtn = CreateWindow("BUTTON", "健康检查",
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    100, 390, 80, 30,
                                    hWnd, (HMENU)IDC_HEALTH_BTN, g_hInst, NULL);
    
    data->hEjectBtn = CreateWindow("BUTTON", "弹出",
                                   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   190, 390, 70, 30,
                                   hWnd, (HMENU)IDC_EJECT_BTN, g_hInst, NULL);
    
    // 设置按钮字体
    SendMessage(data->hRefreshBtn, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    SendMessage(data->hPropertiesBtn, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    SendMessage(data->hPerfBtn, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    SendMessage(data->hHealthBtn, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
    SendMessage(data->hEjectBtn, WM_SETFONT, (WPARAM)hNormalFont, TRUE);
}

// 更新驱动器列表
BOOL UpdateDriveList(MainWindowData* data) {
    char drives[26][8];
    LVITEM lvi = {0};
    int count = 0;
    int itemCount;
    
    if (!data || !data->hDriveList) return FALSE;
    
    data->isUpdating = TRUE;
    SetWindowText(data->hStatusText, "正在扫描驱动器...");
    
    // 清空列表
    ListView_DeleteAllItems(data->hDriveList);
    
    // 释放旧数据
    if (data->diskInfos) {
        free(data->diskInfos);
        data->diskInfos = NULL;
    }
    
    // 获取驱动器列表
    if (!GetLogicalDrivesList(drives, &count)) {
        data->isUpdating = FALSE;
        SetWindowText(data->hStatusText, "扫描失败");
        return FALSE;
    }
    
    if (count == 0) {
        SetWindowText(data->hStatusText, "未找到驱动器");
        data->isUpdating = FALSE;
        return TRUE;
    }
    
    data->diskCount = count;
    data->diskInfos = (DiskInfo*)malloc(count * sizeof(DiskInfo));
    memset(data->diskInfos, 0, count * sizeof(DiskInfo));
    
    // 添加到列表视图
    for (int i = 0; i < count; i++) {
        DiskInfo info;
        
        if (GetDiskInfo(drives[i], &info)) {
            data->diskInfos[i] = info;
            
            // 插入驱动器项
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = i;
            lvi.iSubItem = 0;
            lvi.pszText = drives[i];
            lvi.lParam = i;
            
            itemCount = ListView_InsertItem(data->hDriveList, &lvi);
            
            // 设置类型
            ListView_SetItemText(data->hDriveList, i, 1, (char*)GetDriveTypeString(drives[i]));
            
            // 设置卷标
            if (info.volume_name[0]) {
                ListView_SetItemText(data->hDriveList, i, 2, info.volume_name);
            } else {
                ListView_SetItemText(data->hDriveList, i, 2, "-");
            }
        }
    }
    
    // 更新状态
    char status[256];
    sprintf(status, "找到 %d 个驱动器", count);
    SetWindowText(data->hStatusText, status);
    
    data->isUpdating = FALSE;
    return TRUE;
}

// 显示磁盘信息
void ShowDiskInfo(MainWindowData* data, int driveIndex) {
    if (!data || driveIndex < 0 || driveIndex >= data->diskCount) return;
    
    DiskInfo* info = &data->diskInfos[driveIndex];
    char total_size[32], free_size[32], used_size[32];
    char info_text[4096] = {0};
    
    // 格式化空间大小
    FormatSize(info->total_space, total_size, sizeof(total_size));
    FormatSize(info->free_space, free_size, sizeof(free_size));
    
    ULARGE_INTEGER used_space;
    used_space.QuadPart = info->total_space.QuadPart - info->free_space.QuadPart;
    FormatSize(used_space, used_size, sizeof(used_size));
    
    // 计算使用率
    float used_percent = 0;
    if (info->total_space.QuadPart > 0) {
        used_percent = (float)used_space.QuadPart / info->total_space.QuadPart * 100;
    }
    
    // 构建信息文本
    sprintf(info_text,
        "╔══════════════════════════════════════════════════════════╗\r\n"
        "║                    磁盘信息 - %s                        ║\r\n"
        "╚══════════════════════════════════════════════════════════╝\r\n\r\n"
        "【基本信息】\r\n"
        "────────────────────────────────────────────────\r\n"
        "设备路径:     %s\r\n"
        "卷标:         %s\r\n"
        "文件系统:     %s\r\n"
        "序列号:       %08X-%08X\r\n\r\n"
        "【空间使用】\r\n"
        "────────────────────────────────────────────────\r\n"
        "总空间:       %s\r\n"
        "已用空间:     %s (%.1f%%)\r\n"
        "可用空间:     %s\r\n\r\n"
        "【驱动器属性】\r\n"
        "────────────────────────────────────────────────\r\n"
        "类型:         %s\r\n"
        "最大文件名长度: %lu 字符\r\n",
        info->device_name,
        info->device_name,
        info->volume_name[0] ? info->volume_name : "(无卷标)",
        info->file_system,
        HIWORD(info->serial_number), LOWORD(info->serial_number),
        total_size,
        used_size, used_percent,
        free_size,
        GetDriveTypeString(info->device_name),
        info->max_component_length);
    
    SetWindowText(data->hInfoEdit, info_text);
    
    // 更新进度条
    SendMessage(data->hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(data->hProgressBar, PBM_SETPOS, (WPARAM)used_percent, 0);
}

// 显示磁盘健康状态
void ShowDiskHealth(MainWindowData* data, int driveIndex) {
    if (!data || driveIndex < 0 || driveIndex >= data->diskCount) return;
    
    DiskHealthInfo health;
    char health_text[2048] = {0};
    char drive_path[8];
    strcpy(drive_path, data->diskInfos[driveIndex].device_name);
    
    SetWindowText(data->hInfoEdit, "正在检查磁盘健康状态...\r\n");
    SetWindowText(data->hStatusText, "正在检查健康状态...");
    UpdateWindow(data->hInfoEdit);
    
    if (!CheckDiskHealth(drive_path, &health)) {
        sprintf(health_text, "无法检查磁盘健康状态。\r\n错误: %s", 
                health.error_message[0] ? health.error_message : "未知错误");
        SetWindowText(data->hInfoEdit, health_text);
        SetWindowText(data->hStatusText, "健康检查失败");
        return;
    }
    
    sprintf(health_text,
        "╔════════════════════════════════════════════════╗\r\n"
        "║             磁盘健康状态 - %s                  ║\r\n"
        "╚════════════════════════════════════════════════╝\r\n\r\n"
        "健康状态:     %s\r\n"
        "媒体类型:     %d\r\n"
        "每扇区字节:   %I64u\r\n"
        "总扇区数:     %I64u\r\n",
        drive_path,
        health.is_healthy ? "? 正常" : "? 异常",
        health.media_type,
        health.bytes_per_sector.QuadPart,
        health.total_sectors.QuadPart);
    
    if (health.last_error != 0) {
        char error_line[100];
        sprintf(error_line, "上次错误代码: %d (0x%08X)\r\n", health.last_error, health.last_error);
        strcat(health_text, error_line);
    }
    
    SetWindowText(data->hInfoEdit, health_text);
    SetWindowText(data->hStatusText, "健康检查完成");
}

// 显示性能测试结果
void ShowPerformanceTest(MainWindowData* data, int driveIndex) {
    if (!data || driveIndex < 0 || driveIndex >= data->diskCount) return;
    
    DiskPerformanceInfo perf;
    char perf_text[2048] = {0};
    char drive_path[8];
    strcpy(drive_path, data->diskInfos[driveIndex].device_name);
    
    SetWindowText(data->hInfoEdit, "正在执行性能测试，请稍候...\r\n\r\n测试将进行100MB的读写操作。");
    SetWindowText(data->hStatusText, "性能测试中...");
    UpdateWindow(data->hInfoEdit);
    
    if (!TestDiskPerformance(drive_path, &perf, 100)) {
        SetWindowText(data->hInfoEdit, 
            "性能测试失败。\r\n可能原因：\r\n"
            "1. 磁盘不可写（如光盘）\r\n"
            "2. 磁盘已满\r\n"
            "3. 权限不足\r\n"
            "4. 磁盘正在被其他程序使用");
        SetWindowText(data->hStatusText, "性能测试失败");
        return;
    }
    
    sprintf(perf_text,
        "╔════════════════════════════════════════════════╗\r\n"
        "║             磁盘性能测试 - %s                  ║\r\n"
        "╚════════════════════════════════════════════════╝\r\n\r\n"
        "读取速度:     %lu MB/s\r\n"
        "写入速度:     %lu MB/s\r\n"
        "平均读取时间: %lu ms\r\n"
        "平均写入时间: %lu ms\r\n"
        "当前队列长度: %lu\r\n\r\n"
        "【性能评级】\r\n"
        "────────────────\r\n",
        drive_path,
        perf.read_speed,
        perf.write_speed,
        perf.avg_read_time,
        perf.avg_write_time,
        perf.queue_length);
    
    // 读取性能评级
    if (perf.read_speed >= 300) {
        strcat(perf_text, "读取性能: ★★★★★ (SSD级别)\r\n");
    } else if (perf.read_speed >= 150) {
        strcat(perf_text, "读取性能: ★★★★☆ (高速硬盘)\r\n");
    } else if (perf.read_speed >= 80) {
        strcat(perf_text, "读取性能: ★★★☆☆ (普通硬盘)\r\n");
    } else if (perf.read_speed >= 30) {
        strcat(perf_text, "读取性能: ★★☆☆☆ (低速设备)\r\n");
    } else {
        strcat(perf_text, "读取性能: ★☆☆☆☆ (非常慢)\r\n");
    }
    
    // 写入性能评级
    if (perf.write_speed >= 250) {
        strcat(perf_text, "写入性能: ★★★★★ (SSD级别)\r\n");
    } else if (perf.write_speed >= 120) {
        strcat(perf_text, "写入性能: ★★★★☆ (高速硬盘)\r\n");
    } else if (perf.write_speed >= 60) {
        strcat(perf_text, "写入性能: ★★★☆☆ (普通硬盘)\r\n");
    } else if (perf.write_speed >= 20) {
        strcat(perf_text, "写入性能: ★★☆☆☆ (低速设备)\r\n");
    } else {
        strcat(perf_text, "写入性能: ★☆☆☆☆ (非常慢)\r\n");
    }
    
    SetWindowText(data->hInfoEdit, perf_text);
    SetWindowText(data->hStatusText, "性能测试完成");
}

// 弹出可移动磁盘
BOOL EjectDrive(MainWindowData* data, int driveIndex) {
    if (!data || driveIndex < 0 || driveIndex >= data->diskCount) return FALSE;
    
    char drive_path[8];
    strcpy(drive_path, data->diskInfos[driveIndex].device_name);
    
    // 检查是否可移动
    if (!IsRemovableDrive(drive_path)) {
        MessageBox(data->hMainWnd, "该驱动器不是可移动磁盘，无法弹出。", 
                   "提示", MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }
    
    // 确认对话框
    char msg[256];
    sprintf(msg, "确定要弹出驱动器 %s 吗？\n\n请确保没有程序正在使用该磁盘。", drive_path);
    
    if (MessageBox(data->hMainWnd, msg, "确认弹出", MB_YESNO | MB_ICONQUESTION) != IDYES) {
        return FALSE;
    }
    
    SetWindowText(data->hStatusText, "正在弹出磁盘...");
    UpdateWindow(data->hStatusText);
    
    BOOL result = EjectRemovableDisk(drive_path);
    
    if (result) {
        MessageBox(data->hMainWnd, "磁盘已成功弹出。", "提示", MB_OK | MB_ICONINFORMATION);
        SetWindowText(data->hStatusText, "磁盘已弹出");
        UpdateDriveList(data);
    } else {
        MessageBox(data->hMainWnd, 
                   "弹出磁盘失败。\n可能原因：\n"
                   "1. 磁盘正在被程序使用\n"
                   "2. 磁盘不可移动\n"
                   "3. 权限不足", 
                   "错误", MB_OK | MB_ICONERROR);
        SetWindowText(data->hStatusText, "弹出失败");
    }
    
    return result;
}

// 关于对话框过程
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// 显示关于对话框
void ShowAboutDialog(HWND hParent) {
    // 创建简单的关于对话框
    MessageBox(hParent, 
        "DiskTool v1.0\n\n"
        "磁盘管理工具\n"
        "功能：\n"
        "? 查看磁盘信息\n"
        "? 磁盘健康检查\n"
        "? 性能测试\n"
        "? 弹出可移动磁盘\n\n"
        "Copyright ? 2024",
        "关于 DiskTool",
        MB_OK | MB_ICONINFORMATION);
}

// 主窗口消息处理
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindowData* data = (MainWindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            data = (MainWindowData*)cs->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
            
            // 创建控件
            CreateControls(hWnd, data);
            
            // 初始更新驱动器列表
            UpdateDriveList(data);
            
            // 设置定时器，每5秒刷新状态
            SetTimer(hWnd, IDC_UPDATE_TIMER, 5000, NULL);
            break;
        }
        
        case WM_SIZE: {
            if (data) {
                RECT rc;
                GetClientRect(hWnd, &rc);
                
                // 调整控件大小
                SetWindowPos(data->hDriveList, NULL, 10, 40, 250, rc.bottom - 150, SWP_NOZORDER);
                SetWindowPos(data->hInfoEdit, NULL, 270, 40, rc.right - 280, rc.bottom - 180, SWP_NOZORDER);
                SetWindowPos(data->hProgressBar, NULL, 270, rc.bottom - 130, 400, 20, SWP_NOZORDER);
                SetWindowPos(data->hStatusText, NULL, 680, rc.bottom - 130, 190, 20, SWP_NOZORDER);
                
                // 调整按钮位置
                int btnY = rc.bottom - 90;
                SetWindowPos(data->hRefreshBtn, NULL, 10, btnY, 80, 30, SWP_NOZORDER);
                SetWindowPos(data->hPropertiesBtn, NULL, 100, btnY, 80, 30, SWP_NOZORDER);
                SetWindowPos(data->hPerfBtn, NULL, 10, btnY + 40, 80, 30, SWP_NOZORDER);
                SetWindowPos(data->hHealthBtn, NULL, 100, btnY + 40, 80, 30, SWP_NOZORDER);
                SetWindowPos(data->hEjectBtn, NULL, 190, btnY + 40, 70, 30, SWP_NOZORDER);
            }
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            switch (wmId) {
                case IDC_REFRESH_BTN:
                    UpdateDriveList(data);
                    break;
                    
                case IDC_PROPERTIES_BTN:
                    if (data && data->selectedDrive >= 0) {
                        ShowDiskInfo(data, data->selectedDrive);
                    } else {
                        MessageBox(hWnd, "请先选择一个驱动器", "提示", MB_OK);
                    }
                    break;
                    
                case IDC_PERF_BTN:
                    if (data && data->selectedDrive >= 0) {
                        ShowPerformanceTest(data, data->selectedDrive);
                    } else {
                        MessageBox(hWnd, "请先选择一个驱动器", "提示", MB_OK);
                    }
                    break;
                    
                case IDC_HEALTH_BTN:
                    if (data && data->selectedDrive >= 0) {
                        ShowDiskHealth(data, data->selectedDrive);
                    } else {
                        MessageBox(hWnd, "请先选择一个驱动器", "提示", MB_OK);
                    }
                    break;
                    
                case IDC_EJECT_BTN:
                    if (data && data->selectedDrive >= 0) {
                        EjectDrive(data, data->selectedDrive);
                    } else {
                        MessageBox(hWnd, "请先选择一个驱动器", "提示", MB_OK);
                    }
                    break;
                    
                case IDM_FILE_EXIT:
                    DestroyWindow(hWnd);
                    break;
                    
                case IDM_HELP_ABOUT:
                    ShowAboutDialog(hWnd);
                    break;
            }
            break;
        }
        
        case WM_NOTIFY: {
            if (data && ((LPNMHDR)lParam)->hwndFrom == data->hDriveList) {
                switch (((LPNMHDR)lParam)->code) {
                    case LVN_ITEMCHANGED: {
                        NMLISTVIEW* pnmv = (NMLISTVIEW*)lParam;
                        if ((pnmv->uChanged & LVIF_STATE) && 
                            (pnmv->uNewState & LVIS_SELECTED)) {
                            data->selectedDrive = (int)pnmv->lParam;
                            ShowDiskInfo(data, data->selectedDrive);
                        }
                        break;
                    }
                    
                    case NM_DBLCLK: {
                        if (data->selectedDrive >= 0) {
                            ShowDiskInfo(data, data->selectedDrive);
                        }
                        break;
                    }
                }
            }
            break;
        }
        
        case WM_TIMER: {
            if (wParam == IDC_UPDATE_TIMER && data && !data->isUpdating) {
                // 可以在这里添加自动刷新逻辑
                // UpdateDriveList(data);
            }
            break;
        }
        
        case WM_DESTROY: {
            if (data) {
                KillTimer(hWnd, IDC_UPDATE_TIMER);
                if (data->diskInfos) free(data->diskInfos);
                free(data);
            }
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
