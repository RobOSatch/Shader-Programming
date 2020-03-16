#version 440 core
layout (points) in;
layout (triangle_strip, max_vertices = 15) out;

in VS_OUT {
    vec3 wsCoord;
    vec3 uvw;
    vec4 f0123;
    vec4 f4567;
    uint mc_case;
} gs_in[]; 

vec3 wsVoxelSize= vec3(1.0/95.0, 1.0/95.0,1.0/256.0);

out vec3 fColor;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler3D densityTexture;
uniform isamplerBuffer mcTableTexture;
uniform vec3 densityTextureDimensions;

int edge_start[72] = {
			// 0
			0,0,0,
			1,0,0,
			// 1
			1,0,0,
			1,0,1,
			// 2
			1,0,1,
			0,0,1,
			// 3
			0,0,1,
			0,0,0,
			// 4
			0,1,0,  
			1,1,0,
			// 5
			1,1,0,
			1,1,1,
			// 6
			1,1,1,
			0,1,1,
			// 7
			0,1,1,
			0,1,0,
			// 8
			0,0,0,
			0,1,0,
			// 9
			1,0,0,
			1,1,0,
			// 10
			1,0,1,
			1,1,1,
			// 11
			0,0,1,
			0,1,1
		};

void buildHouse(vec4 position)
{    
    float scale = 0.5;
    vec4 worldPos = inverse(model) * inverse(view) * inverse(projection) * position;
    fColor = vec3(texture(densityTexture, vec3(worldPos.x / densityTextureDimensions.x, worldPos.z / densityTextureDimensions.z, worldPos.y / densityTextureDimensions.y)).rrr); 
    gl_Position = (position * scale + projection * view * model * vec4(-0.5 * scale, -0.5 * scale, 0.0, 0.0)); // 1:bottom-left   
    EmitVertex();   
    gl_Position = (position * scale + projection * view * model * vec4( 0.5 * scale, -0.5 * scale, 0.0, 0.0)); // 2:bottom-right
    EmitVertex();
    gl_Position = (position * scale + projection * view * model * vec4(-0.5 * scale , 0.5  * scale, 0.0, 0.0)); // 3:top-left
    EmitVertex();
    gl_Position = (position * scale + projection * view * model * vec4( 0.5 * scale, 0.5 * scale, 0.0, 0.0)); // 4:top-right
    EmitVertex();
    EndPrimitive();

}

void buildHouseS(vec4 position)
{    
    float scale = 0.5;
    vec4 worldPos = inverse(model) * inverse(view) * inverse(projection) * position;
    
    int tablePos = int(gs_in[0].mc_case * 16);
    //fColor = (texelFetch(mcTableTexture, tablePos + 1)).xxx; 
    //fColor = vec3(float(gs_in[0].mc_case) / 256.0, float(gs_in[0].mc_case) / 256.0 , float(gs_in[0].mc_case) / 256.0);
    fColor = gs_in[0].f0123.xyz;
    //fColor = vec3(texture(densityTexture, vec3(worldPos.x / 96, worldPos.y / 96, worldPos.z / 256)).rrr); 
    gl_Position = (position * scale + projection * view * model * vec4(-0.1 * scale, -0.1 * scale, 0.0, 0.0)); // 1:bottom-left   
    EmitVertex();   
    gl_Position = (position * scale + projection * view * model * vec4( 0.1 * scale, -0.1 * scale, 0.0, 0.0)); // 2:bottom-right
    EmitVertex();
    gl_Position = (position * scale + projection * view * model * vec4(-0.1 * scale, 0.1  * scale, 0.0, 0.0)); // 3:top-left
    EmitVertex();
    gl_Position = (position * scale + projection * view * model * vec4( 0.1 * scale, 0.1 * scale, 0.0, 0.0)); // 4:top-right
    EmitVertex();
    EndPrimitive();

}

void placeVertOnEdge(uint edgeNum)
{
    // Along this cell edge, where does the density value hit zero?
    // float str0= dot(cornerAmask0123[edgeNum], input.field0123) + dot(cornerAmask4567[edgeNum], input.field4567);
    // float str1= dot(cornerBmask0123[edgeNum], input.field0123) + dot(cornerBmask4567[edgeNum], input.field4567);
    // float t= saturate( str0/(str0 -str1) ); //0..1
    // use that to get wsCoordand uvwcoords

    vec3 point0 = vec3(edge_start[edgeNum * 6], edge_start[edgeNum * 6 + 1], edge_start[edgeNum * 6 + 2]);
    vec3 point1 = vec3(edge_start[edgeNum * 6 + 3], edge_start[edgeNum * 6 + 4], edge_start[edgeNum * 6 + 5]);
    vec3 pos_within_cell = mix(point0, point1, 0.5); //[0..1]

    vec3 vecWsCoord= gs_in[0].wsCoord.xyz + pos_within_cell.zyx;// * wsVoxelSize;
    gl_Position = projection * view * model * vec4(vecWsCoord, 1);
    EmitVertex();
    //float3 uvw= input.uvw + ( pos_within_cell*inv_voxelDimMinusOne).xzy;

    //GSOutputoutput;
    //output.wsCoord_Ambo.xyz= wsCoord;
    //output.wsCoord_Ambo.w= grad_ambo_tex.SampleLevel(s, uvw, 0).w;
    //output.wsNormal= ComputeNormal(tex, s, uvw);
    //return output;
}

void buildMarchingCube()
{
    
    fColor = gs_in[0].f0123.xyz;
    int tablePos = int(gs_in[0].mc_case) * 16;

    // for(int i = 0; i < 3; ++i){
    //     placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
    //     tablePos++;
    //     placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
    //     tablePos++;
    //     placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
    //     tablePos++;    
    //     EndPrimitive();
    // }

    while(texelFetch(mcTableTexture, int(tablePos)).x != -1){
        
        EndPrimitive(); 
        placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
        tablePos++;
        placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
        tablePos++;
        placeVertOnEdge(texelFetch(mcTableTexture, int(tablePos)).x);
        tablePos++;    
        EndPrimitive();        
    }
}


void main() {     
    buildMarchingCube();
}
