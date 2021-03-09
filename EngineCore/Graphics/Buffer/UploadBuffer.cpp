#include "pch.h"
#include "UploadBuffer.h"
#include "../GraphicsCore.h"

using namespace Graphics;

void UploadBuffer::Create(const std::wstring& Name, size_t BufferSize)
{
	Destroy();
	m_BufferSize = BufferSize;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_UPLOAD;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	D3D12_RESOURCE_DESC ResourceDesc = {};
	ResourceDesc.Width = BufferSize;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResourceDesc.Flags = D3D12_HEAP_FLAG_NONE;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, MY_IID_PPV_ARGS(&m_Resource)));

	m_GpuVirtualAddress = m_Resource->GetGPUVirtualAddress();

#ifdef RELEASE
	(Name)
#else
	m_Resource->SetName(Name.c_str());
#endif // RELEASE

}

void* UploadBuffer::Map()
{
	void* Memory;
	m_Resource->Map(0, &CD3DX12_RANGE(0, m_BufferSize), &Memory);

	return Memory;
}

void UploadBuffer::Unmap(size_t Begin /* = 0 */, size_t End /* = -1 */)
{
	m_Resource->Unmap(0, CD3DX12_RANGE(Begin, std::min(End, m_BufferSize)));
}