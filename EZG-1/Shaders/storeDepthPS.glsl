#version 330 core
layout (location = 0) out vec3 color;

in VS_OUT
{
    vec4 FragPos;
} fs_in;

void main()
{
    float depth = fs_in.FragPos.z / fs_in.FragPos.w;
    depth = depth * 0.5 + 0.5;

    float moment1 = depth;
    float moment2 = depth * depth;

    // Adjust moments using derivative
    float dx = dFdx(depth);
    float dy = dFdy(depth);
    moment2 += 0.25 * (dx * dx + dy * dy);

    // Debug shit
    color = vec3(moment1, moment2, 0.0);
    //FragColor = vec4(moment1, moment2, 0.0, 0.0);
}
