#ifndef DRAWSCENE_HPP
#define DRAWSCENE_HPP

#include <glm/glm.hpp>
class Model;

// pass: 1=generar shadow map, 2=generar imagen final(aplicar map), 3=normal(sin sombras)
void drawModel(const Model &model, const glm::mat4 &m, int pass);
void drawScene(int pass);

#endif

