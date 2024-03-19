#version 400 core
layout (location = 0 ) in vec2 position;
layout (location = 1 ) in vec2 texCoord;

out vec2 uv;
flat out uint drawID;//flatΪ�˽�ֹ�������Բ�ֵ

void main(void)
{
  uv = texCoord;
  drawID = gl_InstanceID;
  gl_Position = vec4(position,0.0,1.0);
}