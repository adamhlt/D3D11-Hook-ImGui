#pragma once

#include "pch.h"

namespace Drawing
{
	extern bool bDisplay;
	extern bool bInit;
	extern bool bSetPos;

	HRESULT APIENTRY hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flag);
}
