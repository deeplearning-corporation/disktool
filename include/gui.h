#ifndef GUI_H
#define GUI_H

#include <windows.h>
#include <commctrl.h>
#include "disk_info.h"
#include "disk_utils.h"

// 控件ID
#define IDC_DRIVE_LIST      1001
#define IDC_INFO_EDIT       1002
#define IDC_PROGRESS_BAR    1003
#define IDC_STATUS_TEXT     1004
#define IDC_REFRESH_BTN     1005
#define IDC_PROPERTIES_BTN  1006
#define IDC_PERF_BTN        1007
#define IDC_HEALTH_BTN      1008
#define IDC_EJECT_BTN       1009
#define IDC_UPDATE_TIMER    1010

// 菜单ID
#define IDM_FILE_EXIT       2001
#define IDM_HELP_ABOUT      2002

// 主窗口数据
typedef struct {
    HWND hMainWnd;
    HWND hDriveList;
    HWND hInfoEdit;
    HWND hProgressBar;
    HWND hStatusText;
    HWND hRefreshBtn;
    HWND hPropertiesBtn;
    HWND hPerfBtn;
    HWND hHealthBtn;
    HWND hEjectBtn;
    
    // 数据
    DiskInfo* diskInfos;
    int diskCount;
    int selectedDrive;
    
    // 状态
    BOOL isUpdating;
} MainWindowData;

// 函数声明
BOOL InitGUI(HINSTANCE hInstance);
HWND CreateMainWindow(HINSTANCE hInstance);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL UpdateDriveList(MainWindowData* data);
void ShowDiskInfo(MainWindowData* data, int driveIndex);
void ShowDiskHealth(MainWindowData* data, int driveIndex);
void ShowPerformanceTest(MainWindowData* data, int driveIndex);
BOOL EjectDrive(MainWindowData* data, int driveIndex);
void ShowAboutDialog(HWND hParent);

#endif // GUI_H
