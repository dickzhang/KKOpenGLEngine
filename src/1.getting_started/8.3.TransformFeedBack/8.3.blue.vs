#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 projection_matrix;
uniform mat4 view;

out vec4 world_space_position;
out vec3 vs_fs_normal;

void main(void)
{
    vec4 pos = model_matrix * vec4(position ,1.0);
    world_space_position = pos;
    vs_fs_normal = normalize((model_matrix * vec4(normal, 0.0)).xyz);
    gl_Position = projection_matrix *view * pos;
}