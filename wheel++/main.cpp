#include <Windows.h>
#include <WindowsX.h>
#include "Util.h"
#include "resource.h"
#include <exception>

#define CLASSNAME L"wheel++"
#define MUTEX_NAME CLASSNAME

#define WM_TASKTRAY (WM_APP + 1)
#define ID_TASKTRAY 1
#define S_TASKTRAY_TIPS L"wheel++"

#define IDM_TOGGLE 1
#define IDM_EXIT 2

// グローバル変数:
TCHAR szTitle[]=CLASSNAME; // タイトル バーのテキスト
TCHAR szWindowClass[]=CLASSNAME; // メイン ウィンドウ クラス名
HWND g_hWnd = NULL;
HINSTANCE g_hInstance = NULL; // 現在のインターフェイス

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
HANDLE g_hMutex = INVALID_HANDLE_VALUE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ここにコードを挿入してください。
	MSG msg;

	// グローバル文字列を初期化しています。
	MyRegisterClass(hInstance);

	// 多重起動防止
  CMutex mutex;
	try{
		mutex.createMutex(MUTEX_NAME);
	}catch(std::exception e){
		::ErrorMessageBox(L"多重起動です");
		exit(0);
	}

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow)){
		return FALSE;
	}
	
	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0)){
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	//wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	g_hInstance = hInstance; // グローバル変数にインスタンス処理を格納します

	hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	g_hWnd = hWnd;

	if(!hWnd){
		::ShowLastError();
		return FALSE;
	}

	// アイコンの設定
	TasktrayAddIcon(g_hInstance, WM_TASKTRAY, ID_TASKTRAY, IDI_MAIN, S_TASKTRAY_TIPS, hWnd);

	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);
	return TRUE;
}

HHOOK g_mouseHook = NULL;
LRESULT CALLBACK MouseHookProc( int nCode, WPARAM wp, LPARAM lp)
{
	if( nCode < 0 ) //nCodeが負、HC_NOREMOVEの時は何もしない
		return CallNextHookEx( g_mouseHook, nCode, wp, lp );

	if( nCode == HC_ACTION && wp == WM_MOUSEWHEEL){
		MSLLHOOKSTRUCT *msg = (MSLLHOOKSTRUCT *)lp;
		HWND target = ::WindowFromPoint(msg->pt);

		DWORD delta = HIWORD(msg->mouseData);
		DWORD key = LOWORD(msg->mouseData);
		DWORD x = msg->pt.x;
		DWORD y = msg->pt.y;

		WPARAM w = MAKEWPARAM(key, delta);
		LPARAM l = MAKELPARAM(x, y);

		::PostMessage(target, WM_MOUSEWHEEL, w, l);
		return TRUE;
	}
	return CallNextHookEx( g_mouseHook, nCode, wp, lp );
}

void StopHook()
{
	if(::g_mouseHook){
		if(!::UnhookWindowsHookEx(::g_mouseHook)){
			::ShowLastError();
		}
		::g_mouseHook = NULL;
	}
}

void StartHook(HWND hWnd)
{
	if(g_mouseHook){
		::StopHook();
	}

	g_mouseHook = ::SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, ::g_hInstance, 0);
	if(!g_mouseHook){
		::ShowLastError();
	}
}

bool bToggle = false;
// トグルフラグの状態によってタスクトレイのアイコンを返却してくれる関数
int GetTasktrayIcon()
{
	if(::bToggle)
		return IDI_MAIN;
	else
		return IDI_MAIN_STOP;
}

void toggleHook(HWND hWnd)
{
	if(bToggle)
		StartHook(hWnd);
	else
		StopHook();	
	
	::TasktrayModifyIcon(g_hInstance, WM_TASKTRAY, ID_TASKTRAY, hWnd, S_TASKTRAY_TIPS, ::GetTasktrayIcon());
	bToggle = !bToggle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu;
	static HMENU hSubMenu;
	static UINT msgTaskbarCreated;

	switch(message){
	case WM_CREATE:
		// メニューの準備
		hMenu = ::CreateMenu();
		hSubMenu = ::CreateMenu();
			
		//::AppendMenu(hSubMenu, MF_STRING, IDM_TOGGLE, L"トグル");
		::AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"終了");
		::AppendMenu(hMenu, MF_POPUP, (UINT)hSubMenu, NULL);

		::StartHook(hWnd);

		// タスクバーのイベント作成
		msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");
		return 0;
	case WM_DESTROY:
		::TasktrayDeleteIcon(hWnd, ID_TASKTRAY);
		::StopHook();
		::PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDM_TOGGLE:
			::toggleHook(hWnd);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		}
		return 0;
	case WM_TASKTRAY:
		switch(lParam){
		case WM_RBUTTONDOWN:
			{
				POINT point;
				::GetCursorPos(&point);
				::SetForegroundWindow(hWnd);
				TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
			}
			break;
		case WM_LBUTTONDBLCLK:
			::toggleHook(hWnd);
			break;
		}
		return 0;
	default:
		if(message == msgTaskbarCreated){
			TasktrayAddIcon(g_hInstance, WM_TASKTRAY, ID_TASKTRAY, ::GetTasktrayIcon(), S_TASKTRAY_TIPS, hWnd);
		}
	}
	return ::DefWindowProc(hWnd, message, wParam, lParam);
}
