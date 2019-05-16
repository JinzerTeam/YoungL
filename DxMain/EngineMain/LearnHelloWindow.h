#pragma once

#include "DXSample.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class LearnHelloWindow : public DXSample
{
public:
	LearnHelloWindow(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

	void WaitForPreviousFrame();

protected:

	virtual void LoadAssets();
	virtual void PopulateCommandList();
	virtual void CreateDescriptorHeaps();
	virtual void CreateRTAndCmdAllocator();
private:

	void LoadPipeline();

protected:

	static const UINT FrameCount = 2;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	//CD3DX12_VIEWPORT m_viewport;
	//CD3DX12_RECT m_scissorRect;

	//pipeline objects

	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	//UINT m_rtvDescriptorSize;

	//app resource
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	//synchronization objects
	//UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	UINT m_rtvDescriptorSize;
	UINT m_frameIndex;
};