#version 330
// A fragment shader for rendering a mesh that has a texture, but no lighting.
layout (location=0) out vec4 FragColor;

// Input from vertices: interpolated texture coordinate.
in vec2 TexCoord;

// Uniform from application: the texture sampler.
uniform sampler2D baseTexture;

void main() {

    vec2 p = TexCoord - vec2(0.5);
    p.x *= 16.0 / 9.0;

    float r = length(p);

    float radius = 0.48;

    if (r > radius) discard;
    
    FragColor = texture(baseTexture, TexCoord);
}