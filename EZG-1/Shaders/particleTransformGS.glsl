#version 440 core
layout(points) in;
layout(points, max_vertices = 30) out;

in VS_OUT{
    vec3 position;
    vec3 velocity;
    float lifeTime;
    float type;
} gs_in[];


out vec3 outPosition;
out vec3 outVelocity;
out float outLifeTime;
out float outType;

uniform float deltaTime;
uniform float programTime;
uniform sampler1D randomTexture;
uniform bool spawnNewEmitter;
uniform vec3 spawnPosition;

#define PRIMARY_EMITTER 0.0f                                                
#define EMITTER 1.0f                                                
#define TYPE_A 2.0f                                                    
#define TYPE_B 3.0f         

vec3 randomVec(float pos) {
    //vec3 rnd = texture(randomTexture, pos).xyz; //[0...1]
    return vec3(1, 1, 1);
}

void spawnParticle(vec3 position, int amount)
{
    outPosition = position.xyz;
    outLifeTime = 0;
    outType = TYPE_A;
    for (int i = 0; i < amount; ++i) {
        outVelocity = randomVec(((programTime + i) * gs_in[0].lifeTime / 13.f) / 1000.f) * 10;
        EmitVertex();
        EndPrimitive();
    }
}

void spawnEmitter(vec3 position, int amount)
{
    outType = EMITTER;
    for (int i = 0; i < amount; ++i) {
        outPosition = position.xyz;
        outLifeTime = outPosition.x;
        outVelocity = vec3(outPosition.y, 0, 0);
        EmitVertex();
        EndPrimitive();
    }
}

void main() {
    float lifeTime = gs_in[0].lifeTime + deltaTime;
    vec3 position = gs_in[0].position + gs_in[0].velocity * deltaTime;
    //gravity
    vec3 velocity = gs_in[0].velocity + vec3(0, 10, 0) * deltaTime;

    //used to spawn new emitters on runtime
    if (gs_in[0].type == PRIMARY_EMITTER) {
        //emit self
        outPosition = gs_in[0].position;
        outVelocity = vec3(0, 0, 0);
        outLifeTime = lifeTime;
        outType = gs_in[0].type;
        EmitVertex();
        EndPrimitive();
        if (spawnNewEmitter) {
            spawnEmitter(spawnPosition, 2);
        }
    }

    if (gs_in[0].type == EMITTER && lifeTime < 10.f) {

        //velocity gets used for a spawn timer for emitter particles
        outVelocity = gs_in[0].velocity + vec3(deltaTime, 0, 0);
        if (gs_in[0].velocity.x > 0.02f) {
            spawnParticle(gs_in[0].position, 20);

            outVelocity = vec3(0, 0, 0);
        }

        //emit self
        outPosition = gs_in[0].position;
        outLifeTime = lifeTime;
        outType = gs_in[0].type;
        EmitVertex();
        EndPrimitive();
    }

    if (gs_in[0].type == TYPE_A && gs_in[0].lifeTime < 1.0f) {
        //emit self
        outPosition = position;
        outVelocity = velocity;
        outLifeTime = lifeTime;
        outType = gs_in[0].type;
        EmitVertex();
        EndPrimitive();
    }

}