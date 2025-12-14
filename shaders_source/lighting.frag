#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragWorldPos;

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

uniform vec3 attenuationValues;

// The mesh's base (diffuse) texture.
uniform sampler2D baseTexture;

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

void main() {
    // TODO: using the lecture notes, compute ambientIntensity, diffuseIntensity, 
    // and specularIntensity.

    float distance = length(lightPosition - FragWorldPos);
    float attenuation = 1.0 / (attenuationValues.x + attenuationValues.y *
        distance + attenuationValues.z * (distance * distance));

    // ambient component 
    vec3 ambientIntensity = material.x * ambientColor;

   // Compute diffuse intensities (point)
    vec3 pointDiffuseIntensity = vec3(0);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragWorldPos);

    float lambertFactor = dot(norm, normalize(lightDir));
    if (lambertFactor > 0) {
        pointDiffuseIntensity = material.y * directionalColor * lambertFactor;
    }

    // compute specular intensities (point)
    vec3 pointSpecularIntensity = vec3(0);
    if (lambertFactor > 0) {
        vec3 eyeDir = normalize(cameraPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-lightDir, norm));
        float spec = dot(reflectDir, eyeDir);
        if (spec > 0) {
            pointSpecularIntensity = material.z * directionalColor * pow(spec, material.w);
        }
    }

    pointDiffuseIntensity  *= attenuation;
    pointSpecularIntensity *= attenuation;

    vec3 pointLightIntensity = ambientIntensity + pointDiffuseIntensity + pointSpecularIntensity;

   // Compute diffuse intensities (directional)
    vec3 diffuseIntensity = vec3(0);
    if (lambertFactor > 0) {
        diffuseIntensity = material.y * directionalColor * lambertFactor;
    }

    // compute specular intensities (directional)
    vec3 specularIntensity = vec3(0);
    lightDir = -directionalLight; 
    lambertFactor = max(dot(norm, lightDir), 0.0);
    if (lambertFactor > 0) {
        vec3 eyeDir = normalize(cameraPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-lightDir, norm));
        float spec = dot(reflectDir, eyeDir);
        if (spec > 0) {
            specularIntensity = material.z * directionalColor * pow(spec, material.w);
        }
    }

    

    vec3 dirLightIntensity = ambientIntensity + diffuseIntensity + specularIntensity;
    
    vec3 lightIntensity = pointLightIntensity + dirLightIntensity;
    
    FragColor = vec4(lightIntensity, 1) * texture(baseTexture, TexCoord);
}