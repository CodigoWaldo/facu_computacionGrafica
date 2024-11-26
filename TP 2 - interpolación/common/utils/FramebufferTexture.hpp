#ifndef FRAMEBUFFER_TEXTURE_HPP
#define FRAMEBUFFER_TEXTURE_HPP
#include <glad/glad.h>

class FramebufferTexture {
public:
	enum Type { None, Color, Depth, Stencil };
	FramebufferTexture(int width, int height, Type type);
	FramebufferTexture(FramebufferTexture &&);
	FramebufferTexture &operator=(FramebufferTexture &&);
	FramebufferTexture(const FramebufferTexture &) = delete;
	FramebufferTexture &operator=(const FramebufferTexture &) = delete;
	~FramebufferTexture();
	
	void bindFramebuffer(bool and_set_viewport=false) const;
	void bindTexture(int tex_num) const;
	bool isOk() const { return m_type!=None; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	unsigned int getTexture() const { return m_tex; }
	
private:
	Type m_type = None;
	int m_width = 0, m_height = 0;
	unsigned int m_fbo = 0, m_tex = 0;
};

#endif

