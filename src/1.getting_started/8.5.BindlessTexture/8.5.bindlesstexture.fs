#version 450 core
#extension GL_ARB_bindless_texture : require

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main()
{
    vec4 image0 = texture(tex0, TexCoord);
    vec4 image1 = texture(tex1, TexCoord);
    FragColor = image0;
}