#include "YDXRender/YDXRender.h"
#include <unordered_map>
#include <iostream>

#include <d3dcompiler.h>
using namespace YXX;
using namespace std;
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")

//////////////////////////////////////////////////////////////
/////  Compile shader
//////////////////////////////////////////////////////////////
// ------------------------------
// CreateShaderFromFile函数
// ------------------------------
// [In]csoFileNameInOut 编译好的着色器二进制文件(.cso)，若有指定则优先寻找该文件并读取
// [In]hlslFileName     着色器代码，若未找到着色器二进制文件则编译着色器代码
// [In]entryPoint       入口点(指定开始的函数)
// [In]shaderModel      着色器模型，格式为"*s_5_0"，*可以为c,d,g,h,p,v之一
// [Out]ppBlobOut       输出着色器二进制信息
HRESULT CreateShaderBlobFromFile(
	const WCHAR* csoFileNameInOut,
	const WCHAR* hlslFileName,
	LPCSTR entryPoint,
	LPCSTR shaderModel,
	ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	// 寻找是否有已经编译好的顶点着色器
	if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
	{
		return hr;
	}
	else
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
		// 但仍然允许着色器进行优化操作
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// 在Debug环境下禁用优化以避免出现一些不合理的情况
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* errorBlob = nullptr;
		hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
			dwShaderFlags, 0, ppBlobOut, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob != nullptr)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			SAFE_RELEASE(errorBlob);
			return hr;
		}

		// 若指定了输出文件名，则将着色器二进制信息输出
		if (csoFileNameInOut)
		{
			return D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, FALSE);
		}
	}

	return hr;
}

//copy from https://gist.github.com/mobius/b678970c61a93c81fffef1936734909f
HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	return hr;
}


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
	return pool.##FullResourceType##Map[Name];\
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

std::tuple<ID3D11Device*, IDXGISwapChain*> YXX::DXDevice::Init(HWND hWnd)
{
	pImpl->Init(hWnd);
	return { pImpl->mpDevice.Get(),pImpl->mpSwapChain.Get()};
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


ResourceHandle DXDevice::RegisterPSO(const std::string& name, const PipelineStateObjectDesc& desc)
{
	U<DX11::ResourcePool::PipelineStateObject> pso = std::make_unique<DX11::ResourcePool::PipelineStateObject>();

#pragma region CreateShaderAndLayout
	auto& shaderName = desc.ShaderName;

#ifdef _DEBUG
	if (shaderName.substr(shaderName.length() - 4, 4) != "hlsl")
		throw "not hlsl file";
#endif // _DEBUG
	auto str = shaderName.substr(0, shaderName.length() - 5);
	auto vstr = str + "VS.cso";
	auto pstr = str + "PS.cso";

	ComPtr<ID3DBlob> blob;
	// deal vertex shader
	{
		HR(CreateShaderBlobFromFile(StringToWString(vstr).c_str(), StringToWString(shaderName).c_str(), "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
		
		HR(pImpl->mpDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, pso->mpVS.GetAddressOf()));

		HR(CreateInputLayoutDescFromVertexShaderSignature(blob.Get(),pImpl->mpDevice.Get(),pso->mpInputLayout.GetAddressOf()));
		
		// shader reflection
		ComPtr<ID3D11ShaderReflection> pShaderReflection;
		HR(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), __uuidof(ID3D11ShaderReflection),
			reinterpret_cast<void**>(pShaderReflection.GetAddressOf())));

		pso->AppendLayout(pImpl->mpDevice.Get(), pShaderReflection.Get());
	}
	{
		CreateShaderBlobFromFile(StringToWString(pstr).c_str(), StringToWString(shaderName).c_str(), "PS", "ps_5_0", blob.ReleaseAndGetAddressOf());

		// create native shader
		HR(pImpl->mpDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, pso->mpPS.GetAddressOf()));

		// shader reflection
		ComPtr<ID3D11ShaderReflection> pShaderReflection;
		HR(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), __uuidof(ID3D11ShaderReflection),
			reinterpret_cast<void**>(pShaderReflection.GetAddressOf())));

		pso->AppendLayout(pImpl->mpDevice.Get(), pShaderReflection.Get());
	}
#pragma endregion

#pragma region State

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;

	switch (desc.RasterType)
	{
	case RS::DEFAULT:
		break;
	case RS::CULL_BACK:
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		break;
	case RS::CULL_NONE:
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		break;
	case RS::CULL_FRONT:
		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		break;
	case RS::FILL_WIREFRAME:
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		break;
	default:
		break;
	}
	pImpl->mpDevice->CreateRasterizerState(&rasterizerDesc, pso->mpRS.GetAddressOf());

#pragma endregion 

#pragma region Viewport
	pso->mVP = { 0 };
	pso->mVP.Width = desc.ViewportWidth;
	pso->mVP.Height = desc.ViewportHeight;
	pso->mVP.MinDepth = 0;
	pso->mVP.MaxDepth = 1;
#pragma endregion 

	REGISTER_RESOURCE(PipelineStateObject, name, pso); 
}
ResourceHandle YXX::DXDevice::RegisterParaHeap(const std::string& name, ResourceHandle pso)
{
	auto& layout = DX11::ResourcePool::Get().PipelineStateObjectResource[pso]->mpLayout;

	U<DX11::ResourcePool::ParameterHeap> pHeap = std::make_unique<DX11::ResourcePool::ParameterHeap>();
	pHeap->SRVLayout = layout->SRVLayout;
	
	for (auto& iter : layout->CBufLayout)
	{
		auto& ci = iter.first;
		auto& cbs = iter.second;
		pHeap->CBufferHeap[ci.startSlot] = DX11::ResourcePool::ParameterHeap::CBufferData(ci.cbufferName, ci.startSlot, ci.byteWidth, nullptr);
		pHeap->CBufferHeap[ci.startSlot].CreateBuffer(pImpl->mpDevice.Get());

		for (auto& var : cbs)
		{
			pHeap->ConstantVariables[var.first] =
				std::make_shared<DX11::ResourcePool::ParameterHeap::ConstantBufferVariable>(
					var.second.StartOffset,
					var.second.Size,
					&pHeap->CBufferHeap[ci.startSlot]);
		}
	}
	
	REGISTER_RESOURCE(ParameterHeap, name, pHeap);
}

ResourceHandle YXX::DXDevice::RegisterTexture(const std::string& name, Texture2DDesc desc)
{
	
	auto pTex = std::make_shared<DX11::ResourcePool::Texture>(desc.Width, desc.Height, desc.Format,desc.flag,pImpl->mpDevice.Get());
	REGISTER_RESOURCE(Texture, name, pTex);
}

ResourceHandle YXX::DXDevice::RegisterBackBuffer(const std::string &name)
{
	auto pTex = std::make_shared<DX11::ResourcePool::Texture>();

	pImpl->mpSwapChain->GetBuffer(0, IID_PPV_ARGS(pTex->tex.GetAddressOf()));
	pImpl->mpDevice->CreateRenderTargetView(pTex->tex.Get(), NULL, pTex->RTV.ReleaseAndGetAddressOf());
	
	REGISTER_RESOURCE(Texture, name, pTex);
	
}

 void YXX::UpdateBackBuffer(DXDevice& device, const std::string& name,UINT width,UINT height)
{
	auto handle = device.GetTextureHandle(name);
	auto tex = DX11::ResourcePool::Get().GetTextureByHandle(handle);

	tex->RTV.Reset();
	tex->tex->Release();
	
	HR(device.pImpl->mpSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));

	HR(device.pImpl->mpSwapChain->GetBuffer(0, IID_PPV_ARGS(tex->tex.GetAddressOf())));
	HR(device.pImpl->mpDevice->CreateRenderTargetView(tex->tex.Get(), NULL, tex->RTV.ReleaseAndGetAddressOf()));
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
#include <YMesh/MeshGenerator.h>
ResourceHandle YXX::DXDevice::RegisterMesh(GeometryType type, const std::string& objPath)
{
	auto pMesh = std::make_unique<DX11::ResourcePool::MeshData>();
	string name;
	switch (type)
	{
	case YXX::GeometryType::Sphere:
		break;
	case YXX::GeometryType::Cube:
	{
		auto data = YMesh::CreateBox<YMesh::VertexPosNormalTex, DWORD>(0.5f,0.5f,1.0f);
		pMesh->GenerateGPUResource(pImpl->mpDevice.Get(),
			data.vertexVec.data(),
			data.vertexVec.size(),
			sizeof(YMesh::VertexPosNormalTex),
			data.indexTypeVec.data(),
			data.indexTypeVec.size(),
			sizeof(DWORD));
		name = "Cube";
	}
		break;
	case YXX::GeometryType::Obj:
		break;
	default:
		break;
	}
	REGISTER_RESOURCE(MeshData, name, pMesh);

}

BindFlag operator|(BindFlag l, BindFlag r)
{
	return BindFlag((UINT)l | (UINT)r);
}