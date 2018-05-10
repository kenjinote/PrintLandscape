#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton;
	static HWND hEdit;
	static HFONT hFont;
	switch (msg)
	{
	case WM_CREATE:
		hFont = CreateFontW(-MulDiv(10, 96, 72), 0, 0, 0, FW_NORMAL, 0, 0, 0, SHIFTJIS_CHARSET, 0, 0, 0, 0, L"MS Shell Dlg");
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("印刷"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, 0);
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, 0);
		break;
	case WM_SIZE:
		MoveWindow(hButton, 10, 10, 256, 32, TRUE);
		MoveWindow(hEdit, 10, 50, LOWORD(lParam) - 20, HIWORD(lParam) - 60, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			TCHAR szPrinterName[256] = { 0 };
			DWORD dwPrinterNameSize = _countof(szPrinterName);
			if (!GetDefaultPrinter(szPrinterName, &dwPrinterNameSize))
			{
				return 0;
			}
			HANDLE hPrinter;
			if (!OpenPrinter(szPrinterName, &hPrinter, NULL))
			{
				return 0;
			}
			DWORD dwNeeded = DocumentProperties(hWnd, hPrinter, szPrinterName, NULL, NULL, 0);
			LPDEVMODE pDevMode = (LPDEVMODE)malloc(dwNeeded);
			DWORD dwRet = DocumentProperties(hWnd, hPrinter, szPrinterName, pDevMode, NULL, DM_OUT_BUFFER);
			if (dwRet != IDOK)
			{
				free(pDevMode);
				ClosePrinter(hPrinter);
				return 0;
			}
			if (pDevMode->dmFields & DM_ORIENTATION)
			{
				pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
			}
			dwRet = DocumentProperties(hWnd, hPrinter, szPrinterName, pDevMode, pDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
			ClosePrinter(hPrinter);
			if (dwRet != IDOK)
			{
				free(pDevMode);
				return 0;
			}
			HDC hDC = CreateDC(TEXT("WINSPOOL"), szPrinterName, NULL, pDevMode);
			if (hDC == NULL)
			{
				free(pDevMode);
				return 0;
			}
			DOCINFO di = { sizeof(DOCINFO) };
			di.lpszDocName = TEXT("Print Job");
			if (StartDoc(hDC, &di) <= 0)
			{
				DeleteDC(hDC);
				free(pDevMode);
				return 0;
			}
			if (StartPage(hDC) <= 0)
			{
				EndDoc(hDC);
				DeleteDC(hDC);
				free(pDevMode);
				return 0;
			}
			{
				DWORD dwTextSize = GetWindowTextLength(hEdit);
				LPTSTR lpszText = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR)*(dwTextSize + 1));
				GetWindowText(hEdit, lpszText, dwTextSize + 1);
				TextOut(hDC, 0, 0, lpszText, lstrlen(lpszText));
				GlobalFree(lpszText);
			}
			if (EndPage(hDC) <= 0)
			{
				EndDoc(hDC);
				DeleteDC(hDC);
				free(pDevMode);
				return 0;
			}
			if (EndDoc(hDC) <= 0)
			{
				DeleteDC(hDC);
				free(pDevMode);
				return 0;
			}
			DeleteDC(hDC);
			free(pDevMode);
		}
		break;
	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("用紙を横向きで印刷"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
