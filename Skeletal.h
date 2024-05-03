#pragma once
#include "BoneInfo.h"
#include "SkeletalObject.h"
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <unordered_map>
#include <algorithm>

class Skeletal
{
public:
	Skeletal(const std::string& path, bool flipTextureCoords);


	SkeletalObject& getRoot() { return m_root; }


	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }


private:
	SkeletalObject m_root;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;


	SkeletalObject s_assimpLoad(const std::string& path, bool flipTextureCoords);

	SkeletalObject s_processAssimpNode(aiNode* node, const aiScene* scene,
		const std::filesystem::path& modelPath,
		std::unordered_map<std::filesystem::path, Texture>& loadedTextures);

	SkeletalMesh s_fromAssimpMesh(const aiMesh* mesh, const aiScene* scene, const std::filesystem::path& modelPath,
		std::unordered_map<std::filesystem::path, Texture>& loadedTextures);

	void ExtractBoneWeightForVertices(std::vector<SkeletalVertex>& vertices, const aiMesh* mesh, const aiScene* scene);

};
