#version 430 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) buffer SSBO_Data
{
   mat4 projection;
   mat4 view;
   vec4 colors[];
};

uniform mat4 model;
uniform int index;

out vec4 color;

void main()
{   
    color=colors[index];
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  