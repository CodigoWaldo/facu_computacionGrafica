#include <cmath>
#include <glm/ext.hpp>
#include "DrawScene.hpp"
#include "Model.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Flare.hpp"
#include "string"
extern glm::vec4 light_position;
extern glm::vec2 light_position_on_screen;

extern Model sky_dome_model;

extern std::vector<Shader> model_shaders;
extern std::vector<Shader> sky_dome_shaders;

extern std::vector<Flare> flares;

extern bool outter_view, show_tex_coords, is_light_on_screen;

std::vector<glm::vec2> generateTextureCoordinatesForSkyDome(const std::vector<glm::vec3> &positions) 
{
	
	float pi=3.1415926535;
	std::vector<glm::vec2> tex_coords;
	//@todo: generar el vector de coordenadas de texturas para los vertices de la esfera
	
	for(glm::vec3 vertice : positions){
		float s=(atan2(vertice.z,vertice.x)+pi)/(2*pi);
		float t=atan2(sqrt(pow(vertice.x,2) + pow(vertice.z,2)), vertice.y) / pi;
		
		tex_coords.push_back(glm::vec2 (s,t));
	}
	return tex_coords;
	

}

void updateScreenPositionOfLight() {
	common_callbacks::render_matrixes_t matrices = common_callbacks::getMatrixes();
	
	// Update light position to NDC coordinates
	glm::vec4 lightPosWorld = matrices.model * light_position;
	glm::vec4 lightPosNDC = matrices.projection * matrices.view * lightPosWorld;
	
	// NDC light position to range [-1, 1]
	lightPosNDC /= lightPosNDC.w;
	
	// To range [0, 1]
	lightPosNDC = (lightPosNDC + 1.f) * 0.5f;
	
	// To pixel coordinates
	light_position_on_screen = glm::vec2(lightPosNDC.x * win_width, lightPosNDC.y * win_height);
	
	// TODO: Check if object is blocking the light
	// Render depth, compute light position on NDC (or screen?) then check Z.
	
	is_light_on_screen = 
		// Check if light is in front of the view
		glm::dot(glm::vec3(lightPosWorld), view_pos) < 0.f
		// Check if light is on screen
		&& static_cast<unsigned int>(light_position_on_screen.x) <= win_width
		&& static_cast<unsigned int>(light_position_on_screen.y) <= win_height;
}

void drawFlares() {
	if(outter_view) {
		return;
	}
	
	const float screen_size = std::max(win_width, win_height);
	
	// Calcular distancia al centro
	const glm::vec2 screen_center(win_width * 0.5f, win_height * 0.5f);// On pixel coordinates
	const glm::vec2 light_to_center_ray(screen_center - light_position_on_screen);
	const float distance_to_center = glm::length(light_to_center_ray);
	
	// Deshabilitar el depth test para dibujar billboards
	glDisable(GL_DEPTH_TEST);
	
	// Calcular la posicion y el incremento a la posicion del siguiente flare
	const glm::vec2 pos_increment = flares_distance * distance_to_center * glm::normalize(light_to_center_ray);
	glm::vec2 position = light_position_on_screen + pos_increment; // in screen coordinates (pixels)
	
	// Render all flares
	for(int i=0;i<flares_count; i++)
	{
		// Cambiar escala del flare segun la distancia al centro de la pantalla (eleccion cosmetica)
		float scale = glm::length(position - screen_center)/glm::length(screen_center);
		
		// scale affected by the distance to the sun (thresholded to not amplify/reduce extra)
		scale *= std::min(glm::length(position - light_position_on_screen)/glm::length(screen_center) + 0.3f, 1.f);
		
		// Definir tamanio final del flare, en pixeles, segun el tamanio de la pantalla
		const float size = std::max(scale * flare_max_size * screen_size, flare_min_size * screen_size);
		
		flares[i].render(size, position);
		
		// Actualizar posiciones
		position += pos_increment;
	}
	
	glEnable(GL_DEPTH_TEST);
}

void drawSkySphere(const glm::mat4& m) {
	Shader &shader = [&]()->Shader&{
		if(show_tex_coords) {
			return sky_dome_shaders[1];
		}
		
		if (sky_dome_model.texture.isOk() && !sky_dome_model.geometry.tex_coords.empty()) {
			sky_dome_model.texture.bind(0);
			return sky_dome_shaders[0];
		}
		return model_shaders[0];
	}();
	
	shader.use();
	
	setMatrixes(shader, m);
	
	shader.setLight(light_position, glm::vec3{1.f,1.f,1.f}, 0.3f);
	shader.setUniform("colorTexture", 0);
	
	// setup light and material
	shader.setMaterial(sky_dome_model.material);
	
	// send geometry
	shader.setBuffers(sky_dome_model.buffers);
	sky_dome_model.buffers.draw();
}

void drawModel(const Model &model, const glm::mat4 &m) {	
	Shader &shader = [&]()->Shader&{
		if (model.texture.isOk()) {
			model.texture.bind(0);
			return model_shaders[1];
		}
		return model_shaders[0];
	}();
	
	shader.use();
	
	setMatrixes(shader,m);
	
	// setup light and material
	shader.setLight(light_position, glm::vec3{1.f,1.f,1.f}, 0.3f);
	shader.setMaterial(model.material);
	
	common_callbacks::render_matrixes_t matrices = common_callbacks::getMatrixes();
	shader.setUniform("viewPos", glm::vec3(glm::inverse(matrices.model) * glm::vec4(view_pos, 1.f)));
	shader.setUniform("colorTexture", 0);
	
	// send geometry
	shader.setBuffers(model.buffers);
	model.buffers.draw();
}

void drawScene(const Model& model) {
	if(outter_view) {
		// Draw sky sphere (scaled to be seen form outside)
		const glm::mat4 sky_dome_matrix = glm::scale(glm::mat4(1.f), glm::vec3(0.7f));
		drawSkySphere(sky_dome_matrix);
		return;
	}
	
	// Draw sky sphere (scaled to cover the world around the model)
	const glm::mat4 sky_dome_matrix = glm::scale(glm::mat4(1.f), glm::vec3(10.f));
	drawSkySphere(sky_dome_matrix);
	
	// Draw selected model, with specified shader
	glm::mat4 model_matrix = glm::scale(glm::mat4(1.f), glm::vec3(0.6f)); // Shrink 40%
	drawModel(model, model_matrix);	
}

