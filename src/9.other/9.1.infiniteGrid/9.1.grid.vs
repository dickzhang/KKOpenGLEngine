#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 vp;
out vec2 _uv;

const float scale = 100.0;

void main()
{
    vec3 position =aPos * scale;
    gl_Position =  vp*vec4(position, 1.0);
    _uv = position.xz;  // limit the grid to the X-Z plane (y == 0)
}