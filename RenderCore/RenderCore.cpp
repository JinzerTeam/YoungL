﻿// RenderCore.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "RenderCore.h"

#include "D3DExample.h"

#define MAX_LOADSTRING 100



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    DXExample Example(hInstance);
    if (Example.Initialize())
    {
        return Example.Run();
    }
    else
    {
        return 0;
    }
}