// #version 330 core
// layout (location = 0) in vec3 aPos;

// out vec3 TexCoords;

// uniform mat4 projection;
// uniform mat4 view;

// void main()
// {
//     vec4 pos = projection * view * vec4(aPos, 1.0f);
//     // Having z equal w will always result in a depth of 1.0f
//     gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
//     // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
//     TexCoords = vec3(aPos.x, aPos.y, -aPos.z);
// }   

#version 330 core
layout (location = 0) in vec3 vPosition;

uniform mat4 model;

out vec3 TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoord = vPosition;
    vec4 pos = projection * view * model * vec4(vPosition, 1.0);
    gl_Position = pos.xyww;
}