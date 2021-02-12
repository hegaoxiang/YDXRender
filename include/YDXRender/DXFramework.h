#pragma once
#include "Util.h"

namespace YXX
{
	class CommandBuffer;
	

	class DXFramework
	{
	public:
		virtual ~DXFramework();
		virtual void Init() = 0;
 
		virtual void Update(float dt) = 0;

		virtual void Quit() = 0;

		void Run();

	private:
		void BeginUpdate();

		void EndUpdate();

		HWND InitWindow();

		void DXFramework::InitRender(HWND hWnd);

		S<CommandBuffer> pMainList;
	};

	
	

}