#include "YDXRender/YDXRender.h"
#include <unordered_map>
#include <iostream>

using namespace YXX;
using namespace std;
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")

#define REGISTER_RESOURCE(FullResourceType,Name,RealResource)\
{\
	auto& pool = DX11::ResourcePool::Get();\
	auto iter = pool.##FullResourceType##Map.find(Name);\
	if (iter == pool.##FullResourceType##Map.end())\
	{\
		pool.##FullResourceType##Map.insert(std::pair<std::string,YXX::ResourceHandle>(Name, pool.##FullResourceType##Resource.size() ));\
		pool.##FullResourceType##Resource.emplace_back(move(RealResource));\
	}\
	else\
		throw "An Object named "s + Name + "already exists";\
}
#define SEARCH_RESOURCE(FullResourceType,Name)\
{\
	auto& pool = DX11::ResourcePool::Get();\
	auto iter = pool.##FullResourceType##Map.find(Name);\
	if (iter != pool.##FullResourceType##Map.end())\
	{\
		return iter->second;\
	}\
	throw "not find the object named"s + Name;\
}

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DXDevice::Impl
{
public:
	bool Init(HWND hWnd);

	ComPtr<ID3D11Device> mpDevice;
	ComPtr<ID3D11DeviceContext>mpImmediateContext;
	ComPtr<IDXGISwapChain> mpSwapChain;

};

bool DXDevice::Impl::Init(HWND hWnd)
{
	// Setup swap chain
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
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, mpSwapChain.GetAddressOf(), mpDevice.GetAddressOf(), &featureLevel, mpImmediateContext.GetAddressOf()) != S_OK)
		return false;

	D3D11_FEATURE_DATA_THREADING data;
	HR(mpDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &data, sizeof(D3D11_FEATURE_DATA_THREADING)));

}

std::tuple<ID3D11Device*, IDXGISwapChain*, ID3D11DeviceContext*> YXX::DXDevice::Init(HWND hWnd)
{
	pImpl->Init(hWnd);
	return { pImpl->mpDevice.Get(),pImpl->mpSwapChain.Get(),pImpl->mpImmediateContext.Get() };
}
YXX::S<YXX::CommandBuffer> DXDevice::CreateCommandBuffer()
{
	CommandBuffer c;
	auto sCBuf = std::make_shared<CommandBuffer>(c);

	HR(pImpl->mpDevice->CreateDeferredContext(0, sCBuf->mpDeferredContext.GetAddressOf()));
	
	return sCBuf;
}
YXX::DXDevice::DXDevice()
	:pImpl(new Impl())
{
	 
}

DXDevice::~DXDevice()
{

}


void DXDevice::RegisterPSO(const std::string& name, const PipelineStateObjectDesc& desc)
{

}
void YXX::DXDevice::RegisterTexture(const std::string& name, Texture2DDesc desc)
{

}
void YXX::DXDevice::RegisterBackBuffer(const std::string &name)
{
	auto pTex = std::make_shared<DX11::ResourcePool::Texture>();

	pImpl->mpSwapChain->GetBuffer(0, IID_PPV_ARGS(pTex->tex.GetAddressOf()));
	pImpl->mpDevice->CreateRenderTargetView(pTex->tex.Get(), NULL, pTex->RTV.ReleaseAndGetAddressOf());
	
	REGISTER_RESOURCE(Texture, name, pTex);
	
}
void DXDevice::UpdateBackBuffer(const std::string& name,UINT width,UINT height)
{
	auto handle = DXDevice::Get().GetTextureHandle(name);
	auto tex = DX11::ResourcePool::Get().GetTextureByHandle(handle);

	tex->RTV.Reset();
	tex->tex->Release();
	
	HR(pImpl->mpSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));

	HR(pImpl->mpSwapChain->GetBuffer(0, IID_PPV_ARGS(tex->tex.GetAddressOf())));
	HR(pImpl->mpDevice->CreateRenderTargetView(tex->tex.Get(), NULL, tex->RTV.ReleaseAndGetAddressOf()));
}
YXX::ResourceHandle DXDevice::GetTextureHandle(const std::string& name)
{
	SEARCH_RESOURCE(Texture, name);
}
void YXX::DXDevice::ExcuteCommandBuffer(CommandBuffer* cmdBuf, bool clearList)
{
	auto& vecCmd = cmdBuf->mpd3dCommandLists;

	for_each(begin(vecCmd), end(vecCmd), [&](ComPtr<ID3D11CommandList>& cmd) {
		pImpl->mpImmediateContext->ExecuteCommandList(cmd.Get(), FALSE);
	});

	if (clearList)
		cmdBuf->ClearList();
}