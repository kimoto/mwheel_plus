#pragma once
#define WIN32_LEAN_AND_MEAN
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

// ダイアログ用のメッセージクラッカー
#define HANDLE_DLG_MSG(hwnd, msg, fn) \
    case(msg): \
	return SetDlgMsgResult(hwnd, msg, HANDLE_##msg(hwnd, wParam, lParam,fn)) 

#define SLIDER_GETPOS(lp) (::SendMessage((HWND)lp, TBM_GETPOS, 0, 0))

#define DLLIMPORT extern "C" __declspec(dllimport)
#define DLLEXPORT extern "C" __declspec(dllimport)

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
