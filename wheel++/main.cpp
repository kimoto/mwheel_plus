#include <Windows.h>
#include <WindowsX.h>
#include "Util.h"
#include "resource.h"

// �O���[�o���ϐ�:
TCHAR szTitle[256]=L"wheel++"; // �^�C�g�� �o�[�̃e�L�X�g
TCHAR szWindowClass[256]=L"wheelWindowClass"; // ���C�� �E�B���h�E �N���X��
HWND g_hWnd = NULL;
HINSTANCE g_hInstance = NULL; // ���݂̃C���^�[�t�F�C�X

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̐錾��]�����܂�:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

#ifdef _WIN64
#pragma comment(lib, "WndProcHack64")
#else
#pragma comment(lib, "WndProcHack32")
#endif
DLLIMPORT BOOL StartHook(HWND hWnd);
DLLIMPORT BOOL StopHook();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: �����ɃR�[�h��}�����Ă��������B
	MSG msg;

	// �O���[�o������������������Ă��܂��B
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow)){
		return FALSE;
	}

	// ���C�� ���b�Z�[�W ���[�v:
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

void TasktrayAddIcon(HINSTANCE hInstance, HWND hWnd)
{
#define WM_TASKTRAY (WM_APP + 1)
#define ID_TASKTRAY 1
#define S_TASKTRAY_TIPS L"wheel++"
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof( NOTIFYICONDATA );
	nid.uFlags           = (NIF_ICON|NIF_MESSAGE|NIF_TIP);
	nid.hWnd             = hWnd;           // �E�C���h�E�E�n���h��
	nid.hIcon            = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));          // �A�C�R���E�n���h��
	nid.uID              = ID_TASKTRAY; 	// �A�C�R�����ʎq�̒萔
	nid.uCallbackMessage = WM_TASKTRAY;    // �ʒm���b�Z�[�W�̒萔
	lstrcpy( nid.szTip, S_TASKTRAY_TIPS );  // �`�b�v�w���v�̕�����

	// �A�C�R���̕ύX
	if( !Shell_NotifyIcon( NIM_ADD, &nid ) )
		::ShowLastError();
}

void TasktrayModifyIcon(HINSTANCE hInstance, HWND hWnd, UINT icon)
{
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof( NOTIFYICONDATA );
	nid.uFlags           = (NIF_ICON|NIF_MESSAGE|NIF_TIP);
	nid.hWnd             = hWnd;           // �E�C���h�E�E�n���h��
	nid.hIcon            = ::LoadIcon(hInstance, MAKEINTRESOURCE(icon));          // �A�C�R���E�n���h��
	nid.uID              = ID_TASKTRAY; 	// �A�C�R�����ʎq�̒萔
	nid.uCallbackMessage = WM_TASKTRAY;    // �ʒm���b�Z�[�W�̒萔
	lstrcpy( nid.szTip, S_TASKTRAY_TIPS );  // �`�b�v�w���v�̕�����

	if( !::Shell_NotifyIcon(NIM_MODIFY, &nid) )
		::ShowLastError();
}

void TasktrayDeleteIcon(HWND hWnd)
{
	NOTIFYICONDATA nid; 
	nid.cbSize = sizeof(NOTIFYICONDATA); 
	nid.hWnd = hWnd;				// ���C���E�B���h�E�n���h��
	nid.uID = ID_TASKTRAY;			// �R���g���[��ID
	
	if( !::Shell_NotifyIcon(NIM_DELETE, &nid) )
		::ShowLastError();
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	g_hInstance = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂�

	hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	g_hWnd = hWnd;

	if(!hWnd){
		::ShowLastError();
		return FALSE;
	}

	// �A�C�R���̐ݒ�
	TasktrayAddIcon(g_hInstance, hWnd);

	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);
	return TRUE;
}

bool bToggle = false;
void toggleHook(HWND hWnd)
{
	if(bToggle){
		StartHook(hWnd);
		::TasktrayModifyIcon(g_hInstance, hWnd, IDI_MAIN);
		::trace(L"hook start\n");
	}else{
		StopHook();
		::TasktrayModifyIcon(g_hInstance, hWnd, IDI_MAIN_STOP);
		::trace(L"hook stop\n");
	}
	bToggle = !bToggle;
}

#define IDM_TOGGLE 1
#define IDM_EXIT 2

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu;
	static HMENU hSubMenu;
	static UINT msgTaskbarCreated;

	switch(message){
	case WM_CREATE:
		// ���j���[�̏���
		hMenu = ::CreateMenu();
		hSubMenu = ::CreateMenu();
			
		//::AppendMenu(hSubMenu, MF_STRING, IDM_TOGGLE, L"�g�O��");
		::AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"�I��");
		::AppendMenu(hMenu, MF_POPUP, (UINT)hSubMenu, NULL);

		::StartHook(hWnd);

		// �^�X�N�o�[�̃C�x���g�쐬
		msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");
		return 0;
	case WM_DESTROY:
		::TasktrayDeleteIcon(hWnd);
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
			TasktrayAddIcon(g_hInstance, hWnd);
		}
	}
	return ::DefWindowProc(hWnd, message, wParam, lParam);
}
