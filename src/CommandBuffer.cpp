#include "YDXRender/YDXRender.h"

#include <unordered_map>
#include <iostream>

using namespace YXX;
using namespace std;
using namespace DX11;

void CommandBuffer::DrawMesh()
{
	FinishRecord();
}
void YXX::CommandBuffer::SetRenderTargets(vector<ResourceHandle> rtvs, ResourceHandle dsv)
{
	auto& resPool = ResourcePool::Get();

	vector<ID3D11RenderTargetView*> realRtvs;
	for_each(begin(rtvs), end(rtvs), [&resPool, &realRtvs](ResourceHandle handle) {
		realRtvs.push_back(ResourcePool::Get().GetTextureByHandle(handle)->RTV.Get());
		});

	ID3D11DepthStencilView* realDsv = nullptr;
	if(dsv != -1)
		realDsv = ResourcePool::Get().GetTextureByHandle(dsv)->DSV.Get();

	mpDeferredContext->OMSetRenderTargets(realRtvs.size(), realRtvs.data(), realDsv);
}

void YXX::CommandBuffer::ClearRenderTarget(ResourceHandle rtv)
{
	auto tex = ResourcePool::Get().GetTextureByHandle(rtv);

	static float black[4] = { 1,0,0,0 };
	mpDeferredContext->ClearRenderTargetView(tex->RTV.Get(), black);
}

void YXX::CommandBuffer::FinishRecord()
{
	ComPtr<ID3D11CommandList> list;
	HR(mpDeferredContext->FinishCommandList(FALSE, list.GetAddressOf()));

	mpd3dCommandLists.emplace_back(move(list));
}
