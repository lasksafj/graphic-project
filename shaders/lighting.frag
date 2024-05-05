#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragWorldPos;

// add: TBN
in mat3 TBN;

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

// The mesh's base (diffuse) texture.
uniform sampler2D baseTexture;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

// Material parameters for the whole mesh: k_a, k_d, k_s, shininess.
uniform vec4 material;

// Ambient light color.
uniform vec3 ambientColor;

// Direction and color of a single directional light.
uniform vec3 directionalLight; // this is the "I" vector, not the "L" vector.
uniform vec3 directionalColor;
// attenuation
uniform float light_constant;
uniform float light_linear;
uniform float light_quadratic;

// Location of the camera.
uniform vec3 viewPos;

// Location of the light source.
uniform vec3 lightPos;


// Shadow
// in vec4 FragPosLightSpace;
uniform samplerCube depthMap;
uniform float far_plane;
// uniform bool shadows;


// check exist of normal map and specular map
uniform bool hasNormalMap;
uniform bool hasSpecularMap;
uniform bool hasDirectionalLight;


float ShadowCalculation(vec3 normal, vec3 lightDirection) {
    // Shadow value
	float shadow = 0.0f;
	vec3 fragToLight = FragWorldPos - lightPos;
	float currentDepth = length(fragToLight);
	float bias = max(0.5f * (1.0f - dot(normal, lightDirection)), 0.0005f); 

	// Not really a radius, more like half the width of a square
	int sampleRadius = 2;
	float offset = 0.02f;
	for(int z = -sampleRadius; z <= sampleRadius; z++)
	{
		for(int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for(int x = -sampleRadius; x <= sampleRadius; x++)
		    {
		        float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z) * offset).r;
				// Remember that we divided by the far_plane?
				// Also notice how the currentDepth is not in the range [0, 1]
				closestDepth *= far_plane;
				if (currentDepth > closestDepth + bias)
					shadow += 1.0f;     
		    }    
		}
	}
	// Average shadow
	shadow /= pow((sampleRadius * 2 + 1), 3);
        
    return shadow;
}


void main() {
    // TODO: using the lecture notes, compute ambientIntensity, diffuseIntensity, 
    // and specularIntensity.

    float distance = length(lightPos - FragWorldPos);
    float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * (distance * distance));   

    // ambient
    vec3 ambientIntensity = material.x * ambientColor;

    vec3 diffuseIntensity = vec3(0);
    vec3 specularIntensity = vec3(0);

    vec3 norm = vec3(0);

    if (hasNormalMap) {
        norm = vec3(texture(normalMap, TexCoord));
        norm = normalize(norm * 2.0 - 1.0); 
        norm = normalize(TBN * norm);
    }
    else {
        norm = normalize(Normal);
    }
    // norm = normalize(Normal);

    vec3 lightDir = -directionalLight;
    if (!hasDirectionalLight)
        lightDir = normalize(lightPos - FragWorldPos);

    float lambertFactor = dot(norm, normalize(lightDir));
    if (lambertFactor > 0) {
        //diffuse
        diffuseIntensity = material.y * directionalColor * lambertFactor;

        // specular
        vec3 eyeDir = normalize(viewPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-lightDir, norm));
        float spec = dot(reflectDir, eyeDir);
        if (spec > 0) {
            if (hasSpecularMap) {
                specularIntensity = texture(specularMap, TexCoord).x * directionalColor * pow(spec, material.w);
            }
            else {
                specularIntensity = material.z * directionalColor * pow(spec, material.w);
            }
        }

    }


    // vec3 lightIntensity = ambientIntensity + diffuseIntensity + specularIntensity;
    // FragColor = vec4(lightIntensity, 1) * texture(baseTexture, TexCoord);

    float shadow = ShadowCalculation(norm, lightDir);
    // shadow = 0;
    FragColor = vec4(ambientIntensity * attenuation + (1.0 - shadow) * (diffuseIntensity + specularIntensity) * attenuation, 1) 
        * texture(baseTexture, TexCoord); 
// ----------------------------------------------------------------------------------------------------------------------------------------


    // vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // projCoords = projCoords * 0.5 + 0.5;
    // float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // float currentDepth = projCoords.z;

    // FragColor = vec4(0,0, currentDepth > closestDepth  ? 1.0 : 0.0, 1);

    // float depthValue = texture(shadowMap, TexCoord).r;
    // FragColor = vec4(vec3(depthValue), 1.0);

    // ---------------------
    // get vector between fragment position and light position
    // vec3 fragToLight = FragWorldPos - lightPos;
    // // ise the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // // now get current linear depth as the length between the fragment and light position
    // float currentDepth = length(fragToLight);
    // // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // // display closestDepth as debug (to visualize depth cubemap)
    // // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    

    // FragColor = vec4( vec3(1.0 / (light_linear * distance + light_quadratic * (distance * distance))), 1.0);    
}