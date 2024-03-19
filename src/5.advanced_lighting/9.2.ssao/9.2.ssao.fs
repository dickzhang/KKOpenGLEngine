#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 view;
uniform mat4 projection;

void main()
{
    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    float verify1 = texture(gNormal, TexCoords).w;
    if (verify1 == 0.0) 
    {
        for(int i = 0; i < kernelSize; ++i)
        {
            // 获取样本位置
            vec3 samplePos = TBN * samples[i]; // 切线->观察空间
            samplePos = fragPos + samplePos * radius; 
            vec4 offset = vec4(samplePos, 1.0);
            vec4 check2 = view * offset;
            float ssao_guessed_depth = check2.z/ check2.w;

            // project sample position (to sample texture) (to get position on screen/texture)
            offset = projection *view* offset; // from view to clip-space
            offset.xyz /= offset.w; // perspective divide
            offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
         
        	if (offset.x < 1 && offset.y < 1 && offset.x > 0 && offset.y > 0)
           {
               vec3 aaa = texture(gPosition, offset.xy).xyz;
                aaa=(view * vec4(aaa,1.0)).xyz;
                float gbuffer_depth = aaa.z;
                float verify3 = texture(gNormal, offset.xy).w;
                if (verify3 == 0.0)
                {
               	    if (abs(ssao_guessed_depth - gbuffer_depth)<=0.5)
                    {
               		    occlusion+= ((gbuffer_depth > ssao_guessed_depth + bias) ? 1.0 : 0.0);
               	    }
                }
            }
        }
    }
    FragColor = 1.0-(occlusion / kernelSize);
}
