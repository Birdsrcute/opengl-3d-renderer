#version 330
// A fragment shader for rendering a mesh that has a texture, but no lighting.
layout (location=0) out vec4 FragColor;

// Input from vertices: interpolated texture coordinate.
in vec2 TexCoord;

// Uniform from application: the texture sampler.
uniform sampler2D baseTexture;

void main() {

    // arrange coordinates so (0,0) is the center of square
    vec2 p = TexCoord - vec2(0.5);
    p.x *= 16.0 / 9.0;  // make it look rounder in widescreen aspect ratio

    float r = length(p);    // distance from the center/origin

    float radius = 0.48;

    if (r > radius) discard;    // throw away fragment bigger than radius
    
    FragColor = texture(baseTexture, TexCoord);
}