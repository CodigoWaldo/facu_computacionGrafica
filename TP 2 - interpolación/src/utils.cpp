#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "utils.hpp"
#include "Debug.hpp"

BoundingBox::BoundingBox(glm::vec3 &p1, glm::vec3 &p2) 
	: pmin({std::min(p1.x,p2.x),std::min(p1.y,p2.y),std::min(p1.z,p2.z)}),
      pmax({std::max(p1.x,p2.x),std::max(p1.y,p2.y),std::max(p1.z,p2.z)}) 
{
	
}
	
bool BoundingBox::contiene(glm::vec3 &p) const {
	return p.x>=pmin.x && p.x<=pmax.x &&
		p.y>=pmin.y && p.y<=pmax.y &&
		p.z>=pmin.z && p.z<=pmax.z;
}

Pesos calcularPesos(glm::vec3 x0, glm::vec3 x1, glm::vec3 x2, glm::vec3 x) {
	/// @todo: implementar
    ///	cg_error("debe implementar la funcion calcularPesos (utils.cpp)");
	
	
	glm::vec3 a = cross((x2-x0), (x1-x0)); ///area total del triángulo formado por x0, x1 y x2
	glm::vec3 a0 = cross((x2-x), (x1-x)); ///area opuesta a x0
	glm::vec3 a1 = cross((x0-x), (x2-x)); ///area opuesta a x1
	glm::vec3 a2 = cross((x1-x), (x0-x)); ///area opuesta a x0
	
	float alfa0 = dot(a0,a)/dot(a,a); 
	float alfa1 = dot(a1,a)/dot(a,a);
	float alfa2 = dot(a2,a)/dot(a,a);
	
	
	
	return {alfa0, alfa1,alfa2};
	
	
	
	return {0,0,0};
}


