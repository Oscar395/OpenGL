#pragma once

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

class FrameBuffer {
public:
	FrameBuffer() = default;
	FrameBuffer(float width, float height, bool multiSample = true);
	~FrameBuffer();
	unsigned int getFrameTexture();
	void rescaleFrameBuffer(float width, float height);
	void blitFrameBuffers(float width, float height);
	void Bind() const;
	void UnBind() const;
private:
	unsigned int fbo;
	unsigned int texture;
	unsigned int rbo;
	unsigned int textureMultiSampled;
	unsigned int intermediateFBO;
	bool multiSampled = true;
};