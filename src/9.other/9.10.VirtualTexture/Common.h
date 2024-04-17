#pragma once

struct WindowSize
{
	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;
};

enum ETextureFilterType
{
	/// @brief Point filtering - texture pixels become blocky up close.
	Point = 0,

	Linear,
	/// @brief Bilinear filtering - texture samples are averaged.
	Bilinear,
	/// @brief Trilinear filtering - texture samples are averaged and also blended between mipmap levels.
	Trilinear
};

enum ETextureAddressingMode
{
	/// @brief Tiles the texture, creating a repeating pattern at every integer boundary.
	TAM_Repeat,
	/// @brief Clamps the texture to the last pixel at the edge.
	TAM_Clamp,
	/// @brief Tiles the texture, creating a repeating pattern by mirroring it at every integer boundary.
	TAM_Mirror,
};

enum EPixelFormat
{
	/// @brief Unknown.
	PF_Unknown = 0,
	/// @brief Color with alpha texture format, 8-bits per channel.
	PF_A8R8G8B8,
	/// @brief Color with alpha texture format, 8-bits per channel.
	PF_R8G8B8A8,
	/// @brief Color with alpha texture format, 8-bits per channel, use sRGB color space.
	PF_R8G8B8A8_SRGB,
	/// @brief Color texture format, 8-bits per channel.
	PF_R8G8B8,
	/// @brief Color texture format, 8-bits per channel, use sRGB color space.
	PF_R8G8B8_SRGB,
	/// @brief A 16 bit color texture format.
	PF_R5G6B5,
	/// @brief Compressed color texture format.
	PF_DXT1,
	/// @brief Compressed color texture format.
	PF_DXT3,
	/// @brief Compressed color with alpha channel texture format.
	PF_DXT5,
	/// @brief Depth Stencil format, 16-bits per channel.
	PF_DepthStencil,
	/// @brief Depth Stencil format, 16-bits per channel.
	PF_ShadowMap,
	PF_D24S8_UINT,
	/// @brief  Color texture format, 16-bits per channel.
	PF_R16G16B16,
	/// @brief  Color texture format, 32-bits per channel.
	PF_R32G32B32,
	/// @brief  Color with alpha texture format, 16-bits per channel.
	PF_R16G16B16A16, // 13
	/// @brief  Color with alpha texture format, 32-bits per channel.
	PF_R32G32B32A32, // 14
	/// @brief  Single channel (R) texture format, 8 bit integer.
	PF_Red,
	PF_BGRA,
	PF_BGRA_SRGB,
	/// @brief  Android External OES Texture
	PF_Android_OES,
	/// @brief Color with alpha texture format, 8-bits per channel, Value is unisigned int, from 0 to 255
	PF_R8G8B8A8_UI,
	PF_Count
};

enum TextureType
{
	/// @brief None
	TT_None = 0,
	/// @brief GL_TEXTURE_1D
	TT_1D = 1,
	/// @brief GL_TEXTURE_2D
	TT_2D,
	/// @brief GL_TEXTURE_3D
	TT_3D,
	/// @brief GL_TEXTURE_CUBE_MAP
	TT_Cubemap,
	/// @brief Not Supported
	TT_Sphere,
	/// @brief GL_TEXTURE_ARRAY
	TT_2DArray,
	TT_2DOES
};

enum EColorSpace
{
	ECS_Linear,
	ECS_sRGB
};

struct TextureInfo
{
	int width=0;
	int height=0;
	bool hasmip=false;
	int layernum=1;
	ETextureAddressingMode addressingMode= ETextureAddressingMode::TAM_Clamp;
	ETextureFilterType filterType= ETextureFilterType::Point;
	EPixelFormat format= EPixelFormat::PF_R8G8B8A8;
	EColorSpace colorSpace= EColorSpace::ECS_sRGB;
	bool readback = false;
};