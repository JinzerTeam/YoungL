#include "D3DExample.h"
#include <fstream>
#include <DirectXColors.h>

#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

bool DXExample::Initialize()
{
	InitWindow();

	LoadPipline();

	OnResize();

	BuildRootSignature();
	BuildDescriptorHeap();
	BuildConstantBuffers();
	BuildShadersInputLayout();
	BuildPsos();

	LoadAsset();

	return true;
}

bool DXExample::Run()
{
	MSG Msg = { 0 };

	while (Msg.message != WM_QUIT)
	{
		if (PeekMessage(&Msg, 0, 0, 0 , PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			Update();
			Render();
		}
	}

	return (int)Msg.wParam;
}

LRESULT CALLBACK
WndProc(HWND Hwnd, UINT Msg, WPARAM WPara, LPARAM LPara)
{
	switch (Msg)
	{
	case WM_QUIT:
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(Hwnd, Msg, WPara, LPara);
}

void DXExample::InitWindow()
{
	//window class
	WNDCLASS WDClass;
	WDClass.hInstance = AppInstan;
	WDClass.lpfnWndProc = WndProc;
	WDClass.cbClsExtra = 0;
	WDClass.cbWndExtra = 0;
	WDClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WDClass.hCursor = LoadCursor(0, IDC_ARROW);
	WDClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WDClass.style = CS_HREDRAW | CS_VREDRAW;
	WDClass.lpszClassName = L"MainWnd";
	WDClass.lpszMenuName = L"D3DEx";

	RegisterClass(&WDClass);
	//adjust, create window
	RECT Rec = {0, 0, ClientWidth, ClientHeight};
	AdjustWindowRect(&Rec, WS_OVERLAPPEDWINDOW, false);
	int Width = Rec.right - Rec.left;
	int Height = Rec.bottom - Rec.top;
	Mainhandle = CreateWindow(L"MainWnd", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, AppInstan, 0);

	//show window
	ShowWindow(Mainhandle, SW_SHOW);
	UpdateWindow(Mainhandle);

}

void DXExample::LoadPipline()
{
	//debug controller
#if defined(DBUG) || defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> DebugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&DebugInterface));
	DebugInterface->EnableDebugLayer();
#endif
	
	
	//create factory
	CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));

	//enumerate adapter device
	//try to create hard ware device
	HRESULT Result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));

	if (FAILED(Result))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter));
		D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	}
	
	RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//create commang objects
	m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue));
	m_Device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));

	m_CommandList->Close();

	//create fence
	m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	//create swapchain
	m_SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferCount = m_SwapchainBackbufferCount;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = Mainhandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.BufferDesc.Width = ClientWidth;
	SwapChainDesc.BufferDesc.Height = ClientHeight;
	SwapChainDesc.BufferDesc.Format = BackBufferFormat;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;


	m_Factory->CreateSwapChain(m_CommandQueue.Get(), &SwapChainDesc, m_SwapChain.GetAddressOf());
	
	//create discriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NumDescriptors = m_SwapchainBackbufferCount;
	RtvHeapDesc.NodeMask = 0;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_Device->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(&RtvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.NodeMask = 0;
	DsvHeapDesc.NumDescriptors = 1;
	
	m_Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&DsvHeap));
}

void DXExample::OnResize()
{
	FlushCommandQueue();

	//reset command list
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	//reset back buffer;
	for (int i =0; i< m_SwapchainBackbufferCount; ++i)
	{
		m_BackBuffer[i].Reset();
	}
	m_DepthStencilBuffer.Reset();

	//resize swapchain
	m_SwapChain->ResizeBuffers(m_SwapchainBackbufferCount, ClientWidth, ClientHeight, 
		BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	M_CurrentBackBuffer = 0;
		 
	//get back buffer from swap chain && create rtv
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtvhandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_SwapchainBackbufferCount; ++i)
	{
		m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffer[i]));
		m_Device->CreateRenderTargetView(m_BackBuffer[i].Get(), nullptr, Rtvhandle);
		Rtvhandle.Offset(1, RtvDescriptorSize);
	}

	//create depth staencil buffer and dsv
	D3D12_RESOURCE_DESC DRDesc;
	DRDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DRDesc.Alignment = 0;
	DRDesc.Width = ClientWidth;
	DRDesc.Height = ClientHeight;
	DRDesc.DepthOrArraySize = 1;
	DRDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	DRDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	DRDesc.SampleDesc.Count = 1;
	DRDesc.SampleDesc.Quality =  0;
	DRDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DRDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DepthStencilFormat;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;

	m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &DRDesc, D3D12_RESOURCE_STATE_COMMON,
		&ClearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(DsvHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_DEPTH_STENCIL_VIEW_DESC DsvDesc;
	DsvDesc.Format = DepthStencilFormat;
	DsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	DsvDesc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &DsvDesc, DsvHandle);

	//transilate dsv stato to depth
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//excute command list
	m_CommandList->Close();
	ID3D12CommandList* cmdLists[] = {m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	
	//flush command queue
	FlushCommandQueue();

	//update screen view port
	M_ScreenViewport.Width = static_cast<float>(ClientWidth);
	M_ScreenViewport.Height = static_cast<float>(ClientHeight);
	M_ScreenViewport.TopLeftX = 0;
	M_ScreenViewport.TopLeftY = 0;
	M_ScreenViewport.MaxDepth = 1.0f;
	M_ScreenViewport.MinDepth = 0.0f;

	M_ScissorRect = { 0,0, ClientWidth, ClientHeight };
}

void DXExample::LoadAsset()
{
	
	//load asset
	std::vector<Vertex> Vertices;
	std::ifstream Fin("E:/mypros/TestUE4/Saved/Model/ModelSave.Bin", std::ios::in | std::ios::binary);

	int VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	for (int i = 0; i < VertexNum; ++i)
	{
		Vertex NewVert;
		Fin.read((char*)&NewVert.Position.x, sizeof(float));
		Fin.read((char*)&NewVert.Position.y, sizeof(float));
		Fin.read((char*)&NewVert.Position.z, sizeof(float));

		NewVert.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		Vertices.push_back(NewVert);
	}

	std::vector<uint32_t> Indices;
	int IndexNum;
	Fin.read((char*)&IndexNum, sizeof(int));
	for (int i = 0; i < IndexNum; i++)
	{
		uint32_t VertIndex;
		Fin.read((char*)&VertIndex, sizeof(int));
		Indices.push_back(VertIndex);
	}

	Fin.close();

	//create resource
	std::unique_ptr<DrawGeometry> Geometry = std::make_unique<DrawGeometry>(m_Device.Get(), Vertices, Indices);
	Geometies.push_back(std::move(Geometry));

}

void DXExample::Update()
{
	XMFLOAT4X4 mWorld = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	XMFLOAT4X4 mView = mWorld;
	XMFLOAT4X4 mProj = mWorld;

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(100, 0, 0, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	M_ConstantUploadBuffer->CopyData(0, objConstants);
}

void DXExample::Render()
{
	PopulateCommands();
	FlushCommandQueue();
}

void DXExample::PopulateCommands()
{
	//reset
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), M_Pso.Get());
	
	//viewport
	m_CommandList->RSSetViewports(1, &M_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &M_ScissorRect);

	//change targe state
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//clear target view
	m_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_CommandList->OMSetRenderTargets(1, &GetCurrentBackBufferView(), true, &GetCurrentDepthStencilView());

	//
	ID3D12DescriptorHeap* DescriporHeaps[] = {M_CbvSrvUavHeap.Get()};
	m_CommandList->SetDescriptorHeaps(_countof(DescriporHeaps), DescriporHeaps);
	m_CommandList->SetGraphicsRootSignature(M_RootSignaure.Get());
	
	m_CommandList->IASetVertexBuffers(0, 1, &Geometies[0]->GetVertexBufferView());
	m_CommandList->IASetIndexBuffer(&Geometies[0]->GetIndexBufferView());
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CommandList->SetGraphicsRootDescriptorTable(0, M_CbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());

	m_CommandList->DrawIndexedInstanced(Geometies[0]->IndexCount, 1, 0, 0, 0);

	//change targe state
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_CommandList->Close();
	ID3D12CommandList* CmdLists[] = { m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);

	m_SwapChain->Present(0, 0);

	M_CurrentBackBuffer = (M_CurrentBackBuffer + 1) % m_SwapchainBackbufferCount;
}

void DXExample::FlushCommandQueue()
{
	m_CurrentFenceValue++;
	
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFenceValue);

	if (m_Fence->GetCompletedValue() < m_CurrentFenceValue)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr,false, EVENT_ALL_ACCESS);
		m_Fence->SetEventOnCompletion(m_CurrentFenceValue, EventHandle);

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void DXExample::BuildRootSignature()
{
	//D3D12_ROOT_PARAMETER RootParameters[1];
	//RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//
	//D3D12_ROOT_DESCRIPTOR_TABLE DesTable;
	//DesTable.NumDescriptorRanges = 1;

	//D3D12_DESCRIPTOR_RANGE DesRange;
	//DesRange.BaseShaderRegister = 0;
	//DesRange.NumDescriptors = 1;
	//DesRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//DesRange.RegisterSpace = 0;
	//DesTable.pDescriptorRanges = &DesRange;
	//
	//RootParameters[0].DescriptorTable = DesTable;

	//D3D12_ROOT_SIGNATURE_DESC RootSigDesc;
	//RootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	////1, RootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
	//RootSigDesc.NumParameters = 1;
	//RootSigDesc.pParameters = RootParameters;
	//RootSigDesc.NumStaticSamplers = 0;
	//RootSigDesc.pStaticSamplers = nullptr;

	//ComPtr<ID3DBlob> ResultOut;
	//ComPtr<ID3DBlob> ErrorOut;

	//D3D12SerializeRootSignature(&RootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, ResultOut.GetAddressOf(), ErrorOut.GetAddressOf());
	//m_Device->CreateRootSignature(0, ResultOut->GetBufferPointer(), ResultOut->GetBufferSize(), IID_PPV_ARGS(&M_RootSignaure));

	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	m_Device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&M_RootSignaure));
}

//for render model
void DXExample::BuildDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
	HeapDesc.NumDescriptors = 1;
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	HeapDesc.NodeMask = 0;
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&M_CbvSrvUavHeap));
}

void DXExample::BuildShadersInputLayout()
{
	UINT CompileFlags = 0;
	ComPtr<ID3DBlob> CompileError;
	std::wstring Path = L"Shaders\\Test.hlsl";
	D3DCompileFromFile(Path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0", CompileFlags, 0, &M_Vs, &CompileError);

	if (CompileError != nullptr)
	{
		OutputDebugStringA((char*)CompileError->GetBufferPointer());
	}

	D3DCompileFromFile(Path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0", CompileFlags, 0, &M_Ps, &CompileError);
	if (CompileError != nullptr)
	{
		OutputDebugStringA((char*)CompileError->GetBufferPointer());
	}

	M_ShadersInputDesc =
	{
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void DXExample::BuildPsos()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc;
	ZeroMemory(&Desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	Desc.NodeMask = 0;
	Desc.pRootSignature = M_RootSignaure.Get();
	Desc.InputLayout = { M_ShadersInputDesc.data(), (UINT)M_ShadersInputDesc.size()};
	Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	Desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);;
	Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);;
	Desc.VS = 
	{
		reinterpret_cast<BYTE*>(M_Vs->GetBufferPointer()),
		M_Vs->GetBufferSize()
	};
	Desc.PS = 
	{
		reinterpret_cast<BYTE*>(M_Ps->GetBufferPointer()),
		M_Ps->GetBufferSize()
	};
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = 1;
	Desc.RTVFormats[0] = BackBufferFormat;
	Desc.DSVFormat = DepthStencilFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;

	m_Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&M_Pso));
}

void DXExample::BuildConstantBuffers()
{
	M_ConstantUploadBuffer = std::make_unique<UploadBuffer<ObjectConstants>>(m_Device.Get(), 1, true);
	UINT ObjectBufferSize = UploadBuffer<ObjectConstants>::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = M_CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc;
	
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = M_ConstantUploadBuffer->GetResource()->GetGPUVirtualAddress();
	int BufIndex = 0;
	GpuAddress += BufIndex * ObjectBufferSize;
	ViewDesc.BufferLocation = GpuAddress;
	ViewDesc.SizeInBytes = ObjectBufferSize;

	m_Device->CreateConstantBufferView(&ViewDesc, CpuDescriptor);
}
