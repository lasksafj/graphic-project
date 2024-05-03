#version 330
// A vertex shader for rendering vertices with normal vectors and texture coordinates,
// which creates outputs needed for a Phong reflection fragment shader.
layout (location=0) in vec3 vPosition;
layout (location=1) in vec3 vNormal;
layout (location=2) in vec2 vTexCoord;

layout (location = 3) in vec3 vTangent;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragWorldPos;

// add: TBN
out mat3 TBN;

void main() {
    // Transform the position to clip space.
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
    TexCoord = vTexCoord;
    Normal = mat3(transpose(inverse(model))) * vNormal;
    
    // TODO: transform the vertex position into world space, and assign it 
    // to FragWorldPos.
    FragWorldPos = vec3(model * vec4(vPosition, 1.0));


    // add: TBN
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 N = normalize(normalMatrix * vNormal);
    vec3 T = normalize(normalMatrix * vTangent);
    vec3 B = normalize(cross(N, T));
    TBN = mat3(T, B, N);

}