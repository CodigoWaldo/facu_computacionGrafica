#ifndef FLARES_H
#define FLARES_H

#include "Texture.hpp"

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Model;

/// Flare properties
extern float flares_distance;
extern const float flare_max_size;
extern const float flare_min_size;
extern const int flares_max_count;
extern int flares_count;

class Flare {

public:	
	Flare(const std::string &flareFileName);
	Flare(Flare &&flare);

	~Flare();
	
	// Display the billboard with the flare texture
	void render(float size, const glm::vec2& center);
	
private:
	std::vector<glm::vec2> generateTextureCoordinatesForFlare(float size, const glm::vec2& center);
	
	Texture m_texture;
};


#endif
