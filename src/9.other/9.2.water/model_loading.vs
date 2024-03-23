#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clipPlane;

void main()
{
    TexCoords = aTexCoords;    
    gl_ClipDistance[0] = dot(clipPlane, model*vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}