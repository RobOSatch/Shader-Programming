#version 410 core
layout (location = 0) out vec4 FragColor;

uniform float lr;
in vec2 texCoords;
in vec3 fColor;
uniform sampler3D densityTexture;

void main()
{
    //FragColor = vec4(texture(densityTexture, vec3(texCoords.xy,lr).xyz).rrr, 1.0);
    FragColor = vec4(fColor, 1.0);
}  