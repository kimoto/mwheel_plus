// dllmain.cpp : DLL �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
#include "stdafx.h"
#include "Util.h"

#define DLLIMPORT extern "C" __declspec(dllimport)
#define DLLEXPORT extern "C" __declspec(dllexport)
DLLEXPORT BOOL StartHook(HWND hWnd);
DLLEXPORT BOOL StopHook();

#pragma data_seg(".shared")
HHOOK g_hook = NULL;   // �t�b�N�n���h��
#pragma data_seg()
#pragma comment(linker, "/section:.shared,rws")

HINSTANCE g_hInstance;

 // TRUE�ԋp == ���b�Z�[�W�j��
LRESULT CALLBACK MouseHookProc( int nCode, WPARAM wp, LPARAM lp)
{
	if( nCode < 0 ) //nCode�����AHC_NOREMOVE�̎��͉������Ȃ�
		return CallNextHookEx( g_hook, nCode, wp, lp );

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
	return CallNextHookEx( g_hook, nCode, wp, lp );
}

DLLEXPORT BOOL StartHook(HWND hWnd)
{
	g_hook = ::SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)MouseHookProc, g_hInstance, 0);
	if(g_hook == NULL){ // �t�b�N���s�����Ƃ�
		return FALSE;
	}else{
		return TRUE;
	}
}

DLLEXPORT BOOL StopHook()
{
	if( ::UnhookWindowsHookEx(g_hook) == 0 ){
		return FALSE;
	}else{
		return TRUE;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInstance = hModule;
	return TRUE;
}

