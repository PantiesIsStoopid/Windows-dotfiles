#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
// clang-format off
#include <shellapi.h>
// clang-format on

#define HOTKEY_ID 1
#define TRAY_ICON_ID 1001
#define WM_TRAYICON (WM_USER + 1)

HINSTANCE g_hInstance;
HWND g_hWnd;
NOTIFYICONDATA nid = {0};

void ResizeActiveWindow() {
  HWND hWnd = GetForegroundWindow();
  if (!hWnd)
    return;

  DWORD fgPid = 0, thisPid = GetCurrentProcessId();
  GetWindowThreadProcessId(hWnd, &fgPid);

  // Skip if window belongs to another elevated or system process
  if (fgPid != thisPid) {
    DWORD fgAccess = PROCESS_QUERY_LIMITED_INFORMATION;
    HANDLE hProc = OpenProcess(fgAccess, FALSE, fgPid);
    if (!hProc)
      return;
    CloseHandle(hProc);
  }

  if (IsZoomed(hWnd))
    ShowWindow(hWnd, SW_RESTORE);

  MONITORINFO mi = {sizeof(mi)};
  HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
  GetMonitorInfo(hMon, &mi);
  RECT workArea = mi.rcWork;

  int screenW = workArea.right - workArea.left;
  int screenH = workArea.bottom - workArea.top;

  int newW = (int)(screenW * 0.67);
  int newH = (int)(screenH * 0.67);
  int newX = workArea.left + (screenW - newW) / 2;
  int newY = workArea.top + (screenH - newH) / 2;

  BOOL success = MoveWindow(hWnd, newX, newY, newW, newH, TRUE);
  if (!success) {
    MessageBox(
        NULL,
        L"Unable to move the active window. Some apps block repositioning.",
        L"Resize Failed", MB_ICONWARNING);
  }
}

void ShowTrayMenu() {
  POINT pt;
  GetCursorPos(&pt);

  HMENU hMenu = CreatePopupMenu();
  InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 1, L"Exit");

  SetForegroundWindow(g_hWnd);
  TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, g_hWnd,
                 NULL);
  DestroyMenu(hMenu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
  case WM_HOTKEY:
    if (wParam == HOTKEY_ID)
      ResizeActiveWindow();
    break;

  case WM_TRAYICON:
    switch (lParam) {
    case WM_RBUTTONUP:
      ShowTrayMenu();
      break;
    case WM_LBUTTONDBLCLK:
      ResizeActiveWindow();
      break;
    }
    break;

  case WM_COMMAND:
    if (LOWORD(wParam) == 1) {
      Shell_NotifyIcon(NIM_DELETE, &nid);
      PostQuitMessage(0);
    }
    break;

  case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, &nid);
    UnregisterHotKey(hwnd, HOTKEY_ID);
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
  g_hInstance = hInstance;

  const wchar_t CLASS_NAME[] = L"HotkeyTrayWindow";
  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);

  g_hWnd = CreateWindowEx(0, CLASS_NAME, L"Hotkey Tray", 0, 0, 0, 0, 0, NULL,
                          NULL, hInstance, NULL);
  if (!g_hWnd)
    return 0;

  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = g_hWnd;
  nid.uID = TRAY_ICON_ID;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAYICON;
  nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(wchar_t),
           L"Ctrl+Alt+M to resize window\nDouble-click to resize\nRight click "
           L"to exit");
  Shell_NotifyIcon(NIM_ADD, &nid);

  if (!RegisterHotKey(g_hWnd, HOTKEY_ID, MOD_CONTROL | MOD_ALT, 'M')) {
    MessageBox(NULL, L"Failed to register hotkey", L"Error", MB_ICONERROR);
    return 0;
  }

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
