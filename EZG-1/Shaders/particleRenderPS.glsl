#version 330 core

in GS_Out{
    vec4 fColor;
    vec2 texCoord;
} gs_in;

out vec4 FragColor;

void main()
{
    FragColor = vec4(gs_in.fColor);
}