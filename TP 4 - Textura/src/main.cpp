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
#include "DrawScene.hpp"
#include "Flare.hpp"

#define VERSION 20240928

Window window;

const std::vector<std::string> model_files {
	"chookity",
	"teapot",
	"suzanne",
	"sphere",
	"icosahedro"
};



std::vector<Model> models;
Model sky_dome_model;

bool shaders_ok = true;
const std::vector<std::string> model_shaders_files{ "phong", "texture" };
std::vector<Shader> model_shaders;

const std::vector<std::string> sky_dome_shaders_files {"sky_dome", "tex_coords"};
std::vector<Shader> sky_dome_shaders;

const std::vector<std::string> flare_shaders_files{"flare", "flare_debug"};
std::vector<Shader> flare_shaders;

std::vector<Flare> flares;

glm::vec4 light_position(7.22f, 6.378f, -2.65f , 1.f); // On the sun spot of the sky box
glm::vec2 light_position_on_screen; // on pixel coordinates

bool is_light_on_screen = false;

/// UI controls
bool draw_flares = true,
	 debug_flares = false,
	 outter_view = false,
	 show_tex_coords = false;

int loaded_model = 0, loaded_shader = 0;

// extra callbacks
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

void loadModels();
void loadShaders();
void reloadShaders();

int main() {
	
	// initialize window and setup callbacks
	window = Window(win_width,win_height,"CG Demo",Window::fDefaults|Window::fBlend);
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	
	FrameTimer ftime;
	
	loadModels();
	
	loadShaders();
	
	// Update sky dome texture coordinates.
	const auto &positions = sky_dome_model.geometry.positions;
	auto tex_coords = generateTextureCoordinatesForSkyDome(positions);
	if(!tex_coords.empty()) {
		sky_dome_model.geometry.tex_coords = tex_coords;
		sky_dome_model.buffers.updateTexCoords(tex_coords, true);
	}
	
	// setup OpenGL state and load shaders
	glClearColor(0.8f, 0.8f, 0.7f, 1.f);
	
	// Setup frame rate
	double frameRate = 1.f/30.f;
	double accumDt = 0.f;
	do {
		// Control frame rate
		double dt = ftime.newFrame();
		accumDt += dt;
		if(accumDt < frameRate) {
			continue;
		}
		
		accumDt = 0.f;
		
		// Update 2D ligth position (on screen)
		updateScreenPositionOfLight();
		
		// Clear color and depth buffer of the scene
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		// Draw sky box and meshes
		drawScene(models[loaded_model]);
		
		// Draw only if flares are enabled
		if( draw_flares && is_light_on_screen ) {
			drawFlares();
		}
		
		/// Settings sub-window (ImGUI)
		window.ImGuiDialog("CG Example",[&]() {
			if (ImGui::TreeNode("Sky Dome")) {				
				ImGui::Checkbox("Show tex coords", &show_tex_coords);
				ImGui::Checkbox("View from outside", &outter_view);
				ImGui::TreePop();
			}
			
			if (ImGui::TreeNode("Flares")) {
				ImGui::Checkbox("Show",&draw_flares);
				
				if(!draw_flares) ImGui::BeginDisabled();
				
				ImGui::SliderFloat("Distance", &flares_distance, 0.1f, 1.0f, "%.2f", ImGuiSliderFlags_None);
				ImGui::SliderInt("Count", &flares_count, 1, 9, "%d", ImGuiSliderFlags_None);
				
				ImGui::Checkbox("Debug",&debug_flares);
				if(!draw_flares) ImGui::EndDisabled();
				
				ImGui::TreePop();
			}
			
			if (ImGui::BeginCombo(" Mesh", model_files[loaded_model].data(), ImGuiComboFlags_PopupAlignLeft )) {
				for (int i = 0; i < model_files.size(); ++i) {
					const bool isSelected = (loaded_model == i);
					if (ImGui::Selectable(model_files[i].data(), isSelected)) {
						loaded_model = i;
					}
					
					// Set the initial focus when opening the combo
					// (scrolling + keyboard navigation focus)
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			
			if (ImGui::Button("Reload Shaders (F5)"))
				reloadShaders();
			
			ImGui::Text(shaders_ok?"   Shaders compilation: Ok":"    Shaders compilation: ERROR");
			
		});
		
		// finish frame
		window.finishFrame();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'D': debug_flares = !debug_flares; break; // debug mode
		case 'F': draw_flares = !draw_flares; break; // disable flares
		case 'C': show_tex_coords = !show_tex_coords; break; // disable flares
		case GLFW_KEY_F5: reloadShaders(); break;
		}
	}
}

void loadModels() {	
	for(const auto &name : model_files) {
		models.emplace_back(Model::loadSingle("models/" + name, Model::fNone));
	}
	
	sky_dome_model = Model::loadSingle("models/sphere", Model::fKeepGeometry);
	
	// Load sky dome texture
	Texture sky_dome_texture("textures/skydome.jpg");
	sky_dome_model.texture = std::move(sky_dome_texture);
	
	// Load flares and textures
	for(int i=0;i<flares_max_count;i++) {
		const std::string file_name = "textures/flares/tex" + std::to_string(i) + ".png";
		flares.emplace_back(Flare(file_name));
	}
}
	
void loadShaders() {
	for(const auto& name : model_shaders_files) {
		model_shaders.push_back(Shader("shaders/" + name)); 
	}
	
	for(const auto& name : sky_dome_shaders_files) {
		sky_dome_shaders.push_back(Shader("shaders/" + name)); 
	}
	
	for(const auto& name : flare_shaders_files) {
		flare_shaders.push_back(Shader("shaders/" + name)); 
	}
}

bool reloadShader(Shader &shader, std::string file_name) {
	try {
		Shader new_shader(file_name);
		shader = std::move(new_shader);
		return true;
	} catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}
	
void reloadShaders() {	
	shaders_ok = true;
	
	// Reload all shaders
	int i = 0;
	for(const auto& name : model_shaders_files) {
		shaders_ok &= reloadShader(model_shaders[i++], "shaders/" + name);
	}
	
	i=0;
	for(const auto& name : sky_dome_shaders_files) {
		shaders_ok &= reloadShader(sky_dome_shaders[i++], "shaders/" + name);
	}
	
	i=0;
	for(const auto& name : flare_shaders_files) {
		shaders_ok &= reloadShader(flare_shaders[i++], "shaders/" + name);
	}
}
