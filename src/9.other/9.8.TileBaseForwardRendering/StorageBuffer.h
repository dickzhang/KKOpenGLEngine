#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

template<typename T>
class StorageBuffer
{
public:
	StorageBuffer();

	void Init(const T* data,unsigned int type,int numVariables,int bufferBindIndex);
	void Bind();
	void InsertData(T* data,int offset);
	unsigned int GetId() const
	{
		return id_;
	}
	void UnbindBuffer();
	~StorageBuffer();

private:
	unsigned int id_;
	unsigned int type_;
};

template<typename T>
void StorageBuffer<T>::Init(const T* data,unsigned int type,int numVariables,int bufferBindIndex)
{
	type_ = type;
	glGenBuffers(1,&id_);
	glBindBuffer(type_,id_);
	glBufferData(type_,sizeof(T)*numVariables,data,GL_STATIC_DRAW);
	glBindBufferBase(type_,bufferBindIndex,id_);
	glBindBuffer(type_,0);
}

template<typename T>
StorageBuffer<T>::StorageBuffer()
{
}
template<typename T>
void StorageBuffer<T>::Bind()
{
	glBindBuffer(type_,id_);
}
template<typename T>
void StorageBuffer<T>::InsertData(T* data,int offset)
{
	glBufferSubData(type_,offset,sizeof(T),data);
}
template<typename T>
void StorageBuffer<T>::UnbindBuffer()
{
	glBindBuffer(type_,0);
}
template<typename T>
StorageBuffer<T>::~StorageBuffer()
{
}