#version 330
// A fragment shader for rendering fragments in the Phong reflection model.
layout (location=0) out vec4 FragColor;

// Inputs: the texture coordinates, world-space normal, and world-space position
// of this fragment, interpolated between its vertices.
in vec2 TexCoord;
in mat3 TBN;
in vec3 FragWorldPos;   
in vec3 Normal;
in vec4 FragLightPos; 

// Uniforms: MUST BE PROVIDED BY THE APPLICATION.

uniform vec3 attenuationValues;

// The mesh's base (diffuse) texture.
uniform sampler2D baseTexture;

// shadow map
uniform sampler2D shadowMap;

uniform sampler2D specularMap;
uniform bool hasSpecularMap;

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

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 1. Perspective divide: from clip space to NDC (-1 to 1)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // 2. Transform from NDC (-1..1)  texture coords (0..1)
    projCoords = projCoords * 0.5 + 0.5;

    // 3. Get depth of current fragment from light's POV
    float currentDepth = projCoords.z;

    // 4. Sample closest depth from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // 5. Shadow test
    float bias = 0.005;  // prevents shadow acne
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // 6. If outside the shadow map, don't shadow
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main() {

    float bands = 3.0;
    
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
    float pointLambertFactorRaw = max(dot(RealNormal, pointLightDir), 0.0);
    float pointLambertFactorToon = floor(pointLambertFactorRaw * bands) / bands;
    if (pointLambertFactorRaw > 0) {
        pointDiffuseIntensity = material.y * directionalColor * 2.0f * pointLambertFactorToon;
    }

    // compute specular intensities (point)
    vec3 pointSpecularIntensity = vec3(0);
    if (pointLambertFactorRaw > 0) {
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
    float dirLambertRaw = max(dot(RealNormal, normalize(dirLightDir)), 0.0);
    float dirLambertToon = floor(dirLambertRaw * bands) / bands;
    diffuseIntensity = material.y * directionalColor * dirLambertToon;

    // compute specular intensities (directional)
    vec3 specularIntensity = vec3(0);
    
    float dirLambertFactor = max(dot(RealNormal, dirLightDir), 0.0);
    if (dirLambertFactor > 0) {
        vec3 eyeDir = normalize(cameraPos - FragWorldPos);
        vec3 reflectDir = normalize(reflect(-dirLightDir, RealNormal));
        float spec = dot(reflectDir, eyeDir);
        
        float specMask = 1.0;
        if (hasSpecularMap) {
            specMask = texture(specularMap, TexCoord).r;  // use red channel
        }
        
        if (spec > 0.0) {
            specularIntensity = material.z * directionalColor * pow(spec, material.w) * specMask;
        }
    }

    vec3 pointLightIntensity = attenuation * (pointDiffuseIntensity + pointSpecularIntensity);
    vec3 dirLightIntensity = diffuseIntensity + specularIntensity;
    
    vec3 lightIntensity = ambientIntensity + pointLightIntensity + dirLightIntensity;
    
   float shadow = ShadowCalculation(FragLightPos);

    vec3 lighting =
        ambientIntensity +
        pointLightIntensity +                           // point light NOT shadowed by this map
        (1.0 - shadow) * dirLightIntensity;             // directional light shadowed

FragColor = vec4(lighting, 1.0) * texture(baseTexture, TexCoord);

}