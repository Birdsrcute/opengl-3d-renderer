#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in mat3 TBN;
in vec3 FragWorldPos;
in vec3 Normal;

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

uniform vec3 attenuationValues;

// The mesh's base (diffuse) texture.
uniform sampler2D baseTexture;

// the mesh's normal map
uniform sampler2D normalMap;

// Material parameters for the whole mesh: k_a, k_d, k_s, shininess.
uniform vec4 material;

// Ambient light color.
uniform vec3 ambientColor;

// Direction and color of a single directional light.
uniform vec3 directionalLight; // this is the "I" vector, not the "L" vector.
uniform vec3 directionalColor;

// position of a spot light
uniform vec3 lightPosition;

// Location of the camera.
uniform vec3 cameraPos;

// does the fragment have a normal map
uniform bool hasNormalMap;

void main() {
    
    vec3 RealNormal;

    // getting world normal
    if (hasNormalMap) {
        vec3 n = texture(normalMap, TexCoord).rgb;
        n = n * 2.0 - 1.0;
        n.y *= -1.0;
        RealNormal = normalize(TBN * n);
    }
    else {
        RealNormal = normalize(Normal);
    }

    // TODO: using the lecture notes, compute ambientIntensity, diffuseIntensity, 
    // and specularIntensity.

    float distance = length(lightPosition - FragWorldPos);
    float attenuation = 1.0 / (attenuationValues.x + attenuationValues.y *
        distance + attenuationValues.z * (distance * distance));

    // ambient component 
    vec3 ambientIntensity = material.x * ambientColor;

   // Compute diffuse intensities (point)
    vec3 pointDiffuseIntensity = vec3(0);
    vec3 pointLightDir = normalize(lightPosition - FragWorldPos);

    // light direction changes w/ point & directional light
    float pointLambertFactor = dot(RealNormal, normalize(pointLightDir));
    if (pointLambertFactor > 0) {
        pointDiffuseIntensity = material.y * directionalColor * pointLambertFactor;
    }

    // compute specular intensities (point)
    vec3 pointSpecularIntensity = vec3(0);
    if (pointLambertFactor > 0) {
        vec3 eyeDir = normalize(cameraPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-pointLightDir, RealNormal));
        float spec = dot(reflectDir, eyeDir);
        if (spec > 0) {
            pointSpecularIntensity = material.z * directionalColor * pow(spec, material.w);
        }
    }


   // Compute diffuse intensities (directional)
    vec3 dirLightDir = normalize(-directionalLight); 
    vec3 diffuseIntensity = vec3(0);
    float dirLambertFactor = dot(RealNormal, normalize(dirLightDir));
    if (dirLambertFactor > 0) {
        diffuseIntensity = material.y * directionalColor * dirLambertFactor;
    }

    // compute specular intensities (directional)
    vec3 specularIntensity = vec3(0);
    
    dirLambertFactor = max(dot(RealNormal, dirLightDir), 0.0);
    if (dirLambertFactor > 0) {
        vec3 eyeDir = normalize(cameraPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-dirLightDir, RealNormal));
        float spec = dot(reflectDir, eyeDir);
        if (spec > 0) {
            specularIntensity = material.z * directionalColor * pow(spec, material.w);
        }
    }

    vec3 pointLightIntensity = attenuation * (pointDiffuseIntensity + pointSpecularIntensity);
    vec3 dirLightIntensity = diffuseIntensity + specularIntensity;
    
    vec3 lightIntensity = ambientIntensity + pointLightIntensity + dirLightIntensity;
    
    FragColor = vec4(lightIntensity, 1) * texture(baseTexture, TexCoord);
}