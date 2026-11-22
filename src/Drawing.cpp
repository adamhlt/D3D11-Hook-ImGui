#include "pch.h"
#include "Drawing.h"
#include "Hook.h"

namespace Drawing
{
	bool bInit = false; // Status of the initialization of ImGui.
	bool bDisplay = true; // Status of the menu display.
	bool bSetPos = false; // Status to update ImGui window size / position.

	namespace
	{
		ImVec2 vWindowPos = { 0, 0 }; // Last ImGui window position.
		ImVec2 vWindowSize = { 0, 0 }; // Last ImGui window size.

		/**
			@brief : Function that init ImGui and retrieve D3D11 device and device context.
			@param pSwapChain : Pointer to the SwapChain from the Present hook.
		**/
		void InitImGui(IDXGISwapChain* pSwapChain)
		{
			Hook::HookWindow();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.IniFilename = nullptr;
			io.Fonts->AddFontDefault();

			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(Hook::window);

			pSwapChain->GetDevice(__uuidof(Hook::pDevice), reinterpret_cast<void**>(&Hook::pDevice));
			Hook::pDevice->GetImmediateContext(&Hook::pDeviceContext);

			ImGui_ImplDX11_Init(Hook::pDevice, Hook::pDeviceContext);

			bInit = true;
		}

		/**
			@brief : Function that draw the menu.
		**/
		void DrawMenu()
		{
			ImGui::Begin("Menu Window Title", &bDisplay);
			{
				ImGui::SetWindowSize({ 500, 300 }, ImGuiCond_Once);

				if (vWindowPos.x != 0.0f && vWindowPos.y != 0.0f && vWindowSize.x != 0.0f && vWindowSize.y != 0.0f && bSetPos)
				{
					ImGui::SetWindowPos(vWindowPos);
					ImGui::SetWindowSize(vWindowSize);
					bSetPos = false;
				}

				if (!bSetPos)
				{
					vWindowPos = ImGui::GetWindowPos();
					vWindowSize = ImGui::GetWindowSize();
				}

				ImGui::Text("Draw your menu here.");
			}
			ImGui::End();
		}
	}

	/**
		@brief : Hook function of Present, the function render ImGui menu.
				 (https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present)
	**/
	HRESULT APIENTRY hkPresent(IDXGISwapChain* pThis, const UINT SyncInterval, const UINT Flag)
	{
		if (!Hook::pSwapChain)
			Hook::pSwapChain = pThis;

		if (!bInit)
			InitImGui(pThis);

		if (GetAsyncKeyState(VK_INSERT) & 1)
			bDisplay = !bDisplay;

		if (GetAsyncKeyState(VK_END) & 1)
		{
			Hook::UnHookDirectX();
			Hook::Unload();
			return Hook::oPresent(pThis, SyncInterval, Flag);
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (bDisplay)
			DrawMenu();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return Hook::oPresent(pThis, SyncInterval, Flag);
	}
}
