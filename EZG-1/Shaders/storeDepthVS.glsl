#version 330 core
layout (location = 0) in vec3 aPos;

out VS_OUT
{
    vec4 FragPos;
} vs_out;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    vs_out.FragPos = gl_Position;
}