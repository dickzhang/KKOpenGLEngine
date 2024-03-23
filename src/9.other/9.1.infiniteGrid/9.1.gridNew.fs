#version 330 core
#define TILE_SIZE 1.0
#define LINE_WIDTH 1.0
#define ANTIALIASED_LINES 0

uniform float near; //0.01
uniform float far; //100
uniform mat4 vp;

in vec3 nearPoint;
in vec3 farPoint;
in vec2 texcoord;
out vec4 FragColor;

vec4 grid(vec3 fragPos3D, float scale) 
{
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);

    vec4 color = vec4(0.2, 0.2, 0.2,1.0)*(1.0 - min(line, 1.0));

    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);
   
    // z axis
    if(abs(fragPos3D.x) < 0.1 * minimumx)
    {
          color.z = 1.0;
    }
    // x axis
    if(abs(fragPos3D.z) < 0.1 * minimumz)
    {
         color.x = 1.0;
    }

    return color;
}

float computeDepth(vec3 pos) 
{
    vec4 clip_space_pos = vp * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

float computeLinearDepth(vec3 pos)
{
    vec4 clip_space_pos = vp * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far; // normalize
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
      vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
      gl_FragDepth = computeDepth(fragPos3D);
      float linearDepth = computeLinearDepth(fragPos3D); 
      float fading = max(0, (0.5 - linearDepth));
      FragColor = (grid(fragPos3D, 10) + grid(fragPos3D, 1))* float(t > 0); // adding multiple resolution for the grid
      FragColor.a *= fading;
}