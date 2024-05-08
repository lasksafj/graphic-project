#pragma once

#include <iostream>
#include <memory>
#include <glad/glad.h>

#include "Animator.h"
#include "ShaderProgram.h"


#include "Skeletal.h"
#include "SkeletalAnimator.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>

class TransitionSkeletal 
{
private:
	float_t m_currentTime;
	SkeletalAnimation* start_anim;
	SkeletalAnimation* end_anim;
	float start_anim_time;
	float end_anim_time;

	std::vector<glm::mat4> m_FinalBoneMatrices;
	glm::mat4 m_GlobalInverseTransform;

	float duration;

public:
	TransitionSkeletal(float duration_time) {
		m_currentTime = -1;
		duration = duration_time;

	}

	void setAnimTransforms(SkeletalAnimation* _start_anim, SkeletalAnimation* _end_anim, float _start_anim_time, float _end_anim_time) {
		start_anim = _start_anim;
		end_anim = _end_anim;

		start_anim_time = _start_anim_time;
		end_anim_time = _end_anim_time;
		
		m_GlobalInverseTransform = inverse(start_anim->GetRootNode().transformation);

		int size = _start_anim->getBonesSize();
		m_FinalBoneMatrices.resize(size);
		for (int i = 0; i < size; i++)
			m_FinalBoneMatrices[i] = glm::mat4(1.0f);
	}

	void start() {
		m_currentTime = 0;
	}

	bool finish() {
		return m_currentTime == -1;
	}

	std::vector<glm::mat4> GetFinalBoneMatrices() {
		return m_FinalBoneMatrices;
	}

	void updateAnimation(float dt) {
		m_currentTime += dt;
		if (m_currentTime < duration && m_currentTime >= 0) {
			CalculateBoneTransform(&start_anim->GetRootNode(), glm::mat4(1.0f));
		}
		else {
			m_currentTime = -1;
		}
	}

	// Skeletal Animation Blending --------------------------------------------------------------------------------------------------
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* start_bone = start_anim->FindBone(nodeName);
		Bone* end_bone = end_anim->FindBone(nodeName);

		if (start_bone && end_bone)
		{
			glm::mat4 start_translation = start_bone->InterpolatePosition(start_anim_time);
			glm::quat start_rotation = start_bone->quat_InterpolateRotation(start_anim_time);
			glm::mat4 start_scale = start_bone->InterpolateScaling(start_anim_time);

			glm::mat4 end_translation = end_bone->InterpolatePosition(end_anim_time);
			glm::quat end_rotation = end_bone->quat_InterpolateRotation(end_anim_time);
			glm::mat4 end_scale = end_bone->InterpolateScaling(end_anim_time);


			nodeTransform = glm::mix(start_translation, end_translation, m_currentTime / duration)
				* glm::toMat4(glm::slerp(start_rotation, end_rotation, m_currentTime / duration))
				* glm::mix(start_scale, end_scale, m_currentTime / duration);
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		const auto& boneInfoMap = start_anim->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap.at(nodeName).id;
			m_FinalBoneMatrices[index] = m_GlobalInverseTransform * globalTransformation * boneInfoMap.at(nodeName).offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	
};