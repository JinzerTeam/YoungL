#pragma once
#include "MeshActor.h"
#include "SkinData.h"

class ASkinMeshActor : public AMeshActor
{
public:
	ASkinMeshActor(std::string InName = "");

	virtual ~ASkinMeshActor();

	FSkinedData* GetSkinedData(){return SkinedData;}

	virtual void Tick(float DeltaTime)override;

	void SetSkinGeometry(std::unique_ptr<FGeometry<FSkinVertex>>& InGeometry) { Geometry = std::move(InGeometry); };
	FGeometry<FSkinVertex>* GetSkinGeometry() { return Geometry.get(); }

	FBoneTransforms* GetBoneTransfroms(){return BoneTransforms;}

	void TestPlayAnimation();

	void PlayAnimationClip(std::string ClipName);
	void StopAnimationClip();
private:
	FSkinedData* SkinedData;

private:
	std::unique_ptr<FGeometry<FSkinVertex>> Geometry;

	FAnimationPlayInfo* PlayInfo;
	FBoneTransforms* BoneTransforms;
};