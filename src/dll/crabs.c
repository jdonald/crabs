#include <windows.h>

__declspec(dllexport) LRESULT CALLBACK
CallWndProcRet(int nCode, WPARAM wParam, LPARAM lParam);

HINSTANCE hInstance;
HHOOK hHook = NULL;

BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    hInstance = hModule;
	return TRUE;
}

__declspec(dllexport)
void StartMoveHook()
{
	if (hHook == NULL)
		hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndProcRet, hInstance, 0);
}

__declspec(dllexport)
void StopMoveHook()
{
	if (hHook != NULL)
	{
		UnhookWindowsHookEx(hHook);
		hHook = NULL;
	}
}

__declspec(dllexport) LRESULT CALLBACK
CallWndProcRet(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPRETSTRUCT* pcwpret;
	pcwpret = (CWPRETSTRUCT*) lParam;
	if (pcwpret->message == WM_MOVE || pcwpret->message == WM_MOVING) {
		HWND hWndParent;
		if ((hWndParent = GetParent(pcwpret->hwnd)) != NULL)
			InvalidateRect(hWndParent, NULL, FALSE);
		InvalidateRect(pcwpret->hwnd, NULL, FALSE);
	}
	
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
