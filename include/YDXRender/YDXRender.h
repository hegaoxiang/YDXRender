#pragma once
#include <string>
#include <memory>
#include "PipelineStateObject.h"
#include "Util.h"
namespace YXX
{
	enum  BindFlag
	{
		SRV = 0x8L,
		RTV = 0x20L,
		DSV = 0x40L
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
			friend class CommandBuffer;
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
				Texture() = default;
				Texture(UINT width, UINT height, DXGI_FORMAT format, BindFlag BindFlags, ID3D11Device* device)
				{
					D3D11_TEXTURE2D_DESC texDesc;
					texDesc.Width = width;
					texDesc.Height = height;
					texDesc.MipLevels = 1;
					texDesc.ArraySize = 1;
					texDesc.Format = format;
					texDesc.SampleDesc.Count = 1;
					texDesc.SampleDesc.Quality = 0;
					texDesc.Usage = D3D11_USAGE_DEFAULT;
					texDesc.BindFlags = (UINT)BindFlags;
					texDesc.CPUAccessFlags = 0;
					texDesc.MiscFlags = 0;
					
					HR(device->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
	
					if (format != DXGI_FORMAT_D24_UNORM_S8_UINT)
					{
						/// SRV
						D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
						srvDesc.Format = format;
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MipLevels = 1;
						srvDesc.Texture2D.MostDetailedMip = 0;
						HR(device->CreateShaderResourceView(tex.Get(), &srvDesc, SRV.GetAddressOf()));

						/// RTV
						D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
						rtvDesc.Format = format;
						rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
						rtvDesc.Texture2D.MipSlice = 0;

						HR(device->CreateRenderTargetView(tex.Get(), &rtvDesc, RTV.GetAddressOf()));
					}
				}
			};

			class ParameterHeap
			{
				friend class DXDevice;
				friend class CommandBuffer;
				// Constant Buffer
				struct CBufferData
				{
					std::string cbufferName;
					std::unique_ptr<BYTE[]> pData;
					UINT startSlot;
					UINT byteWidth;

					BOOL isDirty;
					ComPtr<ID3D11Buffer> cBuffer;

					CBufferData() = default;
					CBufferData(const std::string& bName, UINT startSlot, UINT byteWidth, BYTE* initData = nullptr)
						: cbufferName(bName), startSlot(startSlot), byteWidth(byteWidth), pData(new BYTE[byteWidth]), isDirty()
					{
						if (initData)
						{
							memcpy_s(pData.get(), byteWidth, initData, byteWidth);
						}
					}
					HRESULT CreateBuffer(ID3D11Device* device)
					{
						if (cBuffer != nullptr)
							return S_OK;
						D3D11_BUFFER_DESC cbd;
						ZeroMemory(&cbd, sizeof(cbd));
						cbd.Usage = D3D11_USAGE_DYNAMIC;
						cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
						cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
						cbd.ByteWidth = byteWidth;
						return device->CreateBuffer(&cbd, nullptr, cBuffer.GetAddressOf());
					}

					void UpdateBuffer(ID3D11DeviceContext* deviceContext)
					{
						if (isDirty)
						{
							isDirty = false;
							D3D11_MAPPED_SUBRESOURCE mappedData;
							deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
							memcpy_s(mappedData.pData, byteWidth, pData.get(), byteWidth);
							deviceContext->Unmap(cBuffer.Get(), 0);
						}
					}
				};
				// variable in one CBuffer
				struct ConstantBufferVariable
				{
				private:
					UINT startByteOffset = 0;
					UINT byteWidth = 0;
					CBufferData* pCBufferData = nullptr;
				public:
					ConstantBufferVariable(UINT startByteOffset,
						UINT byteWidth, CBufferData* pCBufferData) :
						startByteOffset(startByteOffset),
						byteWidth(byteWidth), pCBufferData(pCBufferData) {}
					void SetRaw(const void* data, UINT byteOffset = 0, UINT byteCount = 0xFFFFFFFF)
					{
						if (byteOffset > byteWidth)
							return;
						if (byteOffset + byteCount > byteWidth)
							byteCount = byteWidth - byteOffset;

						// 仅当值不同时更新
						if (memcmp(pCBufferData->pData.get() + startByteOffset + byteOffset, data, byteCount))
						{
							memcpy_s(pCBufferData->pData.get() + startByteOffset + byteOffset, byteCount, data, byteCount);
							pCBufferData->isDirty = true;
						}
					}

					void SetUInt(UINT val)
					{
						SetRaw(&val, 0, 4);
					}

					void SetSInt(INT val)
					{
						SetRaw(&val, 0, 4);
					}

					void SetFloat(FLOAT val)
					{
						SetRaw(&val, 0, 4);
					}

					void SetUIntVector(UINT numComponents, const UINT data[4])
					{
						if (numComponents > 4)
							numComponents = 4;
						UINT byteCount = numComponents * sizeof(UINT);
						if (byteCount > byteWidth)
							byteCount = byteWidth;
						SetRaw(data, 0, byteCount);
					}

					void SetSIntVector(UINT numComponents, const INT data[4])
					{
						if (numComponents > 4)
							numComponents = 4;
						UINT byteCount = numComponents * sizeof(INT);
						if (byteCount > byteWidth)
							byteCount = byteWidth;
						SetRaw(data, 0, byteCount);
					}

					void SetFloatVector(UINT numComponents, const FLOAT data[4])
					{
						if (numComponents > 4)
							numComponents = 4;
						UINT byteCount = numComponents * sizeof(FLOAT);
						if (byteCount > byteWidth)
							byteCount = byteWidth;
						SetRaw(data, 0, byteCount);
					}

					void SetUIntMatrix(UINT rows, UINT cols, const UINT* noPadData)
					{
						SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(noPadData));
					}

					void SetSIntMatrix(UINT rows, UINT cols, const INT* noPadData)
					{
						SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(noPadData));
					}

					void SetFloatMatrix(UINT rows, UINT cols, const DirectX::XMMATRIX noPadData)
					{

						SetMatrixInBytes(rows, cols, reinterpret_cast<const BYTE*>(&noPadData));
					}

					HRESULT GetRaw(void* pOutput, UINT byteOffset = 0, UINT byteCount = 0xFFFFFFFF)
					{
						if (byteOffset > byteWidth || byteCount > byteWidth - byteOffset)
							return E_BOUNDS;
						if (!pOutput)
							return E_INVALIDARG;
						memcpy_s(pOutput, byteCount, pCBufferData->pData.get() + startByteOffset + byteOffset, byteCount);
						return S_OK;
					}

					void SetMatrixInBytes(UINT rows, UINT cols, const BYTE* noPadData)
					{
						// 仅允许1x1到4x4
						if (rows == 0 || rows > 4 || cols == 0 || cols > 4)
							return;
						UINT remainBytes = byteWidth < 64 ? byteWidth : 64;
						BYTE* pData = pCBufferData->pData.get() + startByteOffset;
						while (remainBytes > 0 && rows > 0)
						{
							UINT rowPitch = sizeof(DWORD) * cols < remainBytes ? sizeof(DWORD) * cols : remainBytes;
							// 仅当值不同时更新
							if (memcmp(pData, noPadData, rowPitch))
							{
								memcpy_s(pData, rowPitch, noPadData, rowPitch);
								pCBufferData->isDirty = true;
							}
							noPadData += cols * sizeof(DWORD);
							pData += 16;
							remainBytes = remainBytes < 16 ? 0 : remainBytes - 16;
						}
					}
				};

				using Slot = UINT;
				std::unordered_map<std::string, Slot> SRVLayout;
				// it means null resource when the resourceHandle is -1
				std::unordered_map<Slot, ResourceHandle> SRVHeap;

				std::unordered_map<Slot, CBufferData> CBufferHeap;

				
				using VarName = std::string;
				std::unordered_map<VarName, S<ConstantBufferVariable>> ConstantVariables;

			public:

				void Apply(ID3D11DeviceContext* context);

				S<ConstantBufferVariable> GetConstantBufferVariable(const std::string& vName)
				{
					auto iter = ConstantVariables.find(vName);
					if (iter != ConstantVariables.end())
					{
						return iter->second;
					}
					throw "not find var named" + vName;
				}

				void SetShaderResourceByName(const std::string& bufferName, ResourceHandle pTex)
				{
					auto slot = SRVLayout.find(bufferName);
					if (slot != SRVLayout.end())
						SRVHeap[slot->second] = pTex;
					else
						throw "not find srv named" + bufferName;
				}

			};

			class PipelineStateObject
			{
				friend class CommandBuffer;
				friend class DXDevice;
			
			private:
				

				ComPtr<ID3D11RasterizerState> mpRS;
				ComPtr<ID3D11VertexShader> mpVS;
				ComPtr<ID3D11PixelShader> mpPS;
				ComPtr<ID3D11InputLayout>mpInputLayout;
				D3D11_VIEWPORT mVP;
				
				void AppendLayout(ID3D11Device* device, ID3D11ShaderReflection* pShaderReflection)
				{
					if (mpLayout == nullptr)
						mpLayout = std::make_shared<ParameterLayout>();

					auto& CBufLayout = mpLayout->CBufLayout;
					auto& SRVHeapLayout = mpLayout->SRVLayout;

					D3D11_SHADER_DESC sd;
					HR(pShaderReflection->GetDesc(&sd));

					HRESULT hr;
					for (UINT i = 0;; ++i)
					{
						D3D11_SHADER_INPUT_BIND_DESC sib;
						hr = pShaderReflection->GetResourceBindingDesc(i, &sib);
						//HR(hr);
						// After reading all variable,it fails,but this it not a failed call
						if (FAILED(hr))
							break;

						switch (sib.Type)
						{
							// constant buffer
						case D3D_SIT_CBUFFER:
						{
							ID3D11ShaderReflectionConstantBuffer* pSRCBuffer = pShaderReflection->GetConstantBufferByName(sib.Name);
							// 获取cbuffer内的变量信息并建立映射
							D3D11_SHADER_BUFFER_DESC cbDesc{};
							hr = pSRCBuffer->GetDesc(&cbDesc);
							if (FAILED(hr))
								return;

							ParameterLayout::CBufStructure cbuffer;
							// Record offset of each variable
							for (UINT j = 0; j < cbDesc.Variables; ++j)
							{
								ID3D11ShaderReflectionVariable* pSRVar = pSRCBuffer->GetVariableByIndex(j);
								D3D11_SHADER_VARIABLE_DESC svDesc;

								hr = pSRVar->GetDesc(&svDesc);
								if (FAILED(hr))
									return;

								cbuffer[svDesc.Name].StartOffset = svDesc.StartOffset;
								cbuffer[svDesc.Name].Size = svDesc.Size;
							};
							
							ParameterLayout::CbufferInfo ci;
							ci.cbufferName = sib.Name;
							ci.startSlot = sib.BindPoint;
							ci.byteWidth = cbDesc.Size;
							// Determines the Slot of this buffer
							auto it = CBufLayout.find(ci);
							// per slot per cbuffer
							assert(it == CBufLayout.end());
							{
								CBufLayout.emplace(std::make_pair(ParameterLayout::CbufferInfo(ci), std::move(cbuffer)));
							}
							
						}
						break;
						case D3D_SIT_TEXTURE:
						case D3D_SIT_STRUCTURED:
						case D3D_SIT_BYTEADDRESS:
						case D3D_SIT_TBUFFER:
						{
							auto it = SRVHeapLayout.find(sib.Name);
							assert(it == SRVHeapLayout.end());
							{
								SRVHeapLayout[sib.Name] = sib.BindPoint;
							}
						}
						break;
						// RW resource
						case D3D_SIT_UAV_RWTYPED:
						case D3D_SIT_UAV_RWSTRUCTURED:
						case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
						case D3D_SIT_UAV_APPEND_STRUCTURED:
						case D3D_SIT_UAV_CONSUME_STRUCTURED:
						case D3D_SIT_UAV_RWBYTEADDRESS:
						{
							//auto it = sr->mRWResources.find(sib.BindPoint);
							//if (it == sr->mRWResources.end())
							//{
							//	sr->mRWResources.emplace(std::make_pair(sib.BindPoint,
							//		ShaderReflector::RWResource{ sib.Name, static_cast<D3D11_UAV_DIMENSION>(sib.Dimension), nullptr, 0, false }));
							//}
						}
						break;
						default:
							break;
						}
					}
				}
				S<ParameterLayout> mpLayout;

				void Apply(ID3D11DeviceContext* context);
			};
			
			class MeshData
			{
				UINT mStride;
				ComPtr<ID3D11Buffer> mpVBuf;
				ComPtr<ID3D11Buffer> mpIBuf;
			public:
				void GenerateGPUResource(ID3D11Device* device,const void* vData, size_t vSize,size_t vByteWidth,
					const void* iData, size_t iSize, size_t iByteWidth)
				{
					mStride = vByteWidth;
					// 设置顶点缓冲区描述
					D3D11_BUFFER_DESC vbd;
					ZeroMemory(&vbd, sizeof(vbd));
					vbd.Usage = D3D11_USAGE_IMMUTABLE;
					vbd.ByteWidth = vSize * vByteWidth;
					vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vbd.CPUAccessFlags = 0;

					// 新建顶点缓冲区
					D3D11_SUBRESOURCE_DATA InitData;
					ZeroMemory(&InitData, sizeof(InitData));
					InitData.pSysMem = vData;
					HR(device->CreateBuffer(&vbd, &InitData, mpVBuf.ReleaseAndGetAddressOf()));
				
					// index buffer
					D3D11_BUFFER_DESC indexBufferDesc;
					indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					indexBufferDesc.ByteWidth = iByteWidth * iSize;
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					indexBufferDesc.CPUAccessFlags = 0;
					indexBufferDesc.MiscFlags = 0;
					indexBufferDesc.StructureByteStride = 0;

					//set up index data
					D3D11_SUBRESOURCE_DATA indexData;
					indexData.pSysMem = iData;
					indexData.SysMemPitch = 0;
					indexData.SysMemSlicePitch = 0;

					//create index buffer
					HR(device->CreateBuffer(
						&indexBufferDesc,
						&indexData,
						mpIBuf.ReleaseAndGetAddressOf()));
				}

				void Apply(ID3D11DeviceContext* context);
			};

			std::unordered_map<std::string, ResourceHandle> TextureMap;
			std::vector<S<Texture>> TextureResource;

			std::unordered_map<std::string, ResourceHandle> PipelineStateObjectMap;
			std::vector<U<PipelineStateObject>> PipelineStateObjectResource;

			std::unordered_map<std::string, ResourceHandle> ParameterHeapMap;
			std::vector<U<ParameterHeap>> ParameterHeapResource;

			std::unordered_map<std::string, ResourceHandle> MeshDataMap;
			std::vector<U<MeshData>> MeshDataResource;

			ParameterHeap* GetParameterHeapByHandle(ResourceHandle handle)
			{
				assert(handle < ParameterHeapResource.size());
				return ParameterHeapResource[handle].get();
			}
			PipelineStateObject* GetPipelineStateObjectByHandle(ResourceHandle handle)
			{
				assert(handle < PipelineStateObjectResource.size());
				return PipelineStateObjectResource[handle].get();
			}
		public:
			MeshData* GetMeshDataByHandle(ResourceHandle handle)
			{
				assert(handle < MeshDataResource.size());
				return MeshDataResource[handle].get();
			}
			Texture* GetTextureByHandle(ResourceHandle handle)
			{
				assert(handle < TextureResource.size());
				return TextureResource[handle].get();
			}
			ID3D11ShaderResourceView* GetTextureByHandleForImgui(ResourceHandle handle)
			{
				auto tex = GetTextureByHandle(handle);
				return tex->SRV.Get();
			}
			
		};
	}
	enum class GeometryType
	{
		Sphere,
		Cube,
		Obj
	};

	class DXDevice
	{
		friend class DXFramework;
		std::tuple<ID3D11Device*,IDXGISwapChain*> Init(HWND hWnd);
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

		// support pos_normal_tex only now
		ResourceHandle RegisterMesh(GeometryType type,const std::string& objPath = nullptr);

		ResourceHandle RegisterPSO(const std::string& name,const PipelineStateObjectDesc& desc);
		ResourceHandle GetPSOByName(const std::string& name);

		ResourceHandle RegisterParaHeap(const std::string& name, ResourceHandle pso);
		//ResourceHandle GetPSOByName(const std::string& name);

		ResourceHandle RegisterTexture(const std::string& name, Texture2DDesc desc);

		ResourceHandle GetTextureHandle(const std::string& name);
	protected:
		// called when size changes.
		friend void UpdateBackBuffer(DXDevice& device,const std::string& name, UINT width, UINT height);
		
		ResourceHandle RegisterBackBuffer(const std::string& name);

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

		void DrawMesh(ResourceHandle meshHandle);
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
