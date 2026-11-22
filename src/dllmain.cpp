#include "pch.h"
#include "Hook.h"

BOOL WINAPI DllMain(const HINSTANCE hinstDLL, const DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		Hook::hDLLModule = hinstDLL;

		// The hooks are installed from a separate thread to stay out of the loader lock.
		if (const HANDLE hThread = CreateThread(nullptr, 0, Hook::HookThread, nullptr, 0, nullptr))
			CloseHandle(hThread);
	}

	// lpReserved is not null when the process is terminating, there is nothing to clean up in that case.
	if (fdwReason == DLL_PROCESS_DETACH && lpReserved == nullptr)
		Hook::UnHookDirectX();

	return TRUE;
}
