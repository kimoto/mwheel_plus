#pragma once
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WindowsX.h>
#include <stdlib.h>
#include <tchar.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm")

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")

#include <math.h>
#include <string>

// ダイアログ用のメッセージクラッカー
#define HANDLE_DLG_MSG(hwnd, msg, fn) \
    case(msg): \
	return SetDlgMsgResult(hwnd, msg, HANDLE_##msg(hwnd, wParam, lParam,fn)) 

#define SLIDER_GETPOS(lp) (::SendMessage((HWND)lp, TBM_GETPOS, 0, 0))

#ifndef DLLIMPORT
	#define DLLIMPORT extern "C" __declspec(dllimport)
#endif

#ifndef DLLEXPORT
	#define DLLEXPORT extern "C" __declspec(dllimport)
#endif

#define DUPLICATE_BOOT_CHECK(MUTEX_NAME) DuplicateBootCheck(MUTEX_NAME)

#define SafeDeleteObject(gdiobj) gdiobj != NULL && ::DeleteObject(gdiobj)

void trace(LPCTSTR format, ...);
void FillRectBrush(HDC hdc, int x, int y, int width, int height, COLORREF color);
void BorderedRect(HDC hdc, int x, int y, int width, int height, COLORREF color);
void drawRect(HDC hdc, int x, int y, int width, int height);
void mciShowLastError(MMRESULT result);
void mciAssert(MMRESULT result);
BOOL ReadWaveFile(LPTSTR lpszFileName, LPWAVEFORMATEX lpwf, LPBYTE *lplpData, LPDWORD lpdwDataSize);
BOOL LoadBitmapFromBMPFile( LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette );
BOOL LoadBitmapToDC(LPTSTR szFileName, int x, int y, HDC hdc);
void drawRectColor(HDC hdc, int x, int y, int width, int height, COLORREF color, int bold_width);
void DrawFormatText(HDC hdc, LPRECT rect, UINT type, LPCTSTR format, ...);
void TextFormatOut(HDC hdc, int x, int y, LPCTSTR format, ...);
void mciPlayBGM(LPTSTR szFileName, double volume_scale);
void ShowLastError(void);
BOOL CreateShortcut ( LPCTSTR pszTargetPath /* ターゲットパス */,
    LPCTSTR pszArguments /* 引数 */,
    LPCTSTR pszWorkPath /* 作業ディレクトリ */,
    int nCmdShow /* ShowWindowの引数 */,
    LPCSTR pszShortcutPath /* ショートカットファイル(*.lnk)のパス */ );
double GetPrivateProfileDouble(LPCTSTR section, LPCTSTR key, double def, LPCTSTR path);
BOOL WritePrivateProfileDouble(LPCTSTR section, LPCTSTR key, double val, LPCTSTR path);
BOOL WritePrivateProfileInt(LPCTSTR section, LPCTSTR key, int val, LPCTSTR path);
LPTSTR GetKeyNameTextEx(UINT vk);
LPTSTR GetKeyConfigString(int vk, int opt_vk);
void ErrorMessageBox(LPCTSTR message, ...);
BOOL GetExecuteDirectory(LPTSTR buffer, DWORD buffer_size);
BOOL SetDlgItemDouble(HWND hWnd, UINT id, double value);
double GetDlgItemDouble(HWND hWnd, UINT id);
BOOL GetDesktopPath(LPTSTR buffer, DWORD size_in_words);
BOOL SetMonitorGamma(HDC hdc, double gammaR, double gammaG, double gammaB);
BOOL SetMonitorGamma(HDC hdc, double gamma);
BOOL SetGamma(double gammaR, double gammaG, double gammaB);
BOOL SetGamma(double gamma);
BOOL SetWindowTopMost(HWND hWnd);
BOOL SetWindowTextFormat(HWND hWnd, LPTSTR format, ...);
LPTSTR sprintf_alloc(LPTSTR format, ...);
LPTSTR GetDirectoryFromPath(LPCTSTR path);
LPTSTR GetBaseName(LPCTSTR path);
LPTSTR GetBackupFilePath(LPCTSTR filePath, LPCTSTR backupExt);
BOOL BackupFile(LPCTSTR filePath, LPCTSTR backupExt);
BOOL RestoreFile(LPCTSTR filePath, LPCTSTR backupExt);
LPTSTR GetWindowTitle(HWND hWnd);
BOOL ShowContextMenu(HWND hWnd, UINT menuID);
void TasktrayAddIcon(HINSTANCE hInstance, UINT msg, UINT id, UINT iconId, LPCTSTR tips, HWND hWnd);
void TasktrayModifyIcon(HINSTANCE hInstance, UINT msg, UINT id, HWND hWnd,  LPCTSTR tips, UINT icon);
void TasktrayDeleteIcon(HWND hWnd, UINT id);
HWND WindowFromCursorPos();
void NoticeRedraw(HWND hWnd);
void RectangleNormalize(RECT *rect);
std::wstring str2wstr(std::string str);
LPTSTR GetConfigPath(LPTSTR fileName);

// mouse proxy
LRESULT CALLBACK MouseEventProxyHook(int nCode, WPARAM wp, LPARAM lp);
BOOL StartMouseEventProxy(HWND hWnd, HINSTANCE hInstance);
BOOL StopMouseEventProxy();

// window manipulate
BOOL HighlightWindow(HWND hWnd, int bold, COLORREF color);
BOOL HighlightWindow(HWND hWnd);

void DuplicateBootCheck(LPCTSTR mutexName);

void ShadowTextFormatOut(HDC hdc, int x, int y, int w, COLORREF shadow, COLORREF color, LPCTSTR format, ...);

void StickRect(RECT *selected, RECT *target, int w_px, int h_px);
void CorrectRect(RECT *selected, RECT *target);

// 多重起動防止用簡易クラス
#include <exception>
class CMutex{
private:
	HANDLE m_hMutex;
public:
	CMutex(){
	}
	
	~CMutex(){
		::ReleaseMutex(this->m_hMutex);
		::CloseHandle(this->m_hMutex);
	}

	void createMutex(LPCTSTR mutexName){
		this->m_hMutex = CreateMutex(NULL, TRUE, mutexName);
		if(GetLastError() == ERROR_ALREADY_EXISTS){
			ReleaseMutex(this->m_hMutex);
			CloseHandle(this->m_hMutex);
			throw ::std::exception();
		}
	}
};