#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 vpIn;
out vec3 nearPoint;
out vec3 farPoint;

vec3 UnprojectPoint(vec2 pos, float z)
{
    vec4 unprojectedPoint = vpIn * vec4(pos.xy, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    gl_Position = vec4(aPos, 1.0); // using directly the clipped coordinates
    nearPoint = UnprojectPoint(gl_Position.xy, 0.0).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(gl_Position.xy, 1.0).xyz; // unprojecting on the far plane
}