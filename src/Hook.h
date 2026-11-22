#pragma once

#include "pch.h"

using tPresent = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flag);

namespace Hook
{
	extern IDXGISwapChain* pSwapChain;
	extern ID3D11Device* pDevice;
	extern ID3D11DeviceContext* pDeviceContext;
	extern tPresent oPresent;
	extern HWND window;
	extern HMODULE hDLLModule;

	/**
		@brief : Entry point of the hook thread, hooks the DirectX functions.
	**/
	DWORD WINAPI HookThread(LPVOID lpParameter);

	void HookDirectX();
	void UnHookDirectX();
	void HookWindow();
	void UnHookWindow();

	/**
		@brief : Release ImGui and the D3D11 objects, the context is created again on the next Present.
	**/
	void ReleaseResources();

	/**
		@brief : Unload the DLL from the current process.
	**/
	void Unload();
}
