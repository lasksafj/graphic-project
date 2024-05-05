#version 330 core
out vec4 FragColor;

in vec3 TexCoord;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoord);
    // can mix 2 texture or colors, to make day->night, ...
    // in float blend_factor;
    // tex1 = texture(skybox1, TexCoord);
    // tex2 = texture(skybox2, TexCoord);
    // FragColor = mix(tex1, tex2, blend_factor);
}