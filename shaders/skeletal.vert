#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;
layout(location = 4) in ivec4 boneIds; 
layout(location = 5) in vec4 weights;
	
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragWorldPos;

out mat3 TBN;


// skeletal animation
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool skeletal;


// shadow
// uniform mat4 lightSpaceMatrix;
// out vec4 FragPosLightSpace;

	
void main()
{
    // vec4 totalPosition = vec4(vPosition, 1.0);
    // for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    // {
    //     if(boneIds[i] == -1) 
    //         continue;
    //     if(boneIds[i] >=MAX_BONES) 
    //     {
    //         totalPosition = vec4(vPosition,1.0f);
    //         break;
    //     }
    //     vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vPosition,1.0f);
    //     totalPosition += localPosition * weights[i];
    //     // vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * vNormal;
    // }
    vec4 totalPosition;
    if (!skeletal) {
        totalPosition = vec4(vPosition, 1.0);
    }
    else {
        mat4 boneTransform = finalBonesMatrices[boneIds[0]] * weights[0];
        boneTransform += finalBonesMatrices[boneIds[1]] * weights[1];
        boneTransform += finalBonesMatrices[boneIds[2]] * weights[2];
        boneTransform += finalBonesMatrices[boneIds[3]] * weights[3];

        totalPosition = boneTransform * vec4(vPosition, 1.0);
    }
		
    gl_Position =  projection * view * model * totalPosition;
    TexCoord = vTexCoord;

    
    
    Normal = mat3(transpose(inverse(model))) * vNormal;

    FragWorldPos = vec3(model * totalPosition);
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 N = normalize(normalMatrix * vNormal);
    vec3 T = normalize(normalMatrix * vTangent);
    vec3 B = normalize(cross(N, T));
    TBN = mat3(T, B, N);


    // FragPosLightSpace = lightSpaceMatrix * totalPosition;

}