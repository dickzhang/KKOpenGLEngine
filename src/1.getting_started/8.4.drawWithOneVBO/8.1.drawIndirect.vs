#version 400 core
layout (location = 0 ) in vec2 position;
layout (location = 1 ) in vec2 texCoord;
layout (location = 2 ) in uint drawid;
layout (location = 3 ) in mat4 instanceMatrix;

out vec2 uv;
flat out uint drawID;//flat为了禁止进行线性插值
layout(std140) uniform UBOInfo  
{  
   bool testBind;
   mat4 testMatrix;
   vec4 testVec4;
   vec3 testVec3[5];
}uboInfo;  
void main(void)
{
  uv = texCoord;
  if(uboInfo.testBind)
  {
	drawID = gl_InstanceID;
	gl_Position = instanceMatrix * vec4(position,0.0,1.0);
  }
  else
  {
	gl_Position = instanceMatrix * vec4(position,1.0,1.0);
  }
  
  
}