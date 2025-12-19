#version 330
// A vertex shader for rendering vertices with normal vectors and texture coordinates,
// which creates outputs needed for a Phong reflection fragment shader.
layout (location=0) in vec3 vPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    // compute fragment position in light space.
    gl_Position = lightSpaceMatrix * model * vec4(vPosition, 1.0);;

}