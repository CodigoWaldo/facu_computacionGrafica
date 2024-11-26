#include <glm/ext.hpp>
#include "drawScene.hpp"
#include "Model.hpp"
#include "Shaders.hpp"
#include "Callbacks.hpp"

extern Shader shader_texture, shader_phong, shader_smap;
extern Model model_chookity, model_teapot, model_suzanne, model_floor_flat, model_floor_random, model_light, model_crate;
extern glm::vec4 lightPosition;
extern bool flat_floor;

void drawModel(const Model &model, const glm::mat4 &m, int pass) {
	
	Shader &shader = [&]()->Shader&{
		if (pass==1) return shader_smap;
		if (model.texture.isOk()) {
			model.texture.bind(1);
			return shader_texture;
		}
		return shader_phong;
	}();
	
	shader.use();
	
	setMatrixes(shader,m);
	
	glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosition),
									  glm::vec3( 0.0f, 0.0f,  0.0f),
									  glm::vec3( 0.0f, 1.0f,  0.0f));
	
	constexpr float width = 5.f, near_plane = 1.f, far_plane = 8.f;
	glm::mat4 lightProjection = glm::ortho(-width, width, -width, width, near_plane, far_plane);
	
	shader.setUniform("lightViewMatrix",lightView);
	shader.setUniform("lightProjectionMatrix",lightProjection);
	
	shader.setUniform("viewPos",view_pos);
	
	shader.setUniform("depthTexture",0);
	shader.setUniform("colorTexture",1);
	
	// setup light and material
	shader.setLight(lightPosition, glm::vec3{1.f,1.f,1.f}, 0.4f);
	shader.setMaterial(model.material);
	
	// send geometry
	shader.setBuffers(model.buffers);
	model.buffers.draw();
}

void drawScene(int pass) {
	static glm::vec3 y_axis(0.f,1.f,0.f);
	static const glm::mat4 identity(1.f);
	
	// floor (dos veces porque no es cerrada, por si activan el cull face)
	glFrontFace(GL_CCW);
	drawModel(flat_floor?model_floor_flat:model_floor_random,identity,pass);
	glFrontFace(GL_CW);
	drawModel(flat_floor?model_floor_flat:model_floor_random,identity,pass);
	
	// teapots
	static glm::mat4 teapot1_matrix =
		glm::translate(identity,glm::vec3(-1.25f,+0.2f,-1.8f) ) *
		glm::rotate( identity, 1.23f, y_axis ) *
		glm::scale( identity, glm::vec3(.2f) );
	drawModel(model_teapot,teapot1_matrix,pass);
	static glm::mat4 teapot2_matrix =
		glm::translate(identity,glm::vec3(+1.8f,+0.2f,-0.25f) ) *
		glm::rotate( identity, -0.53f, y_axis ) *
		glm::scale( identity, glm::vec3(.2f) );
	drawModel(model_teapot,teapot2_matrix,pass);
	
	// crates
	static glm::mat4 crate1_matrix =
		glm::translate(identity,glm::vec3(-1.1f,-0.25f,+0.6f) ) *
		glm::rotate( identity, 0.75f, y_axis );
	drawModel(model_crate,crate1_matrix,pass);
	static glm::mat4 crate2_matrix =
		glm::translate(identity,glm::vec3(-0.5f,-0.25f,+1.5f) ) *
		glm::rotate( identity, 2.5f, y_axis );
	drawModel(model_crate,crate2_matrix,pass);
	
	// suzanne
	static glm::mat4 suzzane_matrix =
		glm::translate( identity, glm::vec3(1.4f,0.f,1.2f) ) *
		glm::rotate( identity, -0.75f, glm::vec3(0.8f,0.4f,-0.13f) ) *
		glm::scale( identity, glm::vec3(.5f) );
	drawModel(model_suzanne,suzzane_matrix,pass);
	
	// chookity
	static glm::mat4 chookity_matrix =
		glm::translate( identity, y_axis*0.7f ) *
		glm::rotate( identity, 0.5f, y_axis ) *
		glm::scale( identity, glm::vec3(.4f) );
	drawModel(model_chookity,chookity_matrix,pass);
}

