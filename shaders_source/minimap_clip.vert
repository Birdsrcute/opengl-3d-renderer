#version 330
layout (location=0) in vec3 vPosition;
layout (location=1) in vec2 vTexCoord;
layout (location=2) in vec3 vNormal;
layout (location=3) in vec3 vTangent;

out vec2 TexCoord;

void main() {

    const vec2 scale  = vec2(0.35, 0.35);   // minimap size
    const vec2 offset = vec2(-0.50, 0.50);  // top-left-ish

    float x = vPosition.x * scale.x + offset.x;
    float y = vPosition.y * scale.y + offset.y;

    // assume its local space == clip space.
    gl_Position = vec4(x, y, vPosition.z, 1.0);

    TexCoord = vTexCoord;
}