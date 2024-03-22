#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 proj;
uniform mat4 view;
uniform vec3 eyeWorldPosition;

 out vec3 nearPoint;
 out vec3 farPoint;
 out vec3 eyePos;
 out mat4 fragView;
 out mat4 fragProj;

vec3 unprojectPoint(float x, float y, float z, mat4 mv, mat4 projection)
{
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    fragView = view;
    fragProj = proj;
    eyePos   = eyeWorldPosition;
    gl_Position =vec4(aPos,1.0);
    nearPoint = unprojectPoint(gl_Position.x, gl_Position.y, -1, view, proj);
    farPoint  = unprojectPoint(gl_Position.x, gl_Position.y, 1, view, proj);
}