#version 430 core
#extension GL_ARB_shader_image_load_store : require

layout(binding = 0, rgba8) uniform readonly image3D voxelRadiance;
uniform int volumeDimension;
out vec4 albedo;

void main()
{
   float volumeDimensionF = float(volumeDimension);
	vec3 position = vec3
	(
		gl_VertexID % volumeDimension,
		(gl_VertexID / volumeDimension) % volumeDimension,
		gl_VertexID / (volumeDimension * volumeDimension)
	);
	albedo = imageLoad(voxelRadiance, ivec3(position));
	gl_Position = vec4(position, 1.0f);
}