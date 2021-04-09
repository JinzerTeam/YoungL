#pragma once

#include "RHI/RHIContext.h"
#include "MeshActor.h"

class FRenderer
{
public:
	void CreateRHIContext(int InWidth, int Inheight);
	void DestroyRHIContext();

	void RenderObjects();

	void Resize(int InWidth, int InHeight);

	//
	void CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries);

protected:
private:

	IRHIContext* RHIContext;

	std::map<std::string, IRHIGraphicsPipelineState*> GraphicsPSOs;

	std::vector<IRHIRenderingItem*> RenderingItems;

	//
	FViewport Viewport;
};