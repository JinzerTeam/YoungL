// EngineMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EngineMain.h"
#include "common/d3dUtil.h"
#include "BoxApp.h"
#include "ShapesApp.h"
#include "LandAndWaveApp.h"
#include "LitWave/LitWavesApp.h"
#include "LitWave/LitColumnsApp.h"
#include "Blend/BlendApp.h"
#include "Stencil/StencilApp.h"
#include "Blend/GeometryShaderApp.h"
#include "CS/VecAddApp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif

	try
	{
		//BoxApp theApp(hInstance);
		//ShapeApp theApp(hInstance);
		//LandAndWavesApp theApp(hInstance);
		//LitWaveApp theApp(hInstance);
		//LitColumnsApp theApp(hInstance);
		//BlendApp theApp(hInstance);
		//StencilApp theApp(hInstance);
		//GeometryShaderApp theApp(hInstance);
		VecAddApp theApp(hInstance);
		if (!theApp.Initialize())
		{
			return 0;
		}
		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
	}

	return 1;
}