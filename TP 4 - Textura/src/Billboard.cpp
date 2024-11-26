#include "Billboard.hpp"

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

std::unique_ptr<Model> Billboard::m_billboard = nullptr;

std::unique_ptr<Model>& Billboard::getBillboard() {
	// Create billboard model
	if(!m_billboard) {
		const std::vector<glm::vec3> billboard_vertices = {
			// positions
			{-1.f, -1.f, 0.f},	// bottom left
			{ 1.f, -1.f, 0.f},	// bottom right
			{ 1.f,  1.f, 0.f},	// top right
			{-1.f,  1.f, 0.f}	// top left
		};
		
		const std::vector<int> billboard_indices = {
			0, 1, 2,   // first triangle
			2, 3, 0    // second triangle
		};
		
		const std::vector<glm::vec2> billboard_texCoords = {
			{0.f, 0.f},
			{1.f, 0.f},
			{1.f, 1.f},
			{0.f, 1.f}
		};
		
		Geometry geometry{
			billboard_vertices,			// Positions
			std::vector<glm::vec3>(),	// Normals
			billboard_texCoords,		// Texture coordinates
			billboard_indices			// Indices
		};
		
		geometry.generateNormals();
		
		m_billboard.reset(new Model(std::move(geometry), Material(), Model::Flags::fNone));
	}
	
	return m_billboard;
}
