#include "FramebufferTexture.hpp"
#include "Debug.hpp"

FramebufferTexture::FramebufferTexture(int width, int height, Type type)
	: m_width(width), m_height(height), m_type(type)
{
	glGenFramebuffers(1, &m_fbo);
	glGenTextures(1, &m_tex);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	auto get_component= [&]() {
		switch(type) {
		case Depth: return GL_DEPTH_COMPONENT;
		case Stencil: return GL_STENCIL_INDEX;
		case Color: return GL_RGB;
		default: cg_error("Wrong framebuffer type");
		}
	};
	glTexImage2D(GL_TEXTURE_2D, 0, get_component(),
				 width, height, 0, get_component(), GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	auto get_attachment = [&]() {
		switch(type) {
		case Depth: return GL_DEPTH_ATTACHMENT;
		case Stencil: return GL_STENCIL_ATTACHMENT;
		case Color: return GL_COLOR_ATTACHMENT0;
		default: cg_error("Wrong framebuffer type");
		}
	};
	glFramebufferTexture2D(GL_FRAMEBUFFER, get_attachment(), GL_TEXTURE_2D, m_tex, 0);
}

void FramebufferTexture::bindFramebuffer (bool and_set_viewport) const {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	if (and_set_viewport) glViewport(0,0,m_width,m_height);
}

void FramebufferTexture::bindTexture(int tex_num) const {
	glActiveTexture(GL_TEXTURE0+tex_num);
	glBindTexture(GL_TEXTURE_2D, m_tex);
}

FramebufferTexture::~FramebufferTexture() {
	if (m_type==None) return;
	glDeleteTextures(1,&m_tex);
	glDeleteFramebuffers(1,&m_fbo);
}

FramebufferTexture &FramebufferTexture::operator=(FramebufferTexture &&other) {
	m_type = other.m_type;     other.m_type = None;
	m_tex = other.m_tex;       other.m_tex = 0;
	m_fbo = other.m_fbo;       other.m_fbo = 0;
	m_width = other.m_width;   other.m_width = 0;
	m_height = other.m_height; other.m_height = 0;
	return *this;
}

FramebufferTexture::FramebufferTexture (FramebufferTexture &&other) {
	this->operator=(std::move(other));
}

