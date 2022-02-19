// VertexUI_Demo.cpp : 定义应用程序的入口点。
//
#include "AA.h"
#include <Urlmon.h>
#include <TlHelp32.h>
//#include <Wininet.h>

#include "framework.h"
#include "VertexUI/VertexUI.min.h"
#include "Resource.h"

#pragma comment(lib,"Urlmon.lib")
//#pragma comment(lib, "Wininet.lib")
#pragma warning(disable:4996)
#pragma comment(lib,"winmm.lib") 

#define MAX_LOADSTRING			100
#define FileMapping_NAME		"LightFrameInstCheck"
#define WATCHDOG_TIMEOUT		1000*10
#define USER_CANCEL				if(UserCancel){\
									isUpdateFailed=true;\
									strcpy(CurrentTask,"用户取消");\
									SwitchPanel(L"Panel2");\
									return 1;\
								}

// 全局变量:
HINSTANCE hInst;								// 当前实例
WCHAR szTitle[MAX_LOADSTRING];				  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

LPVOID lpdata = NULL;
LPCWSTR lpVersion, lpFileName;
char Version[32], NewVer[32];
char CurrentTask[32];
int TaskProgress = 0;
bool isUpdateFailed = false, isUpdateSuccess = false;
bool isNewInstall = false;
bool UserCancel = false;
HANDLE hUpdateThread, hWatchDog;
LPCWSTR MirrorURL = L"https://res.iyoroy.top/lightframe/release";

enum USER_MESSAGES {
	
};

using namespace std;

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR	lpCmdLine,
	_In_ int	   nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。
	isNewInstall = (__argc == 1);
	lpFileName = L"LightFrame.exe";
	wchar_t* qwq[20];
	for (int i = 1; i < __argc; i++) {
		if (wcsstr(__wargv[i], L"--CurrentVer")) 
			lpVersion = __wargv[++i];
		if (wcsstr(__wargv[i], L"--FileName")) 
			lpFileName = __wargv[++i];
	}

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_LIGHTFRAMEINSTALLER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LIGHTFRAMEINSTALLER));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LIGHTFRAMEINSTALLER));
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//		在此函数中，我们在全局变量中保存实例句柄并
//		创建和显示主程序窗口。
//
void CenterWindow(HWND hWnd)
{
	int scrWidth, scrHeight;
	RECT rect;
	//获得屏幕尺寸
	scrWidth = GetSystemMetrics(SM_CXSCREEN);
	scrHeight = GetSystemMetrics(SM_CYSCREEN);
	//取得窗口尺寸
	GetWindowRect(hWnd, &rect);
	//重新设置rect里的值  
	long width = rect.right - rect.left;
	long height = rect.bottom - rect.top;
	rect.left = (scrWidth - width) / 2;
	rect.top = (scrHeight - height) / 2;

	//移动窗口到指定的位置
	SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, width, height, SWP_NOSIZE | SWP_NOZORDER);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 600, 320, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}
	LONG_PTR Style = ::GetWindowLongPtr(hWnd, GWL_STYLE);
	Style = Style & ~WS_CAPTION & ~WS_SYSMENU & ~WS_SIZEBOX;
	::SetWindowLongPtr(hWnd, GWL_STYLE, Style);
	CenterWindow(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//


//int GetMemoryVer() //旧版主程序通信
//{
//	//打开一个指定的文件映射对象，获得共享内存对象的句柄
//	HANDLE hmapfile = OpenFileMappingA(FILE_MAP_READ, FALSE, FileMapping_NAME);
//	if (hmapfile == NULL) {
//		sprintf_s(Version, "%s", "ERR - 主程序传入失败");
//	}
//	else
//	{
//		LPVOID lpbase = MapViewOfFile(hmapfile, FILE_MAP_READ, 0, 0, 0);
//		if (lpbase == NULL)
//		{
//			MessageBox(0, L"memory Error", L"Fatal", 0);
//		}
//		else
//		{
//			sprintf_s(Version, "%s", lpbase);
//		}
//		UnmapViewOfFile(lpbase);
//		CloseHandle(hmapfile);
//
//	}
//	return 0;
//}

VertexUIInit;

RUNFUN GoPage2()
{
	SwitchPanel(L"Panel2");
	return 0;
}
RUNFUN GoPage1()
{
	SwitchPanel(L"Init");
	return 0;
}
RUNFUN MoveWin()
{
	HWND h = GhWnd;
	SendMessage(h, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	return -1;
}
void DoClean() {
	DeleteFile(L"newVer");
#ifdef _DEBUG
	HWND hWndLF = FindWindow(L"LIGHTFRAME", L"LightFrame");
	::PostMessage(hWndLF, 0xff3, 0, 1);
	DeleteFile(lpFileName);
#endif
}
DWORD WINAPI WatchDog(LPVOID lpParam) {//防止UpdateThread卡死
	int TaskHistory;
	do {
		if (isUpdateFailed || isUpdateSuccess)return 0;
		TaskHistory = TaskProgress;
		Sleep(WATCHDOG_TIMEOUT);
	} while (TaskHistory != TaskProgress);
	isUpdateFailed = true;
	TerminateThread(hUpdateThread, -1);
	strcpy(CurrentTask, (UserCancel ? "用户取消" : "升级失败：WATCHDOG TIMEOUT"));
	GoPage2();
	return -1;
}
DWORD WINAPI UpdateThread(LPVOID lpParam) {
	isUpdateFailed = false;
	isUpdateSuccess = false;
	UserCancel = false;
	strcpy(CurrentTask, (isNewInstall ? "下载程序本体..." : "下载更新文件..."));
	TaskProgress = 1;
	TCHAR bufferURL[128];
	HRESULT hrDl;
	_stprintf(bufferURL, L"%s/LightFrame.exe?skq=%d", MirrorURL, (int)GetTickCount64());
	hrDl = URLDownloadToFile(NULL, bufferURL, (isNewInstall ? L"LightFrame.exe" : L"LightFrame.ex_"), 0, NULL);
	if (hrDl != S_OK) {
		strcpy(CurrentTask, "错误：网络问题，无法下载");
		isUpdateFailed = true;
		GoPage2();
		return -1;
	}
	USER_CANCEL;

	if (!isNewInstall) {
		strcpy(CurrentTask, "发送退出信息...");
		TaskProgress = 2;
		GoPage2();
		HWND hWndLF = FindWindow(L"LIGHTFRAME", L"LightFrame");
		PROCESSENTRY32  pe32;
		HANDLE hSnaphot;
		HANDLE hApp;
		DWORD dProcess = 0;
		hSnaphot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //获取进程快照
		Process32First(hSnaphot, &pe32); //指向第一个进程
		do {
			if (lstrcmpi(pe32.szExeFile, lpFileName) == 0) {
				dProcess = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnaphot, &pe32)); // 不断循环直到取不到进程
		hApp = OpenProcess(
			PROCESS_VM_OPERATION | SYNCHRONIZE, FALSE, dProcess);
		::PostMessage(hWndLF, 0xff3, 0, 1);
		Sleep(500);
		USER_CANCEL;

		strcpy(CurrentTask, "等待LightFrame主程序退出...");
		TaskProgress = 3;
		GoPage2();
		WaitForSingleObject(hApp, INFINITE);
		Sleep(500);
		USER_CANCEL;

		strcpy(CurrentTask, "覆盖更新...");
		TaskProgress = 4;
		GoPage2();
		DeleteFile(lpFileName);
		MoveFile(L"LightFrame.ex_", lpFileName);
		Sleep(500);
		USER_CANCEL;
	}

	strcpy(CurrentTask, "删除缓存文件...");
	TaskProgress = (isNewInstall ? 2 : 5);
	GoPage2();
	DoClean();
	Sleep(500);
	USER_CANCEL;

	strcpy(CurrentTask, "启动LightFrame...");
	TaskProgress = (isNewInstall ? 3 : 6);
	GoPage2();
	ShellExecute(NULL, L"open", lpFileName, NULL, NULL, SW_SHOWNORMAL);
	Sleep(500);

	strcpy(CurrentTask, "完成！"); 
	TaskProgress = (isNewInstall ? 3 : 6);
	isUpdateSuccess = true; 
	GoPage2();

	return 0;
}
int hState = 0;
const wchar_t* BtnStr = L"下载新版本";
int newver, cver;
int LightFrameAreaEvent(HWND hWnd, LPARAM lParam)
{

	int val = 0;

	int msg = 0;

	UINT pindex = 0;
	RECT winrc;
	GetClientRect(hWnd, &winrc);

	if (PanelID == L"Init" || PanelID == L"Panel1")
	{
		RECT rc = {};
		if ((GetAreaPtInfo(hWnd, ((winrc.right - winrc.left) / 2 - 60), winrc.bottom - 80, 120, 40, rc, lParam)) == 1)
		{
			if (ClickMsg == 1)
			{
				ClickMsg = 0;
				if (cver <= newver)
				{
					GoPage2();
					hUpdateThread = CreateThread(NULL, 0, UpdateThread, NULL, 0, 0);
					hWatchDog = CreateThread(NULL, 0, WatchDog, NULL, 0, 0);
				}
				else
				{
					exit(0);
				}
			}
			if (hState == 0)
			{
				hState = 1;
			}
			return 0;
		}
		if ((GetAreaPtInfo(hWnd, ((winrc.right - winrc.left) - 40), 0, 40, 40, rc, lParam)) == 1)
		{
			if (ClickMsg == 1)
			{
				ClickMsg = 0;
				DoClean();
				DestroyWindow(hWnd);
				PostQuitMessage(0);
			}
			if (hState == 0)
			{
				hState = 1;
			}
			return 0;
		}
		else
		{
			if (hState == 1)
			{
				hState = 0;
				InvalidateRect(hWnd, &winrc, 0);
			}
			return 0;
		}
	}
	if (PanelID == L"Panel2")
	{
		RECT rc = {};
		if (GetAreaPtInfo(hWnd, winrc.right - 170, winrc.bottom - 80, 150, 40, rc, lParam) == 1)
		{
			if (ClickMsg == 1)
			{
				if (isUpdateFailed || isUpdateSuccess) {
					ClickMsg = 0;
					PostQuitMessage(0);
				}
				else {
					ClickMsg = 0;
					UserCancel = true;
					strcpy(CurrentTask, "等待线程退出...");
					SwitchPanel(L"Panel2");
				}
			}
			if (hState == 0)
			{
				HDC hdc = GetDC(hWnd);
				//CreateRect(hWnd, hdc, x, rc.bottom - 2, rc.right - rc.left, 2, VERTEXUICOLOR_GREENDEEPSEA);
				CreateRect(hWnd, hdc, rc.left, rc.bottom - 5, rc.right - rc.left, 5, VERTEXUICOLOR_GREENDEEPSEA);
				DeleteObject(hdc);
				ReleaseDC(hWnd, hdc);
				DeleteDC(hdc);
				hState = 1;
			}
			return 0;
		}
		//-------------------------
			//-------------------------
		else
		{
			if (hState == 1)
			{
				hState = 0;
				InvalidateRect(hWnd, &winrc, 0);
				return 0;
			}
		}
	}
	if (PanelID != 0)
	{
		RECT rc = {};

		msg = 0;
	}


	return 0;
}
void TextPreDrawA(HDC hdc, int x, int y, int sizex, int sizey, const char* txt, COLORREF cl)
{
	RECT rc;
	RectTypeConvert(rc, x, y, sizex, sizey);
	LOGFONT lf;
	HFONT hFont = 0;
	SetTextColor(hdc, cl);
	SetBkMode(hdc, TRANSPARENT);
	if (hFont == 0)
	{
		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = -20;
		wcscpy_s(lf.lfFaceName, L"Segoe UI");
		hFont = CreateFontIndirect(&lf);  // create the font
	}
	HFONT old = (HFONT)SelectObject(hdc, hFont);
	DrawTextA(hdc, txt, strlen(txt), &rc, DT_SINGLELINE | DT_VCENTER);
	DeleteObject(hFont);
	SelectObject(hdc, old);
}

void MainWindow(HWND h, HDC hdc, int scale)
{
	RECT rc;
	GetClientRect(h, &rc);
	CreateRect(h, hdc, rc.left, rc.top, (rc.right - rc.left) * scale, (rc.bottom - rc.top) * scale, RGB(42, 47, 56));
	CreateRoundButtonEx(hdc, ((rc.right - rc.left) / 2 - 60) * scale, (rc.bottom - 80) * scale, 120 * scale, 40 * scale, 40 * scale, BtnStr, 18 * scale, VERTEXUICOLOR_GREENSEA);

}

void Panel1(HWND hWnd, HDC hdc)
{

	int scale = 1;
	RECT rc;
	GetClientRect(hWnd, &rc);

	newver = atoi(NewVer);
	cver = atoi(Version);
	if (cver <= newver)
	{
		CreateAA(hWnd, hdc, rc.left, rc.top, rc.right - rc.left + 6, rc.bottom - rc.top + 6, MainWindow);
		if (!isNewInstall) {
			TextPreDrawEx(hdc, 40, 60, 220, 24, L"当前版本(Total Build) :", 20, 0, VERTEXUICOLOR_WHITE);
			TextPreDrawA(hdc, 240, 60, 200, 24, Version, VERTEXUICOLOR_WHITE);
		}
		TextPreDrawEx(hdc, 40, 120, 220, 24, L"最新版本(Total Build) :", 20, 0, VERTEXUICOLOR_WHITE);
		TextPreDrawA(hdc, 240, 120, 200, 24, NewVer, VERTEXUICOLOR_WHITE);

	}
	else{
		BtnStr = L"关闭";
		CreateAA(hWnd, hdc, rc.left, rc.top, rc.right - rc.left + 6, rc.bottom - rc.top + 6, MainWindow);
		TextPreDrawEx(hdc, 0,0, rc.right-rc.left, rc.bottom-rc.top, L"当前已经是最新版本或更新的版本.", 20, 1, VERTEXUICOLOR_WHITE);
	}
	TextPreDrawA(hdc, 10, 170, 200, 190, "Installer:v0.1.1.10-β", RGB(100, 100, 100));
	CreateRect(hWnd, hdc, 0, 0, rc.right, 40, VERTEXUICOLOR_GREENDEEPSEA);
	PanelDrawCloseBtn(hWnd, hdc, rc.right - 40, 0, 40, 40, 12, RGB(244, 244, 244));
	PanelDrawOutFrame(hWnd, hdc, VERTEXUICOLOR_DARKENX);
}
void Panel2(HWND hWnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	CreateFillArea(hWnd, hdc, VERTEXUICOLOR_DARKNIGHT);
	TextPreDrawEx(hdc, 40, 60, 220, 24, L"当前任务:", 20, 0, VERTEXUICOLOR_WHITE);
	TextPreDrawA(hdc, 180, 60, 300, 24, CurrentTask, VERTEXUICOLOR_WHITE);
	char TargetProg[4];
	sprintf_s(TargetProg, (isNewInstall ? "%d/3" : "%d/6"), TaskProgress);
	TextPreDrawEx(hdc, 40, 120, 220, 24, L"进度:", 20, 0, VERTEXUICOLOR_WHITE);
	TextPreDrawA(hdc, 180, 120, 300, 24, TargetProg, VERTEXUICOLOR_WHITE);
	CreateSimpleButton(hWnd, hdc, rc.right - 170, rc.bottom - 80, 150, 40, (isUpdateFailed || isUpdateSuccess ? L"完成" : L"取消"));
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	RECT Winrc;
	GetWindowRect(hWnd, &Winrc);
	switch (message)
	{
	case WM_CREATE:
	{
		GhWnd = hWnd;
		//GetMemoryVer();
		TCHAR bufferURL[64];
		_stprintf(bufferURL, L"%s/buildver?skq=%d", MirrorURL, (int)GetTickCount64());
		HRESULT ret = URLDownloadToFile(NULL, bufferURL, L"newVer", 0, NULL);
		if (ret != S_OK) {
			MessageBox(NULL, L"Fatal Error:无法获取版本更新信息！", L"LightFrameInstaller", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
		}

		HANDLE hFile;
		DWORD dwReads;
		hFile = CreateFile(L"newVer", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		ReadFile(hFile, NewVer, 32, &dwReads, NULL);
		CloseHandle(hFile);
		NewVer[dwReads] = '\0';


		DWORD  num = WideCharToMultiByte(CP_ACP, 0, lpVersion, -1, NULL, 0, NULL, 0);
		char* pStrVer = (char*)calloc(num, sizeof(char));
		if (pStrVer != NULL) {
			memset(pStrVer, 0, num * sizeof(char));
			WideCharToMultiByte(CP_ACP, 0, lpVersion, -1, pStrVer, num, NULL, 0);
		}
		strcpy(Version, pStrVer);
		free(pStrVer);

		break;
	}
	case WM_PAINT:
	{
		int scrWidth, scrHeight;
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		if (Winrc.top + (rc.bottom - rc.top) >= scrHeight || Winrc.left + (rc.right - rc.left) >= scrWidth || Winrc.left <= 0 || Winrc.top <= 0)
		{
			if (PanelID != PrevPanelID)
			{
				InvalidateRect(hWnd, &rc, 0);
				SendMessage(hWnd, WM_MOUSEMOVE, wParam, lParam);
			}
			if (hState == 0)
			{
				InvalidateRect(hWnd, &rc, 0);

			}
			else
			{
				break;
			}
		}
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// 切换
		if (PanelID == L"Init")
		{
			CreatePanelByFlag(hWnd, hdc, Panel1);
		}
		if (PanelID == L"Panel2")
		{
			CreatePanelByFlag(hWnd, hdc, Panel2);
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_MOUSEMOVE:
	{
		LightFrameAreaEvent(hWnd, lParam);
		break;
	}
	case WM_ERASEBKGND:
	{
		break;
	}
	case WM_LBUTTONUP:
	{
		SendClickEvent(hWnd, wParam, lParam); //发送Click,mousemove检测ClickMsg
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int val = 0;
		int val2 = 0;
		ClickAreaPtInfo(hWnd, rc.right - 80, 0, 80, 40, lParam, val);
		ClickAreaPtInfo(hWnd, 168, 12, 35, 20, lParam, val2);
		if (val == 0)
		{
			if (val2 == 0)
			{
			   ClickArea(hWnd, 0, 0, rc.right - rc.left, 40, lParam, MoveWin);
			}
		}
		break;
	}
	case WM_MOVING:
	{
		hState = 1;
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case SIZE_MAXIMIZED:
		{
			InvalidateRect(hWnd, &rc, 0);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// “关于”框的消息处理程序。 
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
