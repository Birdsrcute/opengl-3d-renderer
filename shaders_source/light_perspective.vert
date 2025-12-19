#version 330
// A vertex shader for rendering vertices with normal vectors and texture coordinates,
// which creates outputs needed for a Phong reflection fragment shader.
layout (location=0) in vec3 vPosition;
layout (location=1) in vec2 vTexCoord;
layout (location=2) in vec3 vNormal;
layout (location=3) in vec3 vTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out vec2 TexCoord;
out mat3 TBN;           // need to pass to fragment shader to sample texture
out vec3 FragWorldPos;
out vec3 Normal;
out vec4 FragLightPos;

void main() {
    // Transform the vertex position from local space to clip space.
    gl_Position = projection * view * model * vec4(vPosition, 1.0);
    // Pass along the vertex texture coordinate.
    TexCoord = vTexCoord;

    // Transform the vertex normal from local space to world space, using the Normal matrix.
    mat4 normalMatrix = transpose(inverse(model));
    Normal = normalize(mat3(normalMatrix) * vNormal);
    vec3 Tangent = normalize(mat3(normalMatrix) * vTangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    // tangent -> world space
    vec3 Bitangent = normalize(cross(Normal, Tangent));
    TBN = mat3(Tangent, Bitangent, Normal);

    FragLightPos = lightSpaceMatrix * model * vec4(vPosition, 1.0);
    
    // TODO: transform the vertex position into world space, and assign it to FragWorldPos.
    FragWorldPos = vec3(model * vec4(vPosition, 1.0));

}