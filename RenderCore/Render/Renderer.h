#pragma once

#include "RHI/RHIContext.h"
#include "../Game/StaticMeshActor.h"
#include "../Game/SkeletalMeshActor.h"
#include "ShadowMap.h"
#include "PostProcessing.h"
#include "RenderResourceManager.h"
#include "PSOManager.h"
#include "GPUDriven/GPUDriven.h"

class FRenderer
{
public:
	FRenderer(){}
	~FRenderer(){}

	void CreateRHIContext(int InWidth, int Inheight);
	void DestroyRHIContext();

	void RenderScene();

	void UpdateSceneConstantsBuffer(FSceneConstant* InSceneConstant);
	void UpdateActorConstantBuffer(std::string ActorName, FObjectConstants* ObjConstants);

	void UpdateSkinnedMeshBoneTransform(std::string ActorName, FBoneTransforms* InBoneTrans);

	void Resize(int InWidth, int InHeight);

	//
	void CreateRenderingItem(std::vector<std::shared_ptr<AStaticMeshActor>>& StaticMeshActors);
	void CreateRenderingItem(std::vector<std::shared_ptr<ASkeletalMeshActor>>& SkeletalMeshActors);
	void CreateRenderingItem(std::vector<std::shared_ptr<AInstancedStaticMeshActor>>& InstancedActors);

	void PostProcess();

protected:
private:
	void InitializeSceneConstant();

	void CreateSceneColor();

	void RenderDepth();
	void RenderSkinnedMesh();
	void RenderInstancedMesh();

	void DrawToBackBuffer();


	void DrawRenderingMeshes(std::unordered_map<std::string, IRHIRenderingMesh*>& Items);

	IRHIContext* RHIContext;

	std::unordered_map<std::string, IRHIRenderingMesh*> RenderingMeshes;
	std::unordered_map<std::string, IRHIRenderingMesh*> SkinnedRenderingMeshes;
	std::unordered_map<std::string, IRHIRenderingMesh*> InstanceRenderingMeshes;

	//
	FViewport Viewport;

	//save camera info
	FSceneConstant SceneConstant;
	IRHIConstantBuffer<FSceneConstant>* SceneConstantBuffer;

	FShadowMap* ShadowMap;

	//scene color
	FRHIColorResource* SceneColor;
	EPixelBufferFormat SceneColorFormat;

	// on and offs
	bool ShouldRenderShadow;
	bool ShouldRenderStatic;
	bool ShouldRenderSkeletal;
	bool ShouldRenderInstanced;
	bool ShouldAutoRotateLight;
	bool EnableGPUDriven;
	//for postprocess
	bool ShouldRenderPostProcess;
	FPostProcessing* PostProcessing;

	//render resource manager
	FRenderResourceManager* ResourceManager;

	//PSO manager
	FPSOManager* PSOManager;

	//GPU driven
	FGPUDriven* GPUDriven;
};
