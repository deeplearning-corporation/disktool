#include <windows.h>
#include "gui.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    
    // 初始化GUI
    if (!InitGUI(hInstance)) {
        MessageBox(NULL, "初始化失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // 创建主窗口
    if (!CreateMainWindow(hInstance)) {
        MessageBox(NULL, "创建窗口失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // 消息循环
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
