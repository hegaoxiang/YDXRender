#pragma once
#include <string>
#include <memory>
#include "PipelineStateObject.h"
#include "DXFramework.h"
namespace YXX
{
	enum class BindFlag
	{
		SRV,
		RTV,
		DSV
	};
	
	struct Texture2DDesc
	{
		UINT Width;         			// 纹理宽度
		UINT Height;        			// 纹理高度
		DXGI_FORMAT Format;
		BindFlag flag;

	};
	
	using ResourceHandle = size_t;

	class CommandBuffer;
	class DXDevice;

	namespace DX11
	{
		class ResourcePool
		{
			friend class DXDevice;
		public:
			static ResourcePool& Get()
			{
				static ResourcePool instance;
				return instance;
			}
		private:
			ResourcePool() {}
			struct Texture
			{
				ComPtr<ID3D11Texture2D> tex;

				ComPtr<ID3D11ShaderResourceView> SRV;

				ComPtr<ID3D11DepthStencilView> DSV;

				ComPtr<ID3D11RenderTargetView> RTV;
			};
			class PipelineStateObject
			{
				friend class CommandBuffer;
			public:

				ParameterLayout GetParameterLayout()const;

				D3D11_RASTERIZER_DESC mRasterState;
				ComPtr<ID3D11VertexShader> mpVS;
				ComPtr<ID3D11PixelShader> mpPS;
				D3D11_VIEWPORT mVP;
			};

			std::unordered_map<std::string, ResourceHandle> TextureMap;
			std::vector<S<Texture>> TextureResource;

			std::unordered_map<std::string, ResourceHandle> PipelineStateObjectMap;
			std::vector<PipelineStateObject> PipelineStateObjectResource;

		public:
			Texture* GetTextureByHandle(ResourceHandle handle)
			{
				assert(handle < TextureResource.size());
				return TextureResource[handle].get();
			}
		};
	}
	
	class DXDevice
	{
		friend class DXFramework;
		std::tuple<ID3D11Device*,IDXGISwapChain*, ID3D11DeviceContext*> Init(HWND hWnd);
	public:
		static DXDevice& Get()noexcept
		{
			static DXDevice ins;
			return ins;
		}

		S<CommandBuffer> CreateCommandBuffer();
		/// <summary>
		/// 
		/// </summary>
		/// <param name="cmdBuf"></param>
		/// <param name="clearList">it will clear all cmd lists per call,set true when you need to modify cmd list per call</param>
		void ExcuteCommandBuffer(CommandBuffer* cmdBuf,bool clearList = true);

		void RegisterPSO(const std::string& name,const PipelineStateObjectDesc& desc);
		ResourceHandle GetPSOByName(const std::string& name);

		void RegisterTexture(const std::string& name, Texture2DDesc desc);
		void RegisterBackBuffer(const std::string& name);

		// called when size changes.
		void UpdateBackBuffer(const std::string& name, UINT width, UINT height);
		ResourceHandle GetTextureHandle(const std::string& name);

	private:
		class Impl;
		U<Impl>pImpl;

		DXDevice();
		~DXDevice();
	};

	
	

	

	class CommandBuffer
	{
		friend class DXDevice;
		friend class DXFramework;
		CommandBuffer() {};
	public:
		CommandBuffer(CommandBuffer& cb)
		{
			mpDeferredContext = cb.mpDeferredContext;
			mpd3dCommandLists = cb.mpd3dCommandLists;
		}

		void SetPipelineStateObject(ResourceHandle pso);
		void SetParameterHeap(ResourceHandle paraHeap);

		void DrawMesh();
		// it is only used by internal imgui
		void FinishRecord();

		void SetRenderTargets(std::vector<ResourceHandle> rtvs,ResourceHandle dsv = -1);
		void ClearRenderTarget(ResourceHandle rtv);

	private:
		void ClearList()
		{
			mpd3dCommandLists.clear();
		}
	private:
		ComPtr<ID3D11DeviceContext> mpDeferredContext;
		std::vector<ComPtr<ID3D11CommandList>> mpd3dCommandLists;
	};

	
	
	

}