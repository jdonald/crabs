#include <windows.h>
#include "resource.h"
#include <stdlib.h>
#include <time.h>

#define MYWM_NOTIFYICON (WM_APP+100)
#define MYWM_TIMER (WM_APP+101)

typedef void (*pfuncvoid)();

typedef struct {
	int iDirection;
	POINT pt;
} crab_t;

const int giCrabs = 7;

crab_t crabs[7];
HICON  ghIcon[8];
HINSTANCE ghInstance;
HMODULE ghModule;
BOOL gfMovehookOn = FALSE;

LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
						 LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	HWND hWnd;
	MSG msg;
	int i;
	time_t timeSeed;
	NOTIFYICONDATA icodata;
	pfuncvoid pMoveHook;
	
	wc.lpszClassName = "CrabsClass";
	wc.lpfnWndProc = MainWndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONUPRIGHT));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
	wc.lpszMenuName = "CrabsMenu";
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClass(&wc);
	ghInstance = hInstance;

	hWnd = CreateWindow("CrabsClass",
		"Crabs",
		WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	
	icodata.cbSize = sizeof(icodata);
	icodata.hWnd = hWnd;
	icodata.uID = 0;
	icodata.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	icodata.uCallbackMessage = MYWM_NOTIFYICON;
	icodata.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	strcpy(icodata.szTip, "Crabs!");
	Shell_NotifyIcon(NIM_ADD, &icodata);

	SetTimer(hWnd, MYWM_TIMER, 50, NULL);

	time(&timeSeed);
	srand(timeSeed);
	ghIcon[0] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONUP));
	ghIcon[1] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONUPRIGHT));
	ghIcon[2] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONRIGHT));
	ghIcon[3] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONDOWNRIGHT));
	ghIcon[4] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONDOWN));
	ghIcon[5] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONDOWNLEFT));
	ghIcon[6] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONLEFT));
	ghIcon[7] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONUPLEFT));

	for (i = 0; i < giCrabs; i++) {
		crabs[i].pt.x = 400;
		crabs[i].pt.y = 400;
		crabs[i].iDirection = 0;
	}

	ghModule = LoadLibrary("crabs.dll");
	if (ghModule != NULL) { /* if no crabs.dll, function anyway */
		pMoveHook = (pfuncvoid) GetProcAddress(ghModule,
		  "StartMoveHook");
		if (pMoveHook != NULL) {
			(*pMoveHook)();
			gfMovehookOn = TRUE;
		}
		else
			ghModule = NULL;
	}
	
	while (GetMessage(&msg, NULL, 0, 0 )) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (ghModule != NULL) {
		pMoveHook = (pfuncvoid) GetProcAddress(ghModule, "StopMoveHook");
		if (pMoveHook != NULL)
			(*pMoveHook)();
		FreeLibrary(ghModule);
	}

	Shell_NotifyIcon(NIM_DELETE, &icodata);
	InvalidateRect(NULL, NULL, FALSE);

	return msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	int i;
	RECT rc;
	switch(msg) {

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_POPUP_CLOSE:
					DestroyWindow(hWnd);
					return TRUE;
				case ID_POPUP_MOVEHOOK:
					return TRUE;
				case ID_POPUP_ABOUT:
					DialogBox(ghInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutDlgProc);
					return TRUE;
				default:
					return FALSE;
			}

		case WM_TIMER:
			hDC = CreateDC("DISPLAY", NULL, "DISPLAY", NULL);
			GetClipBox(hDC, &rc);
			for (i = 0; i < giCrabs; i++) {
				if (rand() % 10 == 0) {
					crabs[i].iDirection = (crabs[i].iDirection +
					  (rand() % 3 - 1)) % 8;
					if (crabs[i].iDirection < 0)
						crabs[i].iDirection += 8;
				}
				switch(crabs[i].iDirection) {
					case 0:
						crabs[i].pt.y -= 3;
						break;
					case 1:
						crabs[i].pt.x += 2;
						crabs[i].pt.y -= 2;
						break;
					case 2:
						crabs[i].pt.x += 3;
						break;
					case 3:
						crabs[i].pt.x += 2;
						crabs[i].pt.y += 2;
						break;
					case 4:
						crabs[i].pt.y += 3;
						break;
					case 5:
						crabs[i].pt.x -= 2;
						crabs[i].pt.y += 2;
						break;
					case 6:
						crabs[i].pt.x -= 3;
						break;
					case 7:
					default:
						crabs[i].pt.x -= 2;
						crabs[i].pt.y -= 2;
						break;
				}
				/* assume top left is (0, 0) */
				if (crabs[i].pt.x < -32)
					crabs[i].pt.x += rc.right + 32;
				else if (crabs[i].pt.x > rc.right)
					crabs[i].pt.x -= (rc.right + 32);
				if (crabs[i].pt.y < -32)
					crabs[i].pt.y  += rc.bottom + 32;
				else if (crabs[i].pt.y > rc.bottom)
					crabs[i].pt.y -= (rc.bottom + 32);
				DrawIcon(hDC, crabs[i].pt.x, crabs[i].pt.y,
				  ghIcon[crabs[i].iDirection]);
			}
			DeleteDC(hDC);
			return TRUE;

		case MYWM_NOTIFYICON:
			switch (lParam) {

				case WM_RBUTTONUP: {
					HMENU hMenu;
					POINT pt;
					GetCursorPos(&pt);
					hMenu = LoadMenu(ghInstance, MAKEINTRESOURCE(IDM_MENU));
					hMenu = GetSubMenu(hMenu, 0);
					EnableMenuItem(hMenu, ID_POPUP_MOVEHOOK, MF_BYCOMMAND | ((ghModule != NULL) ? (MF_ENABLED) : (MF_DISABLED | MF_GRAYED)));
					CheckMenuItem (hMenu, ID_POPUP_MOVEHOOK, MF_BYCOMMAND | (gfMovehookOn ? MF_CHECKED : MF_UNCHECKED));
					
					SetForegroundWindow(hWnd); // workaround for Windows 95 bug
					switch (TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_RIGHTALIGN, pt.x, pt.y, 0, hWnd, NULL)) {
						case ID_POPUP_CLOSE:
							DestroyWindow(hWnd);
							return TRUE;
						case ID_POPUP_MOVEHOOK: {
							pfuncvoid pMoveHook;
							if (ghModule != NULL) {
								if (gfMovehookOn)
									pMoveHook = (pfuncvoid) GetProcAddress(ghModule, "StopMoveHook");
								else
									pMoveHook = (pfuncvoid) GetProcAddress(ghModule, "StartMoveHook");
								if (pMoveHook == NULL) {
									ghModule = NULL;
									gfMovehookOn = FALSE;
								}
								else {
									(*pMoveHook)();
									gfMovehookOn = !gfMovehookOn;
								}
							}
							else
								gfMovehookOn = FALSE;
							if (pMoveHook != NULL)
								(*pMoveHook)();
							return TRUE;
						
						}
						case ID_POPUP_ABOUT:
							DialogBox(ghInstance, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutDlgProc);
							return TRUE;
					}
					PostMessage(hWnd, 0, 0, 0); // workaround for Windows 95 bug
					DestroyMenu(hMenu);
					break;
				}
			}
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return FALSE;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch(wParam) {
				case IDOK:
					EndDialog(hDlg, TRUE);
					return TRUE;
			}
			break;
	}
	return FALSE;
}
