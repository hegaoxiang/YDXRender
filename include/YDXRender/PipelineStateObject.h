#include <string>
#include<d3d11_1.h>
#pragma once

namespace YXX
{
	

	struct ParameterLayout
	{
		
	};
	class ParameterHeap
	{
		friend class CommandBuffer;
	public:
		
	};
	struct 
	{

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
	struct PipelineStateObjectDesc
	{
		std::string VS;
		std::string PS;

		RS RasterType;
		std::string RTV;
		std::string DSV;

		int ViewportWidth;
		int ViewportHeight;
	};
	


}