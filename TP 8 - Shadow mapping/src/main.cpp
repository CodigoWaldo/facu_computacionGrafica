#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "FramebufferTexture.hpp"
#include "DrawBuffers.hpp"
#include "drawScene.hpp"

#define VERSION 20241110

Window window;

// models and settings
Shader shader_texture, shader_phong, shader_smap;
Model model_chookity, model_teapot, model_suzanne, model_floor_flat, model_floor_random, model_light, model_crate;
glm::vec4 lightPosition;
float angle_light = 0.f;
bool flat_floor = false, rotate_light = true,
	 calc_shadow_map = true, display_shadow_map = false,
	 shaders_ok = true;

int shadow_map_resolution = 1024;

// extra callbacks
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

void reloadShaders();

int main() {
	
	// initialize window and setup callbacks
	window = Window(win_width,win_height,"CG Demo");
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	
	
	DrawBuffers draw_buffers;
	
	// setup OpenGL state and load shaders
	glClearColor(0.8f,0.8f,0.7f,1.f);
	shader_smap = Shader ("shaders/shadow_map");
	shader_texture = Shader ("shaders/texture");
	shader_phong = Shader("shaders/phong");
	
	// main loop
	model_chookity = Model::loadSingle("models/chookity",Model::fDontFit);
	model_teapot = Model::loadSingle("models/teapot",Model::fDontFit);
	model_suzanne = Model::loadSingle("models/suzanne",Model::fDontFit);
	model_floor_flat = Model::loadSingle("models/floor_flat",Model::fDontFit);
	model_floor_random = Model::loadSingle("models/floor_random",Model::fDontFit);
	model_crate = Model::loadSingle("models/crate",Model::fDontFit);
	model_light = Model::loadSingle("models/light",Model::fDontFit);
	int loaded_model = -1;
	FrameTimer ftime;
	view_target.y = .75f;
	view_pos.z *= 2;
	
	do {
		FramebufferTexture shadow_map(shadow_map_resolution,shadow_map_resolution,FramebufferTexture::Depth);
		
		view_angle = std::min(std::max(view_angle,-0.1f),1.72f);
		
		// update ligth position
		double dt = ftime.newFrame();
		if (rotate_light) {
			angle_light += static_cast<float>(1.f*dt);
			constexpr double PI = 2.0*acos(0.0);
			if (angle_light>PI) angle_light-=2*PI;
		}
		lightPosition.x = 3.0f*std::sin(angle_light);
		lightPosition.y = 3.0f;
		lightPosition.z = 3.0f*std::cos(angle_light);
		lightPosition.w = 1.0f;
				
			
		// generate shadow map
		shadow_map.bindFramebuffer(true);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);	
		glCullFace(GL_FRONT);
		if (calc_shadow_map) drawScene(1); // 1 -> usar el shader "shadow_map"		
		glDisable(GL_CULL_FACE); 

		window.bindFrameBuffer(true);
		if (display_shadow_map) {
			draw_buffers.drawDepth(win_width,win_height,2.f,shadow_map.getTexture());
			
		} else {
			// render scene
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			shadow_map.bindTexture(0);
			drawScene(2); // 2 -> usar los shaders "phong" y "texture"
			
			// light
			auto light_matrix = glm::scale( glm::translate( glm::mat4(1.f), glm::vec3(lightPosition) ),
											glm::vec3(1.f,1.f,1.f)/10.f );
			drawModel(model_light,light_matrix,2);
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			ImGui::Text("--- Shadow ---");
			ImGui::Checkbox("Generate Shadow Map (S)",&calc_shadow_map);	
			ImGui::Checkbox("Display Shadow Map (D)",&display_shadow_map);
			ImGui::SliderInt("Resolution",&shadow_map_resolution,1,4192);	
			ImGui::Text("--- Light ---");
			ImGui::Checkbox("Rotate Light (L)",&rotate_light);
			ImGui::SliderAngle("Light Angle",&angle_light,-180,+180);
			ImGui::Text("--- Floor ---");
			ImGui::Checkbox("Flat Floor (F)",&flat_floor);
			ImGui::Text("----");
			if (ImGui::Button("Reload Shaders (F5)"))
				reloadShaders();
			ImGui::Text(shaders_ok?"   Shaders compilation: Ok":"    Shaders compilation: ERROR");
		});
		
		window.finishFrame();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

bool reloadShader(Shader &shader, std::string fname) {
	try {
		Shader new_shader(fname);
		shader = std::move(new_shader);
		return true;
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}
void reloadShaders() {
	shaders_ok = reloadShader(shader_phong,"shaders/phong") and
				 reloadShader(shader_texture,"shaders/texture");
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'F': flat_floor = !flat_floor; break;
		case 'L': rotate_light = !rotate_light; break;
		case 'S': calc_shadow_map = !calc_shadow_map; break;
		case 'D': display_shadow_map = !display_shadow_map; break;
		case GLFW_KEY_F5: reloadShaders(); break;
		}
	}
}


