#include "OpenGLTexture.h"
#include "OpenGLMappings.h"

//unsigned int OpenGLTexture::TextureFromFile(const char* path,const string& directory,bool gamma)
//{
//	string filename = string(path);
//	filename = directory+'/'+filename;
//	unsigned int textureID;
//	glGenTextures(1,&textureID);
//	int width,height,nrComponents;
//	unsigned char* data = stbi_load(filename.c_str(),&width,&height,&nrComponents,0);
//	if(data)
//	{
//		GLenum format;
//		if(nrComponents==1)
//			format = GL_RED;
//		else if(nrComponents==3)
//			format = GL_RGB;
//		else if(nrComponents==4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D,textureID);
//		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//
//		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//
//		stbi_image_free(data);
//	}
//	else
//	{
//		std::cout<<"Texture failed to load at path: "<<path<<std::endl;
//		stbi_image_free(data);
//	}
//
//	return textureID;
//}

unsigned int OpenGLTexture::loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1,&textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);

	int width,height,nrChannels;
	for(unsigned int i = 0; i<faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(),&width,&height,&nrChannels,0);
		if(data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
			stbi_image_free(data);
		}
		else
		{
			std::cout<<"Cubemap texture failed to load at path: "<<faces[i]<<std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int OpenGLTexture::generateTexture2D(int w,int h)
{
	unsigned int tex_output;
	glGenTextures(1,&tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex_output);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,w,h,0,GL_RGBA,GL_FLOAT,NULL);
	glBindImageTexture(0,tex_output,0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
	return tex_output;
}

void OpenGLTexture::updateTexture2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,const void* pixels)
{
	glBindTexture(GL_TEXTURE_2D,target);
	//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexSubImage2D(GL_TEXTURE_2D,level,xoffset,yoffset,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
	glBindTexture(GL_TEXTURE_2D,0);
}

void OpenGLTexture::readTexture2D(GLenum target,GLint level,GLenum format,GLenum type,void* pixels)
{
	glBindTexture(GL_TEXTURE_2D,target);
	glGetTexImage(target,level,format,type,pixels);
	glBindTexture(GL_TEXTURE_2D,0);
}

void OpenGLTexture::blit(GLenum dsttarget,GLint dstlevel,GLint dstxoffset,GLint dstyoffset,GLenum srctarget,GLint srclevel,GLint width,GLint height)
{
	GLenum format = OpenGLMappings::Get(EPixelFormat::PF_A8R8G8B8);
	GLenum pixelType = OpenGLMappings::GetPixelType(EPixelFormat::PF_A8R8G8B8);
	std::vector<uint8_t> m_downloadBuffer;
	m_downloadBuffer.resize(width*height*s_channelCount);
	OpenGLTexture::readTexture2D(srctarget,0,format,pixelType,&m_downloadBuffer[0]);
	OpenGLTexture::updateTexture2D(dsttarget,dstlevel,dstxoffset,dstyoffset,width,height,&m_downloadBuffer[0]);
	m_downloadBuffer.clear();
}

unsigned int OpenGLTexture::generateTexture3D(int w,int h,int d)
{
	unsigned int tex_output;
	glGenTextures(1,&tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D,tex_output);

	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA8,w,h,d,0,GL_RGBA,GL_FLOAT,NULL);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindImageTexture(0,tex_output,0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA8);
	return tex_output;
}

void OpenGLTexture::bindTexture2D(unsigned int tex,int unit)
{
	glBindImageTexture(unit,tex,0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
}

unsigned int OpenGLTexture::generateTexture2D(TextureInfo info,const Memory* _mem)
{
	unsigned int tex_output;
	glGenTextures(1,&tex_output);
	glBindTexture(GL_TEXTURE_2D,tex_output);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	auto addressmode = OpenGLMappings::Get(info.addressingMode);
	auto filterType = OpenGLMappings::Get(info.filterType);
	auto format = OpenGLMappings::Get(info.format);
	auto pixelFormat = OpenGLMappings::GetPixelFormat(info.format);
	auto pixelType = OpenGLMappings::GetPixelType(info.format);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,addressmode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,addressmode);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,filterType);
	if(info.hasmip)
	{
		glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,filterType);
	}
	glTexImage2D(GL_TEXTURE_2D,0,format,info.width,info.height,0,pixelFormat,pixelType,_mem ? _mem->data : NULL);
	if(info.hasmip)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D,0);
	return tex_output;
}

