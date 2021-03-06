#include "pch.h"
#include "ModelLoader.h"
#include <fstream>

std::unique_ptr<FGeometry<FVertex>> FModelLoader::LoadStaticMesh(std::wstring& Path)
{
	//load asset
	std::vector<FVertex> Vertices;
	std::ifstream Fin(Path, std::ios::in | std::ios::binary);

	int VertexNum;
	Fin.read((char*)&VertexNum, sizeof(int));
	Vertices.resize(VertexNum);
	Fin.read((char*)Vertices.data(), sizeof(FVertex) * VertexNum);

	std::vector<uint32_t> Indices;
	int IndexNum;
	Fin.read((char*)&IndexNum, sizeof(int));
	Indices.resize(IndexNum);
	Fin.read((char*)Indices.data(), sizeof(int) * IndexNum);

	Fin.close();

	std::vector<uint32_t> TransIndices;
	for (int i = 0; i < Indices.size(); i++)
	{
		TransIndices.push_back(Indices[i]);
	}

	std::unique_ptr<FGeometry<FVertex>> Geo = std::make_unique<FGeometry<FVertex>>(Vertices, TransIndices);

	return std::move(Geo);
}


std::unique_ptr<FGeometry<FVertex>>  FModelLoader::BuildSkullGeometry(std::wstring& Path)
{
	//std::ifstream fin("Models/skull.txt");
	std::ifstream fin(Path);

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;


	std::vector<FVertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Position.X >> vertices[i].Position.Y >> vertices[i].Position.Z;
		fin >> vertices[i].Normal.X >> vertices[i].Normal.Y >> vertices[i].Normal.Z;

		//XMVECTOR P = XMLoadFloat3(&vertices[i].Position);

		//// Project point onto unit sphere and generate spherical texture coordinates.
		//XMFLOAT3 spherePos;
		//XMStoreFloat3(&spherePos, XMVector3Normalize(P));

		//float theta = atan2f(spherePos.z, spherePos.x);

		//// Put in [0, 2pi].
		//if (theta < 0.0f)
		//	theta += XM_2PI;

		//float phi = acosf(spherePos.y);

		//float u = theta / (2.0f * XM_PI);
		//float v = phi / XM_PI;

		//vertices[i].TexC = { u, v };

	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<uint32_t> Indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> Indices[i * 3 + 0] >> Indices[i * 3 + 1] >> Indices[i * 3 + 2];
	}

	fin.close();

	std::unique_ptr<FGeometry<FVertex>> Geo = std::make_unique<FGeometry<FVertex>>(vertices, Indices);
	return std::move(Geo);
}

void FModelLoader::ReadMaterials(std::ifstream& Fin, UINT NumMaterials, std::vector<FMaterial>& Mats)
{
	std::string Ignore;
	Mats.resize(NumMaterials);

	std::string DiffuseMapName;
	std::string NormalMapName;

	Fin >>Ignore;
	for (UINT Index = 0; Index < NumMaterials; Index++)
	{
		Fin >> Ignore >> Mats[Index].Name;
		Fin >> Ignore >> Mats[Index].DiffuseAlbedo.X >> Mats[Index].DiffuseAlbedo.Y >> Mats[Index].DiffuseAlbedo.Z;
		Fin >> Ignore >> Mats[Index].FresnelR0.X >> Mats[Index].FresnelR0.Y >> Mats[Index].FresnelR0.Z;
		Fin >> Ignore >> Mats[Index].Roughness;
		Fin >> Ignore >> Mats[Index].AlphaClip;
		Fin >> Ignore >> Mats[Index].MaterialTypeName;
		Fin >> Ignore >> Mats[Index].DiffuseMapName;
		Fin >> Ignore >> Mats[Index].NormalMapName;
	}
}

void FModelLoader::ReadSubsetTable(std::ifstream& Fin, UINT NumSubSets, std::vector<FSubset>& Subsets)
{
	std::string Ignore;
	Subsets.resize(NumSubSets);

	Fin >> Ignore;
	for (UINT Index = 0; Index < NumSubSets; Index++)
	{
		Fin >> Ignore >> Subsets[Index].Id;
		Fin >> Ignore >> Subsets[Index].VertexCount;
		Fin >> Ignore >> Subsets[Index].VertexStart;
		Fin >> Ignore >> Subsets[Index].FaceStart;
		Fin >> Ignore >> Subsets[Index].FaceCount;
	}
}

void FModelLoader::ReadSkinnedVertices(std::ifstream& Fin, UINT NumVertices, std::vector<FSkinVertex>& OutVertices)
{
	std::string Ignore;
	OutVertices.resize(NumVertices);

	Fin >> Ignore;
	for (UINT Index = 0; Index < NumVertices; Index++)
	{
		float Blah;
		Fin >> Ignore >> OutVertices[Index].Position.X >> OutVertices[Index].Position.Y >> OutVertices[Index].Position.Z;
		Fin >> Ignore >> OutVertices[Index].TangentU.X >> OutVertices[Index].TangentU.Y >> OutVertices[Index].TangentU.Z >> Blah;
		Fin >> Ignore >> OutVertices[Index].Normal.X >> OutVertices[Index].Normal.Y >> OutVertices[Index].Normal.Z;
		Fin >> Ignore >> OutVertices[Index].Uv.X >> OutVertices[Index].Uv.Y;

		Fin >> Ignore >> OutVertices[Index].BoneWights[0] >> OutVertices[Index].BoneWights[1] >>
			OutVertices[Index].BoneWights[2] >> OutVertices[Index].BoneWights[3];

		Fin >> Ignore >> OutVertices[Index].BoneIndex[0] >> OutVertices[Index].BoneIndex[1] >>
			OutVertices[Index].BoneIndex[2] >> OutVertices[Index].BoneIndex[3];

	}
}

void FModelLoader::ReadTriangles(std::ifstream& Fin, UINT NumTriangles, std::vector<uint32_t>& OutIndices)
{
	std::string Ignore;
	OutIndices.resize(NumTriangles * 3);

	Fin >> Ignore;
	for (UINT Index = 0 ; Index < NumTriangles; ++ Index)
	{
		USHORT Index0, Index1, Index2;
		Fin >> Index0 >> Index1 >> Index2;
		OutIndices[Index * 3 + 0] = Index0;
		OutIndices[Index * 3 + 1] = Index1;
		OutIndices[Index * 3 + 2] = Index2;
	}
}

void FModelLoader::ReadBoneOffsets(std::ifstream& Fin, UINT NumBones, std::vector<FMatrix>& BoneOffsets)
{
	std::string Ignore;
	BoneOffsets.resize(NumBones);

	Fin >> Ignore;
	for (UINT Index = 0; Index < NumBones; Index++)
	{
		FVector4D Row0;
		FVector4D Row1;
		FVector4D Row2;
		FVector4D Row3;

		Fin >> Ignore >>
			Row0.X >> Row0.Y >> Row0.Z >> Row0.W >>
			Row1.X >> Row1.Y >> Row1.Z >> Row1.W >>
			Row2.X >> Row2.Y >> Row2.Z >> Row2.W >>
			Row3.X >> Row3.Y >> Row3.Z >> Row3.W;

		BoneOffsets[Index].SetRow0(Row0);
		BoneOffsets[Index].SetRow1(Row1);
		BoneOffsets[Index].SetRow2(Row2);
		BoneOffsets[Index].SetRow3(Row3);
	}
}

void FModelLoader::ReadBoneHierarchy(std::ifstream& Fin, UINT NumBones, std::vector<int>& BoneIndexToParentIndex)
{
	std::string Ignore;
	BoneIndexToParentIndex.resize(NumBones);

	Fin >> Ignore;
	for (UINT Index = 0; Index < NumBones; Index++)
	{
		Fin >> Ignore >> BoneIndexToParentIndex[Index];
	}
}

void FModelLoader::ReadAnimationClips(std::ifstream& Fin, UINT NumBones, UINT NumAnimationClips, 
			std::unordered_map<std::string, FAnimationClip>& AnimationClips)
{
	std::string Ignore;
	Fin >> Ignore;
	for (UINT ClipIndex = 0; ClipIndex < NumAnimationClips; ClipIndex++)
	{
		std::string ClipName;
		Fin >> Ignore >> ClipName;
		Fin >> Ignore;

		FAnimationClip Clip;
		Clip.BoneAnimations.resize(NumBones);
		for (UINT BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			Clip.BoneAnimations[BoneIndex].BoneIndex = BoneIndex;
			ReadBoneKeyFrames(Fin, NumBones, Clip.BoneAnimations[BoneIndex]);
		}
		Fin >> Ignore;

		AnimationClips[ClipName] = Clip;
	}
}

void FModelLoader::ReadBoneKeyFrames(std::ifstream& Fin, UINT NumBones, FBoneAnimation& Animation)
{
	std::string Ignore;
	UINT NumKeyFrames = 0;

	Fin >> Ignore >> Ignore >> NumKeyFrames;
	Fin >> Ignore;

	Animation.KeyFrames.resize(NumKeyFrames);
	for (UINT FrameIndex = 0; FrameIndex < NumKeyFrames; FrameIndex++)
	{
		float T = 0.0f;

		FVector4D Translation = FVector4D(0.0f, 0.0f, 0.0f, 0.0f);
		FVector4D Scale = FVector4D(1.0f, 1.0f, 1.0f, 1.0f);
		FVector4D Rotate = FVector4D(0.0f, 0.0f, 0.0f, 1.0f);

		Fin >> Ignore >> T;
		Fin >> Ignore >> Translation.X >> Translation.Y >> Translation.Z;
		Fin >> Ignore >> Scale.X >> Scale.Y >> Scale.Z;
		Fin >> Ignore >> Rotate.X >> Rotate.Y >> Rotate.Z >> Rotate.W;

		Animation.KeyFrames[FrameIndex].TimePos = T;
		Animation.KeyFrames[FrameIndex].Translation = Translation;
		Animation.KeyFrames[FrameIndex].Scale = Scale;
		Animation.KeyFrames[FrameIndex].Rotate = Rotate;
	}

	Fin >> Ignore;
}

std::unique_ptr<FGeometry<FSkinVertex>> FModelLoader::LoadSkinedMeshAndAnimation(std::wstring& Path, FSkinedData* OutSkinData)
{
	std::ifstream Fin(Path, std::ios::in | std::ios::binary);

	UINT NumMaterials = 0;
	UINT NumVertices =0;
	UINT NumTriangles = 0;
	UINT NumBones = 0;
	UINT NumAnimationClips = 0;

	std::string Ignore;

	std::vector<FSkinVertex> Vertices;
	std::vector<uint32_t> Indices;

	if (Fin)
	{
		Fin >> Ignore;
		Fin >> Ignore >> NumMaterials;
		Fin >> Ignore >> NumVertices;
		Fin >> Ignore >> NumTriangles;
		Fin >> Ignore >> NumBones;
		Fin >> Ignore >> NumAnimationClips;

		if (NumMaterials > 0)
		{
			std::vector<FMaterial> ModelMats;
			ReadMaterials(Fin, NumMaterials, ModelMats);
			std::vector<FSubset> SubSets;
			ReadSubsetTable(Fin, NumMaterials, SubSets);
		}
		
		ReadSkinnedVertices(Fin, NumVertices, Vertices);
		
		ReadTriangles(Fin, NumTriangles, Indices);

		std::vector<FMatrix> BoneOffsets;
		ReadBoneOffsets(Fin, NumBones, BoneOffsets);

		std::vector<int> BoneHierarchy;
		ReadBoneHierarchy(Fin, NumBones, BoneHierarchy);

		std::unordered_map<std::string, FAnimationClip> AnimationClips;
		ReadAnimationClips(Fin, NumBones, NumAnimationClips, AnimationClips);

		//todo:consturct Skin data
		OutSkinData->SetData(BoneHierarchy, BoneOffsets, AnimationClips);

	}

	//construct geometry
	std::unique_ptr<FGeometry<FSkinVertex>> Geo = std::make_unique<FGeometry<FSkinVertex>>(Vertices, Indices);

	return std::move(Geo);

}