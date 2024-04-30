#include <Windows.h>
#include <stdio.h>
#include<TlHelp32.h>
DWORD FindUIThread(DWORD pid) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;
	THREADENTRY32 threadET;
	threadET.dwSize = sizeof(threadET);
	Thread32First(hSnapshot, &threadET);
	DWORD tid = 0; 
	while (Thread32Next(hSnapshot, &threadET)) {
		if (threadET.th32OwnerProcessID == pid)
		{
			GUITHREADINFO info{ sizeof(info) };
			if (GetGUIThreadInfo(threadET.th32ThreadID, &info)) 
				tid = threadET.th32ThreadID;
			break; 
			
		}
	}
	CloseHandle(hSnapshot);
	return tid;
}
void WINAPI SetNotifyTid(DWORD tid);
int main(int argc, const char* argv[]) {
	if (argc < 2)
	{
		printf("Usage: injektor <pid>\n");
		return 1;
	}
	int pid  = atoi(argv[1]); 
	DWORD tid = FindUIThread(pid);
	if (tid == 0) {
		printf("No GUI thread found\n.");
		return 1;
	}
		
	auto hDll = LoadLibrary(L"Injected.dll");
	auto hookFunc = (HOOKPROC)GetProcAddress(hDll, "HookFunc");
	if (!hookFunc) {
		printf("Function not found\n");
		return 1; 
	}
	auto pSetNotifyTid = (decltype(SetNotifyTid)*)GetProcAddress(hDll, "SetNotifyTid");
	if (pSetNotifyTid)
		pSetNotifyTid(GetCurrentThreadId());
	
	auto hHook = SetWindowsHookEx(WH_GETMESSAGE,hookFunc ,hDll , tid);
	if (!hHook) {
		printf("Error in SetWindowsHookEx\n");
		return 1;
	}
	PostThreadMessage(tid, WM_NULL, 0, 0);
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (msg.message == WM_APP) {
			printf("&c",(int)msg.wParam);
			if (msg.wParam == 13)
				printf("\n");
		}
	}
	UnhookWindowsHookEx(hHook);
	FreeLibrary(hDll);
	return 0;



	

}
