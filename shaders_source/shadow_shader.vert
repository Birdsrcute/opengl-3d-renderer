#version 330
// A vertex shader for rendering the scene from the light's POV
layout (location=0) in vec3 vPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    // compute fragment position in light space.
    gl_Position = lightSpaceMatrix * model * vec4(vPosition, 1.0);

}