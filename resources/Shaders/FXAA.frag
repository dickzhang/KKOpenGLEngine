#version 450 core

#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D depthTexture;

struct LuminescenceData
{
    float centerLuma;
    float upRightLuma;
    float upLeftLuma;
    float downRightLuma;
    float downLeftLuma;
};

struct EdgeInterpolationValues 
{
    vec3 color;
    float depth;
};

float rgb2luma(in vec3 rgb)
{
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

void getLuminescenceOfEdgeAndCenterPixels(inout LuminescenceData lumData, vec2 texelSize)
{
    lumData.centerLuma = rgb2luma(texture(sceneTexture, TexCoords).rgb);
    lumData.upRightLuma = rgb2luma(texture(sceneTexture, TexCoords + vec2(1.0, -1.0) * texelSize).rgb);
    lumData.upLeftLuma = rgb2luma(texture(sceneTexture, TexCoords + vec2(-1.0, -1.0) * texelSize).rgb);
    lumData.downRightLuma = rgb2luma(texture(sceneTexture, TexCoords + vec2(1.0, 1.0) * texelSize).rgb);
    lumData.downLeftLuma = rgb2luma(texture(sceneTexture, TexCoords + vec2(-1.0, 1.0) * texelSize).rgb);
}

vec2 getBlurDirection(in LuminescenceData lumData)
{
    float upLuma = lumData.upLeftLuma + lumData.upRightLuma;
    float downLuma = lumData.downLeftLuma + lumData.downRightLuma;
    float leftLuma = lumData.upLeftLuma + lumData.downLeftLuma;
    float rightLuma = lumData.upRightLuma + lumData.downRightLuma;
    
    vec2 blurDirection;
    blurDirection.x = -(upLuma - downLuma);
    blurDirection.y = leftLuma - rightLuma;

    return blurDirection;
}

void normaliseBlurDirectionToMultipleOfeTexelSize(
    inout vec2 blurDirection, 
    in LuminescenceData lumData, 
    vec2 texelSize)
{
    float lumaSum = lumData.upLeftLuma + lumData.upRightLuma + 
                    lumData.upLeftLuma + lumData.downLeftLuma;

    // the dir reduce is to pervent division by 0
    float dirReduce = max(lumaSum * 0.25 * FXAA_REDUCE_MUL, FXAA_REDUCE_MIN);
    float directionScalingFactor = 1.0 / (min(abs(blurDirection.x), abs(blurDirection.y)) + dirReduce);

    blurDirection = clamp(blurDirection * directionScalingFactor,
                          vec2(-FXAA_SPAN_MAX, FXAA_SPAN_MAX),
                          vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX)) * texelSize;
}

vec3 getColorOnEdgeAtOffsetFromCenter(in vec2 offset)
{
    return texture(sceneTexture, TexCoords + offset).rgb; 
}

float getDepthOnEdgeAtOffsetFromCenter(in vec2 offset)
{
    return texture(depthTexture, TexCoords + offset).r; 
}

void getEdgeInterpolationValuesFromCenter(
    inout EdgeInterpolationValues edgeValues,
    vec2 offsetPlusDir,
    vec2 offsetMinusDir)
{
    edgeValues.color = 0.5 * (
        getColorOnEdgeAtOffsetFromCenter(offsetPlusDir) + 
        getColorOnEdgeAtOffsetFromCenter(offsetMinusDir));

    edgeValues.depth = 0.5 * (
        getDepthOnEdgeAtOffsetFromCenter(offsetPlusDir) + 
        getDepthOnEdgeAtOffsetFromCenter(offsetMinusDir));
}

vec3 getBluredColorForPixel(vec2 blurDirection, LuminescenceData lumData)
{
    vec2 oneSixtOffset = blurDirection * vec2(1.0 / 3.0 - 0.5);
    
    EdgeInterpolationValues oneSixtEdgeInterpolated;
    getEdgeInterpolationValuesFromCenter(oneSixtEdgeInterpolated, 
                                         oneSixtOffset,
                                         -oneSixtOffset);

    vec2 oneHalfOffset = blurDirection * vec2(0.5);
    
    EdgeInterpolationValues oneHalfEdgeInterpolated;
    getEdgeInterpolationValuesFromCenter(oneHalfEdgeInterpolated,
                                         oneHalfOffset,
                                         -oneHalfOffset);

    oneHalfEdgeInterpolated.color = (oneHalfEdgeInterpolated.color + oneSixtEdgeInterpolated.color) * 0.5;
    oneHalfEdgeInterpolated.depth = (oneHalfEdgeInterpolated.depth + oneSixtEdgeInterpolated.depth) * 0.5;

    float lumaMin = min(lumData.centerLuma, 
                        min(min(lumData.upRightLuma, lumData.upLeftLuma),
                            min(lumData.downRightLuma, lumData.downLeftLuma)));
    
    float lumaMax = max(lumData.centerLuma,
                        max(max(lumData.upRightLuma, lumData.upLeftLuma),
                            max(lumData.downRightLuma, lumData.downLeftLuma))); 
    
    float lumaOfHalfEdge = rgb2luma(oneHalfEdgeInterpolated.color);

    float depthAtCenter = getDepthOnEdgeAtOffsetFromCenter(vec2(0.0));
    vec3 colorAtCenter = getColorOnEdgeAtOffsetFromCenter(vec2(0.0));

    if (lumaOfHalfEdge < lumaMin || lumaOfHalfEdge > lumaMax)
    {
        return oneSixtEdgeInterpolated.depth == depthAtCenter ? colorAtCenter : oneSixtEdgeInterpolated.color;
    }
    else
    {
        return oneHalfEdgeInterpolated.depth == depthAtCenter ? colorAtCenter : oneHalfEdgeInterpolated.color;
    }
}

void main()
{
    const vec2 texelSize = 1.0 / vec2(textureSize(sceneTexture, 0));
    
    LuminescenceData lumData;
    getLuminescenceOfEdgeAndCenterPixels(lumData, texelSize);

    vec2 blurDirection = getBlurDirection(lumData);
    normaliseBlurDirectionToMultipleOfeTexelSize(blurDirection, lumData, texelSize);
    
    
    FragColor = vec4(getBluredColorForPixel(blurDirection, lumData), 1.0);
}