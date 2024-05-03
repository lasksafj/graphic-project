#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;
layout(location = 4) in ivec4 boneIds; 
layout(location = 5) in vec4 weights;
	

uniform mat4 model;
// uniform mat4 lightSpaceMatrix;
uniform bool skeletal;

	
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

	
void main()
{
    if (!skeletal) {
        gl_Position = model * vec4(vPosition, 1.0);
    }
    else {
        mat4 boneTransform = finalBonesMatrices[boneIds[0]] * weights[0];
        boneTransform += finalBonesMatrices[boneIds[1]] * weights[1];
        boneTransform += finalBonesMatrices[boneIds[2]] * weights[2];
        boneTransform += finalBonesMatrices[boneIds[3]] * weights[3];

        gl_Position = model * boneTransform * vec4(vPosition, 1.0);
    }
}