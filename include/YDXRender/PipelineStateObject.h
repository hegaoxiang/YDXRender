
#pragma once
#include "Util.h"

namespace YXX
{
	struct ParameterLayout
	{
		using Slot = UINT;
		std::unordered_map<std::string, Slot> SRVLayout;
		
		using VarName = std::string;
		using Offset = UINT;
		struct ItemInfo
		{
			Offset StartOffset;
			UINT Size;
		};
		using CBufStructure = std::unordered_map<VarName, ItemInfo>;
		struct CbufferInfo
		{
			std::string cbufferName;
			UINT startSlot;
			UINT byteWidth;
			struct HashFunc
			{
				size_t operator()(const CbufferInfo& i) const {
					return i.startSlot;
				}
			};
			bool operator==(const CbufferInfo& i)const
			{
				return i.startSlot == startSlot;
			}
		};
		std::unordered_map<CbufferInfo, CBufStructure, CbufferInfo::HashFunc> CBufLayout;
	};
	class ParameterHeap
	{
		friend class CommandBuffer;
	public:
		
	};

	enum  class RS
	{
		DEFAULT,
		CULL_BACK,
		CULL_NONE,
		CULL_FRONT,
		FILL_WIREFRAME,

		NUM_RS_PRESET
	};
	enum class ShaderType
	{
		VertexShader,
		PixelShader
	};
	struct PipelineStateObjectDesc
	{
		std::string ShaderName;


		RS RasterType;
		std::string RTV;
		std::string DSV;

		int ViewportWidth;
		int ViewportHeight;
	};
}