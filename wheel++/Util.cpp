#include "Util.h"

#define TRACE_BUFFER_SIZE 1024
#define FORMAT_BUFFER_SIZE 1024
#define MCI_LASTERROR_BUFSIZE 1024

// GlobalAllocのインターフェースで、heapを確保する関数
// 過去のコード書き直すのめんどかったのでつくった
// 名前はおかしい
// 引数のflagsは完全に無視されます
LPVOID GlobalAllocHeap(UINT flags, SIZE_T size)
{
	return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, size);
}

void trace(LPCTSTR format, ...)
{
#ifdef _DEBUG
	va_list arg;
	va_start(arg, format);
	
	TCHAR buffer[TRACE_BUFFER_SIZE];
	::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);
	::OutputDebugString(buffer);	
	va_end(arg);
#endif
}

void DrawFormatText(HDC hdc, LPRECT rect, UINT type, LPCTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);
	
	TCHAR buffer[TRACE_BUFFER_SIZE];
	::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);
	::DrawText(hdc, buffer, lstrlen(buffer), rect, type);	
	va_end(arg);
}

void TextFormatOut(HDC hdc, int x, int y, LPCTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);
	
	TCHAR buffer[FORMAT_BUFFER_SIZE];
	::_vsnwprintf_s(buffer, FORMAT_BUFFER_SIZE, _TRUNCATE, format, arg);
	::TextOut(hdc, x, y, buffer, lstrlen(buffer));
	va_end(arg);
}

void BorderedRect(HDC hdc, int x, int y, int width, int height, COLORREF color)
{
	FillRectBrush(hdc, x, y, width, height, color);
	drawRect(hdc, x, y, width, height);
}

void FillRectBrush(HDC hdc, int x, int y, int width, int height, COLORREF color)
{
	RECT rect;
	rect.top = y;
	rect.left = x;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	HBRUSH brush = ::CreateSolidBrush(color);
	::FillRect(hdc, &rect, brush);
	::DeleteObject(brush);
}

void drawRect(HDC hdc, int x, int y, int width, int height)
{
	::MoveToEx(hdc, x, y, NULL);
	::LineTo(hdc, x + width - 1, y);
	::LineTo(hdc, x + width - 1, y + height);
	::LineTo(hdc, x, y + height);
	::LineTo(hdc, x, y);
}

void drawRectColor(HDC hdc, int x, int y, int width, int height, COLORREF color, int bold_width)
{
	HPEN pen = ::CreatePen(PS_SOLID, bold_width, color);
	HBRUSH oldPen = (HBRUSH)::SelectObject(hdc, pen);

	drawRect(hdc, x, y, width, height);

	::SelectObject(hdc, oldPen);
	::DeleteObject(pen);
}

void mciShowLastError(MMRESULT result)
{
	LPTSTR lpstr = (LPTSTR)::GlobalAllocHeap(GMEM_FIXED, MCI_LASTERROR_BUFSIZE);
	mciGetErrorString(result, lpstr, MCI_LASTERROR_BUFSIZE);
	::MessageBox(NULL, lpstr, L"ERROR", MB_OK);
}

void mciAssert(MMRESULT result)
{
	// エラーだったときはその内容を表示して強制終了します
	if(result != MMSYSERR_NOERROR){
		::mciShowLastError(result);
		exit(1);
	}
}

BOOL ReadWaveFile(LPTSTR lpszFileName, LPWAVEFORMATEX lpwf, LPBYTE *lplpData, LPDWORD lpdwDataSize)
{
	HMMIO    hmmio;
	MMCKINFO mmckRiff;
	MMCKINFO mmckFmt;
	MMCKINFO mmckData;
	LPBYTE   lpData;

	hmmio = mmioOpen(lpszFileName, NULL, MMIO_READ);
	if (hmmio == NULL) {
		MessageBox(NULL, TEXT("ファイルのオープンに失敗しました。"), NULL, MB_ICONWARNING);
		return FALSE;
	}
	
	mmckRiff.fccType = mmioStringToFOURCC(TEXT("WAVE"), 0);
	if (mmioDescend(hmmio, &mmckRiff, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR) {
		MessageBox(NULL, TEXT("WAVEファイルではありません。"), NULL, MB_ICONWARNING);
		mmioClose(hmmio, 0);
		return FALSE;
	}

	mmckFmt.ckid = mmioStringToFOURCC(TEXT("fmt "), 0);
	if (mmioDescend(hmmio, &mmckFmt, NULL, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	mmioRead(hmmio, (HPSTR)lpwf, mmckFmt.cksize);
	mmioAscend(hmmio, &mmckFmt, 0);
	if (lpwf->wFormatTag != WAVE_FORMAT_PCM) {
		MessageBox(NULL, TEXT("PCMデータではありません。"), NULL, MB_ICONWARNING);
		mmioClose(hmmio, 0);
		return FALSE;
	}

	mmckData.ckid = mmioStringToFOURCC(TEXT("data"), 0);
	if (mmioDescend(hmmio, &mmckData, NULL, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
		mmioClose(hmmio, 0);
		return FALSE;
	}
	lpData = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, mmckData.cksize);
	mmioRead(hmmio, (HPSTR)lpData, mmckData.cksize);
	mmioAscend(hmmio, &mmckData, 0);

	mmioAscend(hmmio, &mmckRiff, 0);
	mmioClose(hmmio, 0);

	*lplpData = lpData;
	*lpdwDataSize = mmckData.cksize;

	return TRUE;
}

BOOL LoadBitmapFromBMPFile( LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette )
{
	BITMAP  bm;

	*phBitmap = NULL;
	*phPalette = NULL;

	// Use LoadImage() to get the image loaded into a DIBSection
	*phBitmap = (HBITMAP)LoadImage( NULL, szFileName, IMAGE_BITMAP, 0, 0,
				LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
	if( *phBitmap == NULL )
		return FALSE;

	// Get the color depth of the DIBSection
	GetObject(*phBitmap, sizeof(BITMAP), &bm );
	// If the DIBSection is 256 color or less, it has a color table
	if( ( bm.bmBitsPixel * bm.bmPlanes ) <= 8 )
	{
		HDC           hMemDC;
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;
		WORD          i;

		// Create a memory DC and select the DIBSection into it
		hMemDC = CreateCompatibleDC( NULL );
		hOldBitmap = (HBITMAP)SelectObject( hMemDC, *phBitmap );
		// Get the DIBSection's color table
		GetDIBColorTable( hMemDC, 0, 256, rgb );
		// Create a palette from the color tabl
		pLogPal = (LOGPALETTE *)malloc( sizeof(LOGPALETTE) + (256*sizeof(PALETTEENTRY)) );
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = 256;
		for(i=0;i<256;i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		*phPalette = CreatePalette( pLogPal );
		// Clean up
		free( pLogPal );
		SelectObject( hMemDC, hOldBitmap );
		DeleteDC( hMemDC );
	}
	else   // It has no color table, so use a halftone palette
	{
		HDC    hRefDC;

		hRefDC = GetDC( NULL );
		*phPalette = CreateHalftonePalette( hRefDC );
		ReleaseDC( NULL, hRefDC );
	}
	return TRUE;
}

BOOL LoadBitmapToDC(LPTSTR szFileName, int x, int y, HDC hdc)
{
	HBITMAP hBitmap2, hOldBitmap2;
	HPALETTE hPalette2, hOldPalette2;
	HDC hMemDC;
	BITMAP bm;

	if( LoadBitmapFromBMPFile(szFileName, &hBitmap2, &hPalette2) )
	{
		::GetObject(hBitmap2, sizeof(BITMAP), &bm);
		hMemDC = ::CreateCompatibleDC(hdc);
		hOldBitmap2 = (HBITMAP)::SelectObject(hMemDC, hBitmap2);
		hOldPalette2 = ::SelectPalette(hdc, hPalette2, FALSE);
		::RealizePalette(hdc);
			
		::BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight,
			hMemDC, 0, 0, SRCAND);

		::SelectObject(hMemDC, hOldBitmap2);
		::DeleteObject(hBitmap2);

		::SelectPalette(hdc, hOldPalette2, FALSE);
		::DeleteObject(hPalette2);
		return TRUE;
	}else{
		::OutputDebugString(L"error loading bitmap\n");
		return FALSE;
	}
}

// .wav information
WAVEFORMATEX wfe;
WAVEHDR whdr[2]; // double buffering
HWAVEOUT hWaveOut;
LPBYTE lpWaveData;
DWORD dwDataSize;
int sound_ptr = 0;

void CALLBACK musicCallback(
	HWAVEOUT hwo , UINT uMsg,         
	DWORD dwInstance,  
	DWORD dwParam1, DWORD dwParam2     
){
	if(uMsg == MM_WOM_OPEN)
		trace(L"open\n");
	if(uMsg == MM_WOM_CLOSE)
		trace(L"close\n");
	if(uMsg == MM_WOM_DONE){
		trace(L"done\n");

		if(sound_ptr >= 2){ // 2回目のバッファが終わったら、最初にバッファを参照するように(double buffering)
			sound_ptr = 0;
		}
		::mciAssert( ::waveOutWrite(hWaveOut, &whdr[sound_ptr++], sizeof(WAVEHDR)) );
	}
}

void mciPlayBGM(LPTSTR szFileName, double volume_scale)
{
	if(!ReadWaveFile(szFileName, &wfe, &lpWaveData, &dwDataSize)){
		MessageBox(NULL, TEXT("WAVEデバイスの読み込みに失敗しました。"), NULL, MB_ICONWARNING);
		return;
	}

	if(::waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfe,(DWORD_PTR)&musicCallback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR){
		MessageBox(NULL, TEXT("WAVEデバイスのオープンに失敗しました。"), NULL, MB_ICONWARNING);
		return;
	}

	for(int i=0; i<2; i++){
		whdr[i].lpData = (LPSTR)lpWaveData;
		whdr[i].dwBufferLength = dwDataSize;
		whdr[i].dwFlags = 0;

		::waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(WAVEHDR));

		// 最初は両方のバッファを再生する
		::mciAssert( ::waveOutWrite(hWaveOut, &whdr[i], sizeof(WAVEHDR)) );
	}

	// 音量の設定
	DWORD left = (DWORD)(0xFFFF * volume_scale); // 0xFFFF = max volume
	DWORD right = left;
	DWORD dwVolume = MAKELONG(left, right);
	::mciAssert(::waveOutSetVolume(hWaveOut, dwVolume));
}

void ShowLastError(void){
	LPVOID lpMessageBuffer;
  
	FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER |
	FORMAT_MESSAGE_FROM_SYSTEM,
	NULL,
	GetLastError(),
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // デフォルト ユーザー言語 
	(LPTSTR) &lpMessageBuffer,
	0,
	NULL );

	MessageBox(NULL, (LPCWSTR)lpMessageBuffer, TEXT("Error"), MB_OK);
  
	//... 文字列が表示されます。
	// システムによって確保されたバッファを開放します。
	LocalFree( lpMessageBuffer );
}


// ショートカット作成
BOOL CreateShortcut ( LPCTSTR pszTargetPath /* ターゲットパス */,
    LPCTSTR pszArguments /* 引数 */,
    LPCTSTR pszWorkPath /* 作業ディレクトリ */,
    int nCmdShow /* ShowWindowの引数 */,
    LPCSTR pszShortcutPath /* ショートカットファイル(*.lnk)のパス */ )
{
    IShellLink *psl = NULL;
    IPersistFile *ppf = NULL;
    enum
    {
        MY_MAX_PATH = 65536
    };
    TCHAR wcLink[ MY_MAX_PATH ]=_T("");

    // IShellLinkインターフェースの作成
    HRESULT result = CoCreateInstance( CLSID_ShellLink, NULL,CLSCTX_INPROC_SERVER, IID_IShellLink, ( void ** ) &psl);
	if(FAILED(result))
    {
		return result;
	}

    // 設定
    psl->SetPath ( pszTargetPath );
    psl->SetArguments ( pszArguments );
    psl->SetWorkingDirectory ( pszWorkPath );
    psl->SetShowCmd ( nCmdShow );

    // IPersistFileインターフェースの作成
    if ( FAILED ( psl->QueryInterface ( IID_IPersistFile, ( void ** ) &ppf ) ) )
    {
        psl->Release ();
        return FALSE;
    }
    
    // lpszLinkをWCHAR型に変換
    MultiByteToWideChar ( CP_ACP, 0, pszShortcutPath, -1, wcLink, MY_MAX_PATH );
    if ( FAILED ( ppf->Save ( wcLink, TRUE ) ) )
    {
        ppf->Release ();
        return FALSE;
    }

	result = ppf->Save((LPCOLESTR)pszShortcutPath,TRUE);
	
    // 解放
    ppf->Release ();
    psl->Release ();

    return TRUE;
}

#define PROFILE_STRING_BUFFER_SIZE 256
double GetPrivateProfileDouble(LPCTSTR section, LPCTSTR key, double def, LPCTSTR path)
{
	TCHAR buf[PROFILE_STRING_BUFFER_SIZE];
	::GetPrivateProfileString(section, key, L"NOTFOUND", buf, sizeof(buf), path);

	if(::wcscmp(buf, L"NOTFOUND") == 0 || ::wcscmp(buf, L"") == 0)
		return def;
	return ::_wtof(buf);
}

BOOL WritePrivateProfileDouble(LPCTSTR section, LPCTSTR key, double val, LPCTSTR path)
{
	TCHAR buf[PROFILE_STRING_BUFFER_SIZE];
	::_stprintf_s(buf, L"%.2f", val);
	return ::WritePrivateProfileString(section, key, buf, path);
}

BOOL WritePrivateProfileInt(LPCTSTR section, LPCTSTR key, int val, LPCTSTR path)
{
	TCHAR buf[PROFILE_STRING_BUFFER_SIZE];
	::_stprintf_s(buf, L"%d", val);
	return ::WritePrivateProfileString(section, key, buf, path);
}

// 指定された仮想キーの文字列表現を
// 動的に確保したバッファ領域に格納して返却します
#define KEYNAMETEXT_BUFFER_SIZE 256
LPTSTR GetKeyNameTextEx(UINT vk)
{
	UINT uScanCode = ::MapVirtualKey(vk, 0);
	LPARAM lParam = (uScanCode << 16);

	switch (vk) {
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_END:
	case VK_HOME:
	case VK_INSERT:
	case VK_DELETE:
	case VK_DIVIDE:
	case VK_NUMLOCK:
		lParam = (uScanCode << 16) | (1 << 24);
		break;
	}

	LPTSTR buffer = (LPTSTR)::GlobalAllocHeap(GMEM_FIXED | GMEM_ZEROINIT, KEYNAMETEXT_BUFFER_SIZE);
	::GetKeyNameText(lParam, buffer, KEYNAMETEXT_BUFFER_SIZE);
	return buffer;
}

// 指定されたキーの組み合わせのの文字列表現を取得します
// 返却されるバッファは動的に確保しますので、使用後は解放が必要です
// たとえば VK_ENTER + VK_CONTROLの場合は "Ctrl + ENTER" みたいな文字列に変換される
LPTSTR GetKeyConfigString(int vk, int opt_vk)
{
	// wp == optのときは、Ctrl + Ctrlとかなので重複して表示させないようにケア
	if( vk == opt_vk )
		opt_vk = NULL;

	LPTSTR vk_str = ::GetKeyNameTextEx(vk);
	LPTSTR opt_vk_str = ::GetKeyNameTextEx(opt_vk);

	LPTSTR s_buffer = (LPTSTR)::GlobalAllocHeap(GMEM_FIXED | GMEM_ZEROINIT, 256);
	if(opt_vk != NULL){ // CONTROL,ALTキーが設定されていた場合はALT + Bみたいな文字列になる
		::wsprintf(s_buffer, L"%s + %s", opt_vk_str, vk_str);
	}else{
		::wsprintf(s_buffer, vk_str);
	}

	::GlobalFree(vk_str);
	::GlobalFree(opt_vk_str);

	return s_buffer;
}

void ErrorMessageBox(LPCTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);

	TCHAR buffer[TRACE_BUFFER_SIZE];
	::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);
	::MessageBox(NULL, buffer, L"Error", MB_OK);
	va_end(arg);
}

// 現在のプロセスの実行ファイルの存在するパスを取得します
BOOL GetExecuteDirectory(LPTSTR buffer, DWORD size_in_words)
{
	TCHAR modulePath[MAX_PATH]; 
	TCHAR drive[MAX_PATH];
	TCHAR dir[MAX_PATH];

	DWORD result = ::GetModuleFileName(NULL, modulePath, MAX_PATH);
	if(result == 0){
		return FALSE;
	}

	if( ::_wsplitpath_s(modulePath, drive, MAX_PATH, dir, MAX_PATH, NULL, 0, NULL, 0) != 0 )
		return FALSE;

	if( ::_snwprintf_s(buffer, size_in_words, _TRUNCATE, L"%s%s", drive, dir) < 0 )
		return FALSE;

	return TRUE;
}

#define SETDOUBLE_BUFFER_SIZE 256
BOOL SetDlgItemDouble(HWND hWnd, UINT id, double value)
{
	TCHAR buf[SETDOUBLE_BUFFER_SIZE]=_T("");
	::_stprintf_s(buf, SETDOUBLE_BUFFER_SIZE, _T("%.2f"), value);
	return ::SetDlgItemText(hWnd, id, (LPCTSTR)buf);
}

double GetDlgItemDouble(HWND hWnd, UINT id)
{
	TCHAR buf[SETDOUBLE_BUFFER_SIZE]=TEXT("");
	::GetDlgItemText(hWnd, id, (LPTSTR)&buf, sizeof(buf));
	return ::_wtof(buf);
}

BOOL GetDesktopPath(LPTSTR buffer, DWORD size_in_words)
{
	/* SHGetPathFromIDListがバッファのサイズを検証してくれないので
	勝手に「最低でもMAX_PATH文字数分のバッファが確保されていること」を確認します */
	if(size_in_words < MAX_PATH){
		return FALSE;
	}

	LPITEMIDLIST lpidlist;
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &lpidlist );
	SHGetPathFromIDList( lpidlist, buffer );
	return TRUE;
}

// モニタ個別にガンマを設定
BOOL SetMonitorGamma(HDC hdc, double gammaR, double gammaG, double gammaB)
{
	gammaR = 1.0 / gammaR;
	gammaG = 1.0 / gammaG;
	gammaB = 1.0 / gammaB;

	WORD ramp[256*3];
	for(int i=0; i<256; i++){
		double valueR = pow((i + 1) / 256.0, gammaR) * 65536;
		double valueG = pow((i + 1) / 256.0, gammaG) * 65536;
		double valueB = pow((i + 1) / 256.0, gammaB) * 65536;
		
		if(valueR < 0) valueR = 0; if(valueR > 65535) valueR = 65535;
		if(valueG < 0) valueG = 0; if(valueG > 65535) valueG = 65535;
		if(valueB < 0) valueB = 0; if(valueB > 65535) valueB = 65535;
		
		ramp[0+i] = (WORD)valueR;
		ramp[256+i] = (WORD)valueG;
		ramp[512+i] = (WORD)valueB;
	}
	return !::SetDeviceGammaRamp(hdc, &ramp);
}

BOOL SetMonitorGamma(HDC hdc, double gamma)
{
	return SetMonitorGamma(hdc, gamma, gamma, gamma);
}

// すべてのモニタ共通にガンマを設定
BOOL SetGamma(double gammaR, double gammaG, double gammaB)
{
	return SetMonitorGamma(::GetDC(NULL), gammaR, gammaG, gammaB);
}

BOOL SetGamma(double gamma)
{
	return SetGamma(gamma, gamma, gamma);
}

BOOL SetWindowTopMost(HWND hWnd)
{
	return SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
}

LPTSTR sprintf_alloc(LPTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);
	
	LPTSTR buffer = (LPTSTR)::GlobalAllocHeap(GMEM_FIXED | GMEM_ZEROINIT, TRACE_BUFFER_SIZE * sizeof(TCHAR));
	::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);
	va_end(arg);

	return buffer;
}

BOOL SetWindowTextFormat(HWND hWnd, LPTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);

	TCHAR buffer[TRACE_BUFFER_SIZE];
	::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);
	::SetWindowText(hWnd, buffer);
	va_end(arg);

	return TRUE;
}

LPTSTR GetDirectoryFromPath(LPCTSTR path)
{
	TCHAR drive[MAX_PATH];
	TCHAR dir[MAX_PATH];
	
	::_wsplitpath_s(path, drive, MAX_PATH, dir, MAX_PATH, NULL, 0, NULL, 0);
	return ::sprintf_alloc(L"%s%s", drive, dir);
}

LPTSTR GetBaseName(LPCTSTR path)
{
	TCHAR filename[MAX_PATH];
	TCHAR ext[MAX_PATH];
	::_wsplitpath_s(path, NULL, 0, NULL, 0, filename, MAX_PATH, ext, MAX_PATH);
	return ::sprintf_alloc(L"%s%s", filename, ext);
}

// ファイルパスを元に、バックアップ対象のファイルパスを構築します
// test.txt -> test.txt.bak みたいなかんじ
LPTSTR GetBackupFilePath(LPCTSTR filePath, LPCTSTR backupExt)
{
	// 指定されたファイルのDirectoryを取得する
	LPTSTR directoryPath = ::GetDirectoryFromPath(filePath);
	LPTSTR baseName = ::GetBaseName(filePath);

	// バックアップ先ファイル名構築
	LPTSTR backupFilePath = ::sprintf_alloc(L"%s%s%s", directoryPath, baseName, backupExt);
	
	// 解放します
	::GlobalFree(directoryPath);
	::GlobalFree(baseName);
	return backupFilePath;
}

// 指定されたパスと同じ場所にバックアップファイルを作成します
// 元のファイル名 + .bak
// たとえばtest.txtのバックアップファイルはtest.txt.bakになります
// filepathはfullpathであること
BOOL BackupFile(LPCTSTR filePath, LPCTSTR backupExt=L".bak")
{
	BOOL bRet = FALSE;

	// バックアップ対象ファイルが実際に存在することを確認
	if( ::PathFileExists(filePath) ) {
		// 実際のバックアップ処理
		LPTSTR backupFilePath = ::GetBackupFilePath(filePath, backupExt);
		
		trace(L"BackupFile: %s -> %s\n", filePath, backupFilePath);
		if( !::CopyFile(filePath, backupFilePath, FALSE) ){ // すでにファイルがあっても上書きします
			//::ShowLastError();
			//::ErrorMessageBox(L"バックアップ先にすでにファイルが存在しています\n%s", backupFilePath);
			bRet = FALSE;
		}else{
			bRet = TRUE; // SUCCESS!
		}

		::GlobalFree(backupFilePath);
	}else{
		// 対象ファイルなかったらエラー
		::ShowLastError();
		bRet = FALSE;
	}

	return bRet;
}

// 指定されたパスをもとに、バックアップファイル名を推測し
// 復元処理を行います
// 元のファイル名 + .bakのファイルを探してきて、copy 元のファイル名.bak -> 元のファイル名します
// filepathはfullpathであること
BOOL RestoreFile(LPCTSTR filePath, LPCTSTR backupExt=L".bak")
{
	BOOL bRet = FALSE;
	LPTSTR backupFilePath = ::GetBackupFilePath(filePath, backupExt);

	// 復元対象ファイルが実際に存在することを確認
	if( ::PathFileExists(backupFilePath) ) {
		// 実際のバックアップ処理
		trace(L"RestoreFile: %s -> %s\n", backupFilePath, filePath);
		if( !::CopyFile(backupFilePath, filePath, FALSE) ){ // すでにファイルがあっても上書きします
			//::ShowLastError();
			//::ErrorMessageBox(L"バックアップ先にすでにファイルが存在しています\n%s", backupFilePath);
			bRet = FALSE;
		}else{
			bRet = TRUE; // SUCCESS!
		}
	}else{
		// 対象ファイルなかったらエラー
		::ShowLastError();
		bRet = FALSE;
	}

	::GlobalFree(backupFilePath);
	return bRet;
}

LPTSTR GetWindowTitle(HWND hWnd)
{
	LPTSTR buffer = (LPTSTR)::GlobalAllocHeap(GMEM_FIXED, 256 * sizeof(TCHAR));
	::GetWindowText(hWnd, buffer, 256);
	return buffer;
}

// 指定されたIDのコンテキストメニューを表示します
BOOL ShowContextMenu(HWND hWnd, UINT menuID)
{
	// 実質メモリリークしてるけど何回繰り返しても一定値から増えないから気にしてない
	// おそらくwindows側で同じインスタンスが存在しないように管理してくれてる
	HMENU hMenu = ::LoadMenu(NULL, MAKEINTRESOURCE(menuID));
	HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
	
	POINT point;
	::GetCursorPos(&point);

	::SetForegroundWindow(hWnd);

	::TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
	::PostMessage(hWnd, WM_NULL, 0, 0);
	return TRUE;
}

void TasktrayAddIcon(HINSTANCE hInstance, UINT msg, UINT id, UINT iconId, LPCTSTR tips, HWND hWnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof( NOTIFYICONDATA );
	nid.uFlags           = (NIF_ICON|NIF_MESSAGE|NIF_TIP);
	nid.hWnd             = hWnd;           // ウインドウ・ハンドル
	nid.hIcon            = ::LoadIcon(hInstance, MAKEINTRESOURCE(iconId));          // アイコン・ハンドル
	nid.uID              = id; 	// アイコン識別子の定数
	nid.uCallbackMessage = msg;    // 通知メッセージの定数
	lstrcpy(nid.szTip, tips);  // チップヘルプの文字列

	// アイコンの変更
	if( !Shell_NotifyIcon( NIM_ADD, &nid ) )
		::ShowLastError();
}

void TasktrayModifyIcon(HINSTANCE hInstance, UINT msg, UINT id, HWND hWnd,  LPCTSTR tips, UINT icon)
{
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof( NOTIFYICONDATA );
	nid.uFlags           = (NIF_ICON|NIF_MESSAGE|NIF_TIP);
	nid.hWnd             = hWnd;           // ウインドウ・ハンドル
	nid.hIcon            = ::LoadIcon(hInstance, MAKEINTRESOURCE(icon));          // アイコン・ハンドル
	nid.uID              = id; 	// アイコン識別子の定数
	nid.uCallbackMessage = msg;    // 通知メッセージの定数
	lstrcpy( nid.szTip, tips );  // チップヘルプの文字列

	if( !::Shell_NotifyIcon(NIM_MODIFY, &nid) )
		::ShowLastError();
}

void TasktrayDeleteIcon(HWND hWnd, UINT id)
{
	NOTIFYICONDATA nid; 
	nid.cbSize = sizeof(NOTIFYICONDATA); 
	nid.hWnd = hWnd;				// メインウィンドウハンドル
	nid.uID = id;			// コントロールID
	
  if(!Shell_NotifyIcon(NIM_DELETE, &nid))
    ::ShowLastError();
}

HWND WindowFromCursorPos()
{
	POINT pt;
	::GetCursorPos(&pt);
	return ::WindowFromPoint(pt);
}

void NoticeRedraw(HWND hWnd)
{
	::InvalidateRect(hWnd, NULL, FALSE);
	::UpdateWindow(hWnd);
	::RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	::SendMessage(hWnd, WM_PAINT, 0, 0);
}

void RectangleNormalize(RECT *rect)
{
	// 常に左上基点の構造体に変換
	if(rect->right - rect->left < 0){
		// 左右逆
		int tmp = rect->left;
		rect->left = rect->right;
		rect->right = tmp;
	}
	if(rect->bottom - rect->top < 0){
		int tmp = rect->top;
		rect->top = rect->bottom;
		rect->bottom = tmp;
	}
}

std::wstring str2wstr(std::string str)
{
	// そのサイズだけ確保し、変換します
	wchar_t *wbuf = NULL;

	// マルチバイト文字を変換するに当たって、変換後の文字数を調べます
	int need_buf_size = ::MultiByteToWideChar(0, 0, str.c_str(), str.size(), NULL, 0);

	wbuf = new wchar_t[need_buf_size];
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), wbuf, need_buf_size);

	// 返却するためにオブジェクトにくるみます
	std::wstring result;
	result += wbuf;
	delete wbuf;
	return result;
}

// 実行ファイルのディレクトリに、config.ini(デフォルト)を付加した物になります
// 動的にバッファを格納して返却するので、解放必須です
LPTSTR GetConfigPath(LPTSTR fileName)
{
	LPTSTR lpExecDirectory = (LPTSTR)::GlobalAlloc(GMEM_FIXED, MAX_PATH * sizeof(TCHAR));
	if( ::GetExecuteDirectory(lpExecDirectory, MAX_PATH) ) {
		LPTSTR lpConfigPath = sprintf_alloc(L"%s%s", lpExecDirectory, fileName);
		::GlobalFree(lpExecDirectory);
		return lpConfigPath;
	} else {
		::GlobalFree(lpExecDirectory);
		::ShowLastError();
		return NULL;
	}
}

HHOOK g_mouseProxyHook = NULL;
HWND g_mouseProxyHwnd = NULL;
LRESULT CALLBACK MouseEventProxyHook(int nCode, WPARAM wp, LPARAM lp)
{
	if( nCode < 0 ) //nCodeが負、HC_NOREMOVEの時は何もしない
		return CallNextHookEx(g_mouseProxyHook, nCode, wp, lp );

	if( nCode == HC_ACTION){
		MSLLHOOKSTRUCT *msg = (MSLLHOOKSTRUCT *)lp;
		if( wp == WM_MOUSEMOVE ){
			::PostMessage(g_mouseProxyHwnd, wp, 0, MAKELPARAM(msg->pt.x, msg->pt.y));
			return CallNextHookEx(::g_mouseProxyHook, nCode, 0, lp);
		}

		if( wp == WM_LBUTTONDOWN || wp == WM_LBUTTONUP ||
			wp == WM_RBUTTONDOWN || wp == WM_RBUTTONUP ){
				::PostMessage(g_mouseProxyHwnd, wp, 0, MAKELPARAM(msg->pt.x, msg->pt.y));
				return TRUE;
		}
	}
	return CallNextHookEx(::g_mouseProxyHook, nCode, 0, lp);
}

BOOL StartMouseEventProxy(HWND hWnd, HINSTANCE hInstance)
{
	::g_mouseProxyHwnd = hWnd;
	::g_mouseProxyHook = ::SetWindowsHookEx(WH_MOUSE_LL, MouseEventProxyHook, hInstance, 0);
	if(!::g_mouseProxyHook){
		return FALSE;
	}
	return TRUE;
}

BOOL StopMouseEventProxy()
{
	if(g_mouseProxyHook){
		if(!UnhookWindowsHookEx(g_mouseProxyHook)){
			return FALSE;
		}
		g_mouseProxyHook = NULL;
	}
	::g_mouseProxyHwnd = NULL;
	return TRUE;
}

BOOL HighlightWindow(HWND hWnd, int bold, COLORREF color)
{
	HDC hdc = ::GetWindowDC(hWnd);
	if(hdc == NULL){
		return FALSE;
	}

	HPEN hPen = CreatePen(PS_SOLID, bold, color);
	HBRUSH hBrush = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);

	HGDIOBJ hPrevPen = ::SelectObject(hdc, hPen);
	HGDIOBJ hPrevBrush = ::SelectObject(hdc, hBrush);

	RECT rect;
	::GetWindowRect(hWnd, &rect);
	::Rectangle(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

	::SelectObject(hdc, hPrevPen);
	::SelectObject(hdc, hPrevBrush);

	::DeleteObject(hPen);
	::DeleteObject(hBrush);

	::ReleaseDC(hWnd, hdc);
	return TRUE;
}

BOOL HighlightWindow(HWND hWnd)
{
	return ::HighlightWindow(hWnd, 5, RGB(0,0,0));
}

void DuplicateBootCheck(LPCTSTR mutexName)
{
	CMutex mutex;
	try{
		mutex.createMutex(mutexName);
	}catch(std::exception e){
		::ErrorMessageBox(L"多重起動です");
		exit(0);
	}
}

void ShadowTextFormatOut(HDC hdc, int x, int y, int w, COLORREF shadow, COLORREF color, LPCTSTR format, ...)
{
	va_list arg;
	va_start(arg, format);

	TCHAR buffer[256];
	::SetBkMode(hdc, TRANSPARENT);
	::_vsnwprintf_s(buffer, 256, _TRUNCATE, format, arg);

	// 影の描画
	::SetTextColor(hdc, shadow);
	::TextOut(hdc, x + w, y + w, buffer, lstrlen(buffer));

	// 本体の描画
	::SetTextColor(hdc, color);
	::TextOut(hdc, x, y, buffer, lstrlen(buffer));

	va_end(arg);
}


void StickRect(RECT *selected, RECT *target, int w_px, int h_px)
{
	// 左側
	if(target->left <= selected->left && selected->left <= w_px){
		selected->right = target->left + (selected->right - selected->left);
		selected->left = target->left;
	}
	// 上側
	if(target->top <= selected->top && selected->top <= h_px){
		selected->bottom = target->top + (selected->bottom - selected->top);
		selected->top = target->top;
	}
	// 下側
	if(target->bottom - h_px <= selected->bottom && selected->bottom <= target->bottom){
		selected->top = target->bottom - (selected->bottom - selected->top);
		selected->bottom = target->bottom;
	}
	// 右側
	if(target->right - w_px <= selected->right && selected->right <= target->right){
		selected->left = target->right - (selected->right - selected->left);
		selected->right = target->right;
	}

	// 上下反転したときの上側
	if(selected->bottom < target->top + h_px){
		selected->top = target->top + (selected->top - selected->bottom);
		selected->bottom = target->top;
	}
	// 上下反転したときの下側
	if(target->bottom - h_px <= selected->top){
		selected->bottom = target->bottom - (selected->top - selected->bottom);
		selected->top = target->bottom;
	}
	// 左右反転したときの左側
	if(selected->right < target->left + w_px){
		selected->left = selected->left - selected->right;
		selected->right = target->left;
	}
	// 左右反転したときの右側
	if(target->right - w_px < selected->left){
		selected->right = target->right - (selected->left - selected->right);
		selected->left = target->right;
	}
}

// 指定されたウインドウ範囲から出られなくします
void CorrectRect(RECT *selected, RECT *target)
{
	// 左側
	if(selected->left < target->left){
		selected->right = target->left + (selected->right - selected->left);
		selected->left = target->left;
	}
	// 上側
	if(selected->top < target->top){
		selected->bottom = target->top + (selected->bottom - selected->top);
		selected->top = target->top;
	}
	// 右側
	// ...

	// 右側(逆版)
	if(selected->left > target->right){
		int w = selected->left - selected->right;
		selected->left = target->right;
		selected->right = selected->left - w;
	}

	// 下側
	/*
	if(selected->bottom > target->bottom){
	int h = selected->bottom - selected->top;
	selected->bottom = target->bottom;
	selected->top = selected->bottom - h;
	}
	*/

	// 下側(逆版)
	if(selected->top > target->bottom){
		int h = selected->top - selected->bottom;
		selected->top = target->bottom;
		selected->bottom = selected->top - h;
	}
}

void LocaleErrorMsgBox(UINT msgId, ...)
{
	va_list arg;
	va_start(arg, msgId);

  TCHAR format[1024];
  ::LoadString(::GetModuleHandle(NULL), msgId, format, sizeof(format)); 

  TCHAR buffer[1024];
  ::_vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, arg);

  ::MessageBox(NULL, buffer, L"Error", MB_OK);
  va_end(arg);
}
