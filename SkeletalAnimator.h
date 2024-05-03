#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "SkeletalAnimation.h"
#include "Bone.h"

class SkeletalAnimator
{
public:
	SkeletalAnimator(SkeletalAnimation* animation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

		m_GlobalInverseTransform = inverse(m_CurrentAnimation->GetRootNode().transformation);
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(SkeletalAnimation* pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap.at(nodeName).id;
			m_FinalBoneMatrices[index] = m_GlobalInverseTransform * globalTransformation * boneInfoMap.at(nodeName).offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	void resetAnimation() {
		m_CurrentTime = 0.0f;
		for (int i = 0; i < m_FinalBoneMatrices.size(); i++)
			m_FinalBoneMatrices[i] = glm::mat4(1.0);
	}

private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	SkeletalAnimation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;


	glm::mat4 m_GlobalInverseTransform;
};