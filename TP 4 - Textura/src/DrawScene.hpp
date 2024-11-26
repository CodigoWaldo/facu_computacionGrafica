#ifndef DRAWSCENE_HPP
#define DRAWSCENE_HPP

#include "Shaders.hpp"

#include <glm/glm.hpp>

class Model;

std::vector<glm::vec2> generateTextureCoordinatesForSkyDome(const std::vector<glm::vec3> &positions);

void updateScreenPositionOfLight();

void drawFlares();
void drawSkySphere(const glm::mat4 &m);

void drawModel(const Model &model, const glm::mat4 &m);
void drawScene(const Model &model);

#endif

