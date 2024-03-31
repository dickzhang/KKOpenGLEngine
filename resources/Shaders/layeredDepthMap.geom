#version 450 core

// --------------------------------
// These defines come from the runtime, the comments are here to address the IDE errors
//!#define NUM_CSM_PLANES 5
// --------------------------------


layout(triangles, invocations = NUM_CSM_PLANES) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 lightSpaceMatrices[NUM_CSM_PLANES];

void main()
{          
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}  