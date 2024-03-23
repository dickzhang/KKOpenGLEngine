#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 vp;
uniform vec3 cameraPos;
out vec2 _uv;
out float scale;

void main()
{
    scale=length(cameraPos)+100;
    vec3 position = aPos *scale;
    gl_Position = vp * vec4(position, 1.0);
    _uv = position.xz;
}