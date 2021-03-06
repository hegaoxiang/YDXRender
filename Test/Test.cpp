#include <iostream>
#include <YDXRender/DXFramework.h>
#include <YDXRender/YDXRender.h>
#include <imgui/imgui.h>
#include <direct.h>
using namespace std;
using namespace YXX;

class Test :public YXX::DXFramework
{
public:

	ResourceHandle psoHandle;
	ResourceHandle psoParamHandle;
	ResourceHandle meshHandle;
	S<CommandBuffer> pCB;
	void Init() override
	{
		auto s = getcwd(NULL,0);
		PipelineStateObjectDesc desc;
		desc.ShaderName = "HLSL/Test.hlsl";
		desc.ViewportHeight = 400;
		desc.ViewportWidth = 400;
		psoHandle = DXDevice::Get().RegisterPSO("Test", desc);
	
		psoParamHandle = DXDevice::Get().RegisterParaHeap("Tetst", psoHandle);
	
		meshHandle = DXDevice::Get().RegisterMesh(GeometryType::Cube,"");

		Texture2DDesc texdesc;
		texdesc.Height = 400;
		texdesc.Width = 400;
		texdesc.flag = BindFlag(BindFlag::RTV | BindFlag::SRV);
		texdesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;

		auto image = DXDevice::Get().RegisterTexture("TestImage", texdesc);

		auto cb = DXDevice::Get().CreateCommandBuffer();
		cb->SetPipelineStateObject(psoHandle);
		cb->SetParameterHeap(psoParamHandle);
		cb->SetRenderTargets({ image });
		cb->ClearRenderTarget(image);
		pCB = cb;
	}


	void Update(float dt) override
	{
		pCB->DrawMesh(meshHandle);
		DXDevice::Get().ExcuteCommandBuffer(pCB.get());

		ImGui::Begin("Test");
		ImGui::Text("WT");
		ImGui::Image(DX11::ResourcePool::Get().GetTextureByHandleForImgui(DXDevice::Get().GetTextureHandle("TestImage"))
			, ImGui::GetContentRegionAvail());
		ImGui::End();
	}


	void Quit() override
	{
	}

};
int main()
{
	cout << "Begin Test";
	Test t;
	t.Run();
	cout << "end";
}