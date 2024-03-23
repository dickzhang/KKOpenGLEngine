#version 330 core

const float scale = 100.0;
const float lod_floor = 8.0;  // minimum number of pixels between lines before LOD could switch
const vec4 x_axis_color = vec4(220, 20, 60, 255) / 255.0;
const vec4 z_axis_color = vec4(0, 46, 255, 255) / 255.0;

uniform float cell_size = 1.0;
uniform vec4 thin_line_color = vec4(vec3(1.0,0.0,0.0), 1.0);
uniform vec4 wide_line_color = vec4(vec3(0.0,1.0,1.0), 1.0);  // every 10th line is thick

#define INV_LN10 0.434294481903252  // 1 over ln10
#define clamp01(x) clamp(x, 0.0, 1.0)

float log10(float x) 
{ 
    return log(x) * INV_LN10;
}

float max2(const vec2 v)
{
    return max(v.x, v.y);
}

vec2 step3(const vec2 a, const vec2 x, const vec2 b)
{ 
    return step(a, x) - step(b, x); 
}

in vec2 _uv;

layout(location = 0) out vec4 color;

void main() {
    // higher derivative = farther cell = smaller LOD = less details = more transparent
    vec2 derivative = fwidth(_uv);
    float lod = max(0.0, log10(length(derivative) * lod_floor / cell_size) + 1.0);
    float fade = fract(lod);

    // cell size at LOD level 0, 1 and 2, each higher level is 10 times larger
    float cell_size_0 = cell_size * pow(10.0, floor(lod));
    float cell_size_1 = cell_size_0 * 10.0;
    float cell_size_2 = cell_size_1 * 10.0;

    derivative *= 4.0;  // each anti-aliased line covers up to 4 pixels

    // compute absolute distance to cell line centers for each LOD and pick max x/y to be the alpha
    // alpha_0 >= alpha_1 >= alpha_2
    float alpha_0 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_0) / derivative) * 2.0 - 1.0));
    float alpha_1 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_1) / derivative) * 2.0 - 1.0));
    float alpha_2 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_2) / derivative) * 2.0 - 1.0));

    // line margins can be used to check where the current line is (e.g. x = 0, or y = 3, etc)
    vec2 margin = min(derivative, 1.0);
    vec2 basis = step3(vec2(0.0), _uv, margin);

    // blend between falloff colors to handle LOD transition and highlight world axis X and Z
    vec4 c = alpha_2 > 0.0
        ? (basis.y > 0.0 ? x_axis_color : (basis.x > 0.0 ? z_axis_color : wide_line_color))
        : (alpha_1 > 0.0 ? mix(wide_line_color, thin_line_color, fade) : thin_line_color);

    // calculate opacity falloff based on distance to grid extents
    float opacity_falloff = 1.0 - clamp01(length(_uv) / scale);

    // blend between LOD level alphas and scale with opacity falloff
    c.a *= (alpha_2 > 0.0 ? alpha_2 : alpha_1 > 0.0 ? alpha_1 : (alpha_0 * (1.0 - fade))) * opacity_falloff;
    color = c;
};