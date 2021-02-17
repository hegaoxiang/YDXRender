#include "YDXRender/YDXRender.h"

void YXX::DX11::ResourcePool::ParameterHeap::Apply(ID3D11DeviceContext* context)
{
	for (auto& iter : SRVHeap)
	{
		auto slot = iter.first;
		auto tex = ResourcePool::Get().GetTextureByHandle(iter.second);

		context->PSSetShaderResources(slot, 1, tex->SRV.GetAddressOf());
		context->VSSetShaderResources(slot, 1, tex->SRV.GetAddressOf());
	}

	for (auto& iter : CBufferHeap)
	{
		auto slot = iter.first;
		auto& data = iter.second;
		context->VSSetConstantBuffers(slot, 1, data.cBuffer.GetAddressOf());
		context->PSSetConstantBuffers(slot, 1, data.cBuffer.GetAddressOf());
	}
}

void YXX::DX11::ResourcePool::PipelineStateObject::Apply(ID3D11DeviceContext* context)
{
	context->VSSetShader(mpVS.Get(), nullptr, 0);
	context->VSSetShader(mpVS.Get(), nullptr, 0);
	context->IASetInputLayout(mpInputLayout.Get());
	//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(mpRS.Get());
	context->RSSetViewports(1,&mVP);
}

void YXX::DX11::ResourcePool::MeshData::Apply(ID3D11DeviceContext* context)
{
	static UINT offset = 0;
	context->IASetVertexBuffers(0, 1, mpVBuf.GetAddressOf(), &mStride, &offset);
	context->IASetIndexBuffer(mpIBuf.Get(), DXGI_FORMAT_R32_UINT, 0);
}