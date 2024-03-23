#version 330 core                                                                     

layout (location = 0) in vec3 aPos;                                           
layout (location = 1) in vec3 aNor;   
layout (location = 2) in vec2 aTex;

uniform mat4 modelMatrix;
uniform mat4 gVP;
uniform float textureTiling;
uniform vec3 cameraPos;
uniform vec3 lightPos;

out DATA {
	vec4 worldPosition;
	vec2 textureCoord;
	vec3 normal;
	vec3 toCamera;
	vec3 fromLight;
} Out;

void main(){
	vec3 worldPosition =vec3(modelMatrix*vec4(aPos,1.0));
	Out.worldPosition = gVP * modelMatrix*vec4(aPos,1.0);
	Out.textureCoord = aTex * textureTiling;
	Out.normal = aNor;
	Out.toCamera = cameraPos - worldPosition;
	Out.fromLight =worldPosition - lightPos;
	gl_Position = Out.worldPosition;
}