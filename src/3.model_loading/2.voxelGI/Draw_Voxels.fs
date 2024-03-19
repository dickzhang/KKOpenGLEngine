#version 430 core

in vec4 voxelColor;
out vec4 fragColor;

void main()
{
    fragColor = voxelColor;
}