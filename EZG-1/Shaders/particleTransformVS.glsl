#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 velocity;
layout(location = 2) in float lifeTime;
layout(location = 3) in float type;

out VS_OUT{
    vec3 position;
    vec3 velocity;
    float lifeTime;
    float type;
} vs_out;



void main() {
    vs_out.position = position;
    vs_out.velocity = velocity;
    vs_out.lifeTime = lifeTime;
    vs_out.type = type;
}