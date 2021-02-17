#include "YDXRender/DXFramework.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "YDXRender/YDXRender.h"

const char* MainBackBuf = "MainBackBuffer";
using namespace YXX;
using namespace std;
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static IDXGISwapChain* pSwapChain = nullptr;
LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before m_hMainWnd is valid.
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
	{
		if (pSwapChain && wParam != SIZE_MINIMIZED)
		{
			UpdateBackBuffer(DXDevice::Get(),MainBackBuf, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		}
	}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return true;
}

DXFramework::~DXFramework()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DXFramework::Run()
{
	auto hWnd = InitWindow();

	InitRender(hWnd);

	Init();
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else//äÖÈ¾
		{
			BeginUpdate();
			Update(ImGui::GetIO().DeltaTime);
			EndUpdate();
		}
	}
	
	Quit();
}

void DXFramework::BeginUpdate()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
}

void DXFramework::EndUpdate()
{
	auto rtHandle = DXDevice::Get().GetTextureHandle(MainBackBuf);

	pMainList->ClearRenderTarget(rtHandle);
	pMainList->SetRenderTargets(vector<ResourceHandle>(1, rtHandle));

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	pMainList->FinishRecord();

	DXDevice::Get().ExcuteCommandBuffer(pMainList.get());

	pSwapChain->Present(1, 0);
}	 

HWND YXX::DXFramework::InitWindow()
{
	ImGui_ImplWin32_EnableDpiAwareness();

	HWND hWnd;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MainWndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, TEXT("GL"), NULL };
	::RegisterClassEx(&wc);
	hWnd = ::CreateWindow(wc.lpszClassName, TEXT("GL"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
	// Show the window
	::ShowWindow(hWnd, SW_SHOWDEFAULT);
	::UpdateWindow(hWnd);
	return hWnd;
}

void YXX::DXFramework::InitRender(HWND hWnd)
{
	auto& device = DXDevice::Get();
	
	auto [pDevice, pswapChain] = device.Init(hWnd);
	pSwapChain = pswapChain;

	pMainList = device.CreateCommandBuffer();

	device.RegisterBackBuffer(MainBackBuf);

	#pragma region IMGUI_SETUP
{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;
		//io.ConfigViewportsNoDefaultParent = true;
		//io.ConfigDockingAlwaysTabBar = true;
		//io.ConfigDockingTransparentPayload = true;
#if 1
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI
#endif

	// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(pDevice, pMainList->mpDeferredContext.Get());

	}
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#pragma endregion IMGUI_SETUP


}
