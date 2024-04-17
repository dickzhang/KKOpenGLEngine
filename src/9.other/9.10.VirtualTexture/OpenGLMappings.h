#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Common.h"

class  OpenGLMappings
{
public:
	static GLenum Get(ETextureFilterType type)
	{
		switch (type)
		{
			case ETextureFilterType::Point:
				return GL_NEAREST;
			case ETextureFilterType::Linear:
			case ETextureFilterType::Bilinear:
			case ETextureFilterType::Trilinear:
				return GL_LINEAR;
			default:
				return GL_NEAREST;
		}
	}

	static GLenum Get(ETextureAddressingMode value)
	{
		switch (value)
		{
			case ETextureAddressingMode::TAM_Clamp:		return GL_CLAMP_TO_EDGE;
			case ETextureAddressingMode::TAM_Repeat:	return GL_REPEAT;
			case ETextureAddressingMode::TAM_Mirror:	return GL_MIRRORED_REPEAT;
		}
		return -1;
	}

	static GLenum Get(EPixelFormat value)
	{
		switch (value)
		{
			case EPixelFormat::PF_R5G6B5:
				return GL_RGB565;
			case EPixelFormat::PF_R8G8B8:
				return GL_RGB8;
			case EPixelFormat::PF_R8G8B8_SRGB:
				return GL_SRGB8;
			case EPixelFormat::PF_R8G8B8A8_SRGB:
			case EPixelFormat::PF_BGRA_SRGB:
				return GL_SRGB8_ALPHA8;
			case EPixelFormat::PF_R8G8B8A8:
			case EPixelFormat::PF_BGRA:
				return GL_RGBA8;
			case EPixelFormat::PF_R16G16B16:
				return GL_RGB16F;
			case EPixelFormat::PF_R32G32B32:
				return GL_RGB32F;
			case EPixelFormat::PF_R16G16B16A16:
				return GL_RGBA16F;
			case EPixelFormat::PF_R32G32B32A32:
				return GL_RGBA32F;
			case EPixelFormat::PF_Red:
				return GL_RED;
			case EPixelFormat::PF_DepthStencil:
				return GL_DEPTH24_STENCIL8;
			case EPixelFormat::PF_ShadowMap:
				return GL_DEPTH_COMPONENT24;
		}
		return -1;
	}

	static EPixelFormat GetEnginePixelFormat(GLenum internalFormat, GLenum format)
	{
		switch (internalFormat)
		{
			case GL_RGB565:
				return EPixelFormat::PF_R5G6B5;
			case GL_RGB8:
			case GL_COMPRESSED_RGB8_ETC2:
				return EPixelFormat::PF_R8G8B8;
			case GL_SRGB8:
			case GL_COMPRESSED_SRGB8_ETC2:
				return EPixelFormat::PF_R8G8B8_SRGB;
			case GL_SRGB8_ALPHA8:
			case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
				if (format == GL_RGBA)
				{
					return EPixelFormat::PF_R8G8B8A8_SRGB;
				}
				else
				{
					return EPixelFormat::PF_BGRA_SRGB;
				}
			case GL_RGBA8:
			case GL_COMPRESSED_RGBA8_ETC2_EAC:
				if (format == GL_RGBA)
				{
					return EPixelFormat::PF_R8G8B8A8;
				}
				else
				{
					return EPixelFormat::PF_BGRA;
				}
			case GL_RGB16F:
				return EPixelFormat::PF_R16G16B16;
			case GL_RGB32F:
				return EPixelFormat::PF_R32G32B32;
			case GL_RGBA16F:
				return EPixelFormat::PF_R16G16B16A16;
			case GL_RGBA32F:
				return EPixelFormat::PF_R32G32B32A32;
			case GL_RED:
				return EPixelFormat::PF_Red;
			case GL_DEPTH24_STENCIL8:
				return EPixelFormat::PF_DepthStencil;
			case GL_DEPTH_COMPONENT24:
				return EPixelFormat::PF_ShadowMap;
		}
		return EPixelFormat::PF_R8G8B8A8_SRGB;
	}

	static GLenum Get(TextureType type)
	{
		switch (type)
		{
			case TextureType::TT_1D:
				return GL_TEXTURE_1D;
			case TextureType::TT_2D:
				return GL_TEXTURE_2D;
			case TextureType::TT_3D:
				return GL_TEXTURE_3D;
			case TextureType::TT_Cubemap:
				return GL_TEXTURE_CUBE_MAP;
			case TextureType::TT_2DArray:
				return GL_TEXTURE_2D_ARRAY;

		}
		return GL_NONE;
	}

	static GLenum GetPixelFormat(EPixelFormat format)
	{
		switch (format)
		{
			case EPixelFormat::PF_Red:
				return GL_RED;
			case EPixelFormat::PF_R5G6B5:
			case EPixelFormat::PF_R8G8B8:
			case EPixelFormat::PF_R8G8B8_SRGB:
			case EPixelFormat::PF_R16G16B16:
			case EPixelFormat::PF_R32G32B32:
				return GL_RGB;
			case EPixelFormat::PF_R8G8B8A8:
			case EPixelFormat::PF_R8G8B8A8_SRGB:
			case EPixelFormat::PF_R16G16B16A16:
			case EPixelFormat::PF_R32G32B32A32:
				return GL_RGBA;
			case EPixelFormat::PF_DepthStencil:
				return GL_DEPTH_STENCIL; ;
			case EPixelFormat::PF_ShadowMap:
				return GL_DEPTH_COMPONENT;
			case EPixelFormat::PF_BGRA:
			case EPixelFormat::PF_BGRA_SRGB:
				return GL_BGRA;
		}
		return ~0;
	}

	static GLenum GetPixelType(EPixelFormat format)
	{
		switch (format)
		{
			case EPixelFormat::PF_R5G6B5:
			case EPixelFormat::PF_R8G8B8:
			case EPixelFormat::PF_R8G8B8_SRGB:
			case EPixelFormat::PF_R8G8B8A8:
			case EPixelFormat::PF_R8G8B8A8_SRGB:
			case EPixelFormat::PF_R8G8B8A8_UI:
			case EPixelFormat::PF_Red:
			case EPixelFormat::PF_BGRA:
			case EPixelFormat::PF_BGRA_SRGB:
				return GL_UNSIGNED_BYTE;
			case EPixelFormat::PF_R16G16B16:
			case EPixelFormat::PF_R16G16B16A16:
				return GL_HALF_FLOAT;
			case EPixelFormat::PF_R32G32B32A32:
			case EPixelFormat::PF_R32G32B32:
				return GL_FLOAT;
			case EPixelFormat::PF_DepthStencil:
				return GL_UNSIGNED_INT_24_8;
			case EPixelFormat::PF_ShadowMap:
				return GL_UNSIGNED_INT;
		}
		return ~0;
	}
};
