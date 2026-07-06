#include "VertexBuffer.h"


ID3D11Buffer* CreateVertexBuffer(ID3D11Device* pDevice, void* vtxData, UINT vtxNum)
{
	D3D11_BUFFER_DESC vtxBufDesc;
	ZeroMemory(&vtxBufDesc, sizeof(vtxBufDesc));
	vtxBufDesc.ByteWidth = sizeof(Vertex) * vtxNum;
	vtxBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vtxBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vtxSubResource;
	ZeroMemory(&vtxSubResource, sizeof(vtxSubResource));
	vtxSubResource.pSysMem = vtxData;

	ID3D11Buffer* pVtxBuf;
	HRESULT hr = pDevice->CreateBuffer(&vtxBufDesc, &vtxSubResource, &pVtxBuf);
	if (FAILED(hr)) return nullptr;

	return pVtxBuf;
}