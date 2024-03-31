#pragma once

#include "ShaderProgram.h"

class ComputeShader : public ShaderProgram
{
public:

	ComputeShader(const ShaderData& computeShaderData, int workGroupSizeX, int workGroupSizeY);
	void Execute();
	void Wait();

private:

	int workGroupSizeX_;
	int workGroupSizeY_;
};

