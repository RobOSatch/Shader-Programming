#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D displacementMap;
uniform sampler2D normalMap;

uniform float normalLevel;
uniform float heightScale;
uniform int primaryLayers;
uniform int secondaryLayers;

vec2 ParallaxMapping(sampler2D depthMap, vec2 texCoords, vec3 viewDir)
{
    // calculate the size of each primary layer
    float primaryLayerDepth = 1.0 / primaryLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / primaryLayers;

    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of Pr
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += primaryLayerDepth;
    }

    // go back one primary step
    currentLayerDepth -= primaryLayerDepth;
    currentTexCoords += deltaTexCoords;
    currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    // secondary layers
    float secondaryLayerDepth = 1.0 / (primaryLayers * secondaryLayers);
    deltaTexCoords = P / (primaryLayers * secondaryLayers);
    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of Pr
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += secondaryLayerDepth;
    }

    return currentTexCoords;
}


void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    texCoords = ParallaxMapping(displacementMap, fs_in.TexCoords, viewDir);
    if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    vec3 color = texture(diffuseMap, texCoords).rgb;

    // ambient
    vec3 ambient = 0.3 * color;
    
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal.xy *= normalLevel;
    normal = normalize(normal);

    // lighting
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}