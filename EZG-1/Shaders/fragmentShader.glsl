#version 440 core
layout (location = 0) out vec4 FragColor;

#define TEXTURE_SCALE 0.02

in GS_OUT{
    vec3 wsCoord;
    vec3 wsNormal;
    vec3 fColor;
} gs_in;

layout(binding = 0) uniform sampler3D densityTexture;

layout(binding = 2) uniform sampler2D diffuseX;
layout(binding = 3) uniform sampler2D diffuseY;
layout(binding = 4) uniform sampler2D diffuseZ;
layout(binding = 5) uniform sampler2D displacementX;
layout(binding = 6) uniform sampler2D displacementY;
layout(binding = 7) uniform sampler2D displacementZ;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPos;

float g_initialStepIterations = 3;
float g_refinementStepIterations = 1;
vec3 g_parallaxDepth = 300 * vec3(1, 1, 1);

vec3 AddParallax(vec2 coord, sampler2D dispMap, vec3 toEyeVec, float dispStrength)
{
    float h = 1;
    vec2 uv = coord;
    float prev_hits;
    float ddh = 1.0 / float(g_initialStepIterations);
    float dd_texels_max = float(-toEyeVec.xy * dispStrength);
    vec2 dduv_max = vec2(1.0 / 4096.0 * dd_texels_max);
    vec2 dduv = dduv_max / float(g_initialStepIterations);

    // first do some iterations to find the intersection.
    // this will just determine the displacement silhouette,
    // not the shading - we'll refine/interp this for smooth shading
    // between two of these iterations.
    prev_hits = 0;
    float hit_h = 0;
    for (int it = 0; it < g_initialStepIterations; it++)
    {
        h -= ddh;
        uv += dduv;
        float h_tex = texture(dispMap, uv).x;
        float is_first_hit = clamp((h_tex - h - prev_hits) * 4999999, 0.0, 1.0);
        hit_h += is_first_hit * h;
        prev_hits += is_first_hit;
    }

    // REFINEMENT ITERATIONS
    // NOTE: something in this chunk is causing the grey sparklies...
    h = hit_h + ddh - 0.0085; // back up to before the hit.
    // **extra offset helps alleviate grey sparklie artifacts!
    uv = coord + dduv_max * (1 - h);
    float x = 1.0 / float(g_refinementStepIterations);
    ddh *= x;
    dduv *= x;

    prev_hits = 0;
    hit_h = 0;
    for (int it2 = 0; it2 < g_refinementStepIterations; it2++)
    {
        h -= ddh;
        uv += dduv;
        float h_tex = texture(dispMap, uv).x;
        float is_first_hit = clamp((h_tex - h - prev_hits) * 4999999, 0.0, 1.0);
        hit_h += is_first_hit * h;
        prev_hits += is_first_hit;
    }

    // assuming the two points in the second iteration only covered
    // one texel, we can now interpolate between the heights at those
    // two points and get the EXACT intersection point.  
    float h1 = (hit_h - ddh);
    float h2 = (hit_h);
    float v1 = texture(dispMap, coord + dduv_max * (1 - h1)).x; // .x is like orig height, but gauss-blurred 1 pixel.  (more is bad; makes the iterations really obvious)
    float v2 = texture(dispMap, coord + dduv_max * (1 - h2)).x; // .x is like orig height, but gauss-blurred 1 pixel.  (more is bad; makes the iterations really obvious)
    // Q: WHY THE -1???
    float t_interp = clamp((v1 - h1) / (h2 + v1 - h1 - v2) - 1, 0.0, 1.0);
    hit_h = (h1 + t_interp * (h2 - h1));

    vec2 new_coord = coord + dduv_max * (1 - hit_h);
    return vec3(new_coord, hit_h);
}

void main()
{


    //normal for shading
    vec3 pixelNormalW;
    pixelNormalW = gs_in.wsNormal;

    vec3 toEyeVec = normalize((viewPos.xyz) - gs_in.wsCoord);
    //float toEyeDistance = distance(-viewPos, gs_in.wsCoord);


    //blend weights for triplanar projection
    vec3 blendWeights = abs(gs_in.wsNormal) - 0.2;
    blendWeights *= 7;
    blendWeights = pow(blendWeights, vec3(3, 3, 3));
    blendWeights = max(blendWeights, 0);
    blendWeights.y *= 0.005;
    // and so they sum to 1.0:
    blendWeights /= dot(blendWeights, vec3(1, 1, 1));


    //triplanar projection coords
    vec2 xCoord = gs_in.wsCoord.yz * TEXTURE_SCALE;
    vec2 yCoord = gs_in.wsCoord.xz * TEXTURE_SCALE;
    vec2 zCoord = gs_in.wsCoord.xy * TEXTURE_SCALE;


    //Displacement Mapping
    const float parallaxDistanceHighLOD = 2.5f; // dist @ which it's full-on 
    const float parallaxDistanceLowLOD = 5.0f; // dist @ which it's first noticeable
    float parallaxEffectLOD = 1.0f;//clamp((parallaxDistanceLowLOD - toEyeDistance) / (parallaxDistanceLowLOD - parallaxDistanceHighLOD), 0.0, 1.0);

    //parallaxEffectDetail = 1; //<----- define if you want parallax without LOD!

    if (parallaxEffectLOD > 0)
    {
        vec3 toEyeVecWithLOD = toEyeVec * parallaxEffectLOD;
        const vec3 parallax_str = abs(pixelNormalW) * g_parallaxDepth.xxx;

        if (parallax_str.x * blendWeights.x > 0)
            xCoord = AddParallax(xCoord, displacementX, toEyeVecWithLOD.yzx, parallax_str.x).xy;

        if (parallax_str.y * blendWeights.y > 0)
            yCoord = AddParallax(yCoord, displacementY, toEyeVecWithLOD.xzy, parallax_str.y).xy;

        if (parallax_str.z * blendWeights.z > 0)
            zCoord = AddParallax(zCoord, displacementZ, toEyeVecWithLOD.xyz, parallax_str.z).xy;
    }

    //blend colors from textures
    vec4 xColor = texture(diffuseX, xCoord);
    vec4 yColor = texture(diffuseY, yCoord);
    vec4 zColor = texture(diffuseZ, zCoord);

    vec4 blendedColor = xColor * blendWeights.xxxx +
        yColor * blendWeights.yyyy +
        zColor * blendWeights.zzzz;


    FragColor = vec4(blendedColor);
}