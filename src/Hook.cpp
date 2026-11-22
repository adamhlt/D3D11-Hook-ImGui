#include "pch.h"
#include "Hook.h"
#include "Drawing.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using tResizeBuffers = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
using tSetFullscreenState = HRESULT(APIENTRY*)(IDXGISwapChain* pThis, BOOL FullScreen, IDXGIOutput* pTarget);

namespace Hook
{
	IDXGISwapChain* pSwapChain = nullptr; // D3D11 Swap Chain of the game, retrieved from the Present hook.
	ID3D11Device* pDevice = nullptr; // D3D11 device.
	ID3D11DeviceContext* pDeviceContext = nullptr; // D3D11 device context.
	tPresent oPresent = nullptr; // Pointer of the original Present function.
	HWND window = nullptr; // Window of the current process.
	HMODULE hDLLModule = nullptr; // HMODULE of the DLL.

	namespace
	{
		constexpr size_t SWAP_CHAIN_VTABLE_SIZE = 40; // Number of functions in the IDXGISwapChain virtual table.
		constexpr size_t PRESENT_INDEX = 8;
		constexpr size_t SET_FULLSCREEN_STATE_INDEX = 10;
		constexpr size_t RESIZE_BUFFERS_INDEX = 13;

		bool bHooked = false; // Status of the DirectX hooks.
		void* d3d11SwapChain[SWAP_CHAIN_VTABLE_SIZE]; // Array of pointer of the DirectX functions.
		WNDPROC oWndProc = nullptr; // Pointer of the original window message handler.
		tResizeBuffers oResizeBuffers = nullptr; // Pointer of the original ResizeBuffers function.
		tSetFullscreenState oSetFullscreenState = nullptr; // Pointer of the original SetFullscreenState function.

		/**
			@brief : Thread routine that unloads the DLL, FreeLibraryAndExitThread is used because the thread code lives in the DLL.
		**/
		DWORD WINAPI UnloadThread(LPVOID lpParameter)
		{
			FreeLibraryAndExitThread(hDLLModule, 0);
		}

		/**
			@brief : Function that enumerate windows.
		**/
		BOOL CALLBACK EnumWind(const HWND handle, LPARAM lp)
		{
			DWORD procID;
			GetWindowThreadProcessId(handle, &procID);
			if (GetCurrentProcessId() != procID)
				return TRUE;

			window = handle;
			return FALSE;
		}

		/**
			@brief : Function that retrieve the window of the current process.
			@retval : window of the current process.
		**/
		HWND GetProcessWindow()
		{
			window = nullptr;
			EnumWindows(EnumWind, NULL);
			return window;
		}

		/**
			@brief : Function that get the virtual table of a D3D11 Swap Chain.
			@param  pTable : Array of functions pointer.
			@param  size   : Size of the array of pointers in bytes.
			@retval : True if the function succeed else False.
		**/
		bool GetD3D11SwapChain(void** pTable, const size_t size)
		{
			if (!pTable)
				return false;

			const HWND hWindow = GetProcessWindow();
			if (!hWindow)
				return false;

			IDXGISwapChain* pDummySwapChain = nullptr;
			ID3D11Device* pDummyDevice = nullptr;

			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 2;
			sd.BufferDesc.Width = 0;
			sd.BufferDesc.Height = 0;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWindow;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = (GetWindowLongPtr(hWindow, GWL_STYLE) & WS_POPUP) == 0;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice, nullptr, nullptr);
			if (res == DXGI_ERROR_UNSUPPORTED)
				res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice, nullptr, nullptr);
			if (res != S_OK)
				return false;

			memcpy(pTable, *reinterpret_cast<void***>(pDummySwapChain), size);

			pDummySwapChain->Release();
			pDummyDevice->Release();
			return true;
		}

		/**
			@brief : A callback function, which you define in your application, that processes messages sent to a window. (https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc)
			@param  hWnd : A handle to the window.
			@param  msg : The message.
			@param  wParam : Additional message information.
			@param  lParam : Additional message information.
			@retval : The return value is the result of the message processing, and depends on the message sent.
		**/
		LRESULT WINAPI WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
		{
			if (Drawing::bDisplay && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			{
				ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
				return true;
			}

			if (Drawing::bInit)
				ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

			if (msg == WM_CLOSE)
			{
				UnHookDirectX();
				TerminateProcess(GetCurrentProcess(), 0);
			}

			if (ImGui::GetIO().WantCaptureMouse)
				return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

			return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
		}

		/**
			@brief : Hook function of ResizeBuffers, the function release ImGui context and D3D11 objects when window resize.
					 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers)
		**/
		HRESULT APIENTRY hkResizeBuffers(IDXGISwapChain* pThis, const UINT BufferCount, const UINT Width, const UINT Height, const DXGI_FORMAT NewFormat, const UINT SwapChainFlags)
		{
			Drawing::bSetPos = true;
			ReleaseResources();

			return oResizeBuffers(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		}

		/**
			@brief : Hook function of SetFullscreenState, the function release ImGui context and D3D11 objects when window fullscreen state change.
					 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-setfullscreenstate)
		**/
		HRESULT APIENTRY hkSetFullscreenState(IDXGISwapChain* pThis, const BOOL FullScreen, IDXGIOutput* pTarget)
		{
			Drawing::bSetPos = true;
			ReleaseResources();

			return oSetFullscreenState(pThis, FullScreen, pTarget);
		}
	}

	/**
		@brief : Entry point of the hook thread.
	**/
	DWORD WINAPI HookThread(LPVOID lpParameter)
	{
		HookDirectX();
		return 0;
	}

	/**
		@brief : Function that hook the ResizeBuffers, SetFullscreenState and Present functions.
	**/
	void HookDirectX()
	{
		if (!GetD3D11SwapChain(d3d11SwapChain, sizeof(d3d11SwapChain)))
			return;

		oPresent = reinterpret_cast<tPresent>(d3d11SwapChain[PRESENT_INDEX]);
		oResizeBuffers = reinterpret_cast<tResizeBuffers>(d3d11SwapChain[RESIZE_BUFFERS_INDEX]);
		oSetFullscreenState = reinterpret_cast<tSetFullscreenState>(d3d11SwapChain[SET_FULLSCREEN_STATE_INDEX]);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<PVOID&>(oPresent), Drawing::hkPresent);
		DetourAttach(&reinterpret_cast<PVOID&>(oResizeBuffers), hkResizeBuffers);
		DetourAttach(&reinterpret_cast<PVOID&>(oSetFullscreenState), hkSetFullscreenState);
		DetourTransactionCommit();

		bHooked = true;
	}

	/**
		@brief : Function that unhook ResizeBuffers, SetFullscreenState and Present functions, and also disable the window message handler.
				 The function can be called several times, only the first call does the work.
	**/
	void UnHookDirectX()
	{
		if (!bHooked)
			return;

		bHooked = false;
		ReleaseResources();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&reinterpret_cast<PVOID&>(oPresent), Drawing::hkPresent);
		DetourDetach(&reinterpret_cast<PVOID&>(oResizeBuffers), hkResizeBuffers);
		DetourDetach(&reinterpret_cast<PVOID&>(oSetFullscreenState), hkSetFullscreenState);
		DetourTransactionCommit();
	}

	/**
		@brief : Function that setup the WndProc callback function.
	**/
	void HookWindow()
	{
		oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
	}

	/**
		@brief : Function that restore the original window message handler.
	**/
	void UnHookWindow()
	{
		if (!oWndProc)
			return;

		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
		oWndProc = nullptr;
	}

	void ReleaseResources()
	{
		UnHookWindow();

		if (Drawing::bInit)
		{
			Drawing::bInit = false;
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			pDeviceContext->Release();
			pDevice->Release();
		}

		pSwapChain = nullptr;
		pDevice = nullptr;
		pDeviceContext = nullptr;
	}

	void Unload()
	{
		if (const HANDLE hThread = CreateThread(nullptr, 0, UnloadThread, nullptr, 0, nullptr))
			CloseHandle(hThread);
	}
}
