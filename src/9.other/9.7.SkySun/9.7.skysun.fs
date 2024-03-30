#version 330 core

uniform vec3 uLightDirection;
uniform vec3 uCamPosition;
uniform float uSkyIntensity;
uniform float uSunSize;
uniform vec4 uAtmospheric;
uniform vec4 uSun;
uniform vec4 uGlare1;
uniform vec4 uGlare2;
uniform vec3 ugroundColor;

in vec2 varTexCoord0;
in vec4 varColor;
in vec4 varWorldPos;
out vec4 FragColor;

const float gamma = 2.2;
const float invGamma = 1.0 / 2.2;

vec3 sRGB(vec3 color)
{
	return pow(color, vec3(gamma));
}

vec3 linearRGB(vec3 color)
{
	return pow(color, vec3(invGamma));
}

vec3 GetSkyColor(
	vec3 viewDir,
	vec3 sunDirection,
	float intensity,
	vec4 atmosphericColor,
	vec4 sunColor,
	vec4 glowColor1,
	vec4 glowColor2,
	float sunRadius)
{
	float y = 1.0 - (max(viewDir.y, 0.0) * 0.8 + 0.2) * 0.8;
	vec3 skyColor = vec3(pow(y, 2.0), y, 0.6 + y*0.4) * intensity;
	float sunAmount = max(dot(sunDirection, viewDir), 0.0);
	skyColor += atmosphericColor.rgb * sunAmount * sunAmount * atmosphericColor.a;
	skyColor += sunColor.rgb * pow(sunAmount, sunRadius) * sunColor.a;
	skyColor += glowColor1.rgb * pow(sunAmount, 8.0) * glowColor1.a;
	skyColor += glowColor2.rgb * pow(sunAmount, 3.0) * glowColor2.a;
	return skyColor;
}

void main(void)
{
	vec3 viewDir = normalize(varWorldPos.xyz - uCamPosition.xyz);
	vec3 skyColor = GetSkyColor(
		viewDir,
		normalize(uLightDirection),
		uSkyIntensity,
		uAtmospheric,
		uSun,
		uGlare1,
		uGlare2,
		uSunSize
	);
	vec3 result = mix(skyColor, sRGB(ugroundColor), pow(smoothstep(0.0,-0.025, viewDir.y), 0.2));
	FragColor = vec4(result, 1.0);
}