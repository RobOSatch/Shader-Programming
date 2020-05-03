#version 410 core
layout (location = 0) in vec2 aPos;

out int instanceID;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 densityTextureDimensions;

vec3 wsVoxelSize = vec3(2.0/95.0);

out vec2 texCoords;

out VS_OUT {
    vec3 wsCoord;
    vec3 uvw;
    vec4 f0123;
    vec4 f4567;
    uint mc_case;
} vs_out;

uniform sampler3D densityTexture;
uniform int cameraSector;


void main()
{
    vs_out.wsCoord = vec3(aPos.x, aPos.y, cameraSector * (densityTextureDimensions.z - 1) + gl_InstanceID);
    vs_out.uvw = vec3(aPos.x / densityTextureDimensions.x, aPos.y / densityTextureDimensions.y,  gl_InstanceID / densityTextureDimensions.z - 1);

    vec3 step = vec3(1.0 / densityTextureDimensions.x, 0, 1.0 / densityTextureDimensions.z);
    
    vs_out.f0123 = vec4(texture(densityTexture, vs_out.uvw + step.yyy).x,
        texture(densityTexture, vs_out.uvw + step.yyz).x,
        texture(densityTexture, vs_out.uvw + step.xyz).x,
        texture(densityTexture, vs_out.uvw + step.xyy).x);
    vs_out.f4567 = vec4(texture(densityTexture, vs_out.uvw + step.yxy).x,
        texture(densityTexture, vs_out.uvw + step.yxz).x,
        texture(densityTexture, vs_out.uvw + step.xxz).x,
        texture(densityTexture, vs_out.uvw + step.xxy).x);

    // determine marching cubes case
    uvec4 n0123 = uvec4(clamp(vs_out.f0123*99999, 0.0, 1.0));
    uvec4 n4567 = uvec4(clamp(vs_out.f4567*99999, 0.0, 1.0));
    vs_out.mc_case = (n0123.x) | (n0123.y << 1) | (n0123.z << 2) | (n0123.w << 3) | (n4567.x << 4) |  (n4567.y << 5) | (n4567.z << 6) | (n4567.w << 7);
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, gl_InstanceID, 1.0);
}