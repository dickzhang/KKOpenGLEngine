#version 330 core

 in vec3 nearPoint;
 in vec3 farPoint;
 in vec3 eyePos;
 in mat4 fragView;
 in mat4 fragProj;

uniform float near;
uniform float far;
uniform float scale1;
uniform float scale2;

out vec4 FragColor;

vec4 grid(vec3 fragPos3D, float scale)
{
    vec2 coord = fragPos3D.xy * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    
    vec4 color = vec4(0.2,0.2,0.2,1.0 - min(line, 1.0));
    if(fragPos3D.x > -10 * minimumx && fragPos3D.x < 10 * minimumx)
    {
        color.z = 1.0;
    }
    if(fragPos3D.y > -10 * minimumz && fragPos3D.y < 10 * minimumz)
    {
        color.x = 1.0;
    }
    return color;
}

float computeLinearDepth(vec3 pos) 
{
    vec4 clip_space_pos = fragProj * fragView * vec4(pos.xzy, 1.0);
    // put back between -1 and 1
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    // get linear value between 0.01 and 100
    float lDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return lDepth / far; // normalize
}

void main()
{
    // our grid should be on the XY plane so check fo z
    float t = -eyePos.z / (farPoint.z - eyePos.z);
   
    vec3 fragPos = eyePos + t * (farPoint - eyePos);
    
    float linearDepth = computeLinearDepth(fragPos);
    
    // fade at the horizon  
    vec3 eyeVec = normalize(eyePos - fragPos);
    float fading = min(1.0, abs(eyeVec.z) * 15.0);
    
    FragColor = 
            (grid(fragPos, scale1) + grid(fragPos, scale2)) *    // multi resolution
            vec4(1.0, 1.0, 1.0, max(0.0, fading - float(t<0.0)));// hide the grid where needed 
    
}