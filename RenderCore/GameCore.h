#pragma once

#include <intsafe.h>
#include "MeshActor.h"

#define  AssetPath L"Models/ModelSave.Bin"


class FGameCore
{
public:
	FGameCore(){}
	~FGameCore(){}

	virtual void Initialize();
	virtual void ShutDown(){ Geometries.empty();}

	virtual void Tick();

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);

	std::vector<std::unique_ptr<AMeshActor>>& GetGeometries() {
		return Geometries;}
private:
	void LoadAssets();

	std::vector<std::unique_ptr<AMeshActor>> Geometries;

};

