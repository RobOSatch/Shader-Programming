#version 430 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VS_OUT{
	vec3 velocity;
	float type;
} gs_in[];

out GS_Out{
	// Stream out to a VB & save for reuse!
	// .xyz = wsCoord, .w = occlusion
		vec4 fColor;
		vec2 texCoord;
} gs_out;

float size = 3.f;

void buildParticle(vec4 position)
{
	gs_out.texCoord = vec2(0.0, 0.0);
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0) * size;    // 1:bottom-left
	EmitVertex();
	gs_out.texCoord = vec2(1.0, 0.0);
	gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0) * size;    // 2:bottom-right
	EmitVertex();
	gs_out.texCoord = vec2(0.0, 1.0);
	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0) * size;    // 3:top-left
	EmitVertex();
	gs_out.texCoord = vec2(1.0, 1.0);
	gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0) * size;    // 4:top-right
	EmitVertex();
	EndPrimitive();
}

void main() {

	if (gs_in[0].type == 1.0f)
		gs_out.fColor = vec4(0, 1, 0, 1);
	if (gs_in[0].type == 2.0f)
		gs_out.fColor = vec4(1, 1, 1, 1);
	if (gs_in[0].type == 3.0f)
		gs_out.fColor = vec4(0, 0, 1, 1);

	gs_out.fColor = vec4(1, 1, 1, 1);
	//if(gs_in[0].type != 0.0f)
	buildParticle(gl_in[0].gl_Position);
}