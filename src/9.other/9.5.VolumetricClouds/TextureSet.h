#pragma once

class TextureSet
{ //for drawing compute shader
public:
	TextureSet(int W, int H, int num);
	void bindTexture(int i, int unit);
	unsigned int getColorAttachmentTex(int i);
	int getNTextures() const {
		return nTextures;
	}
	void bind();
	unsigned int generateTexture2D(int w, int h);
	void bindTexture2D(unsigned int tex, int unit);
private:
	int nTextures;
	unsigned int * texture;
};