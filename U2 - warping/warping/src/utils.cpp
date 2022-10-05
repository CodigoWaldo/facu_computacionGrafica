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

Pesos calcularPesos(glm::vec3 x0, glm::vec3 x1, glm::vec3 x2, glm::vec3 &x) {
	// Se calculan los pesos que ejercen los puntos x0,x1 y x2 sobre el punto x, se utiliza el método de COORDENADAS BARICENTRICAS
	// el cálculo de los pesos se obtiene por la proporción de las areas de los subtriangulos y el area total sobre p
	// si el area del subtriangulo opuesto al vertice de la malla es igual al area total, significa que el punto está sobre dicho vertice
	
	// cálculo del área total del Triangulo
	
	glm::vec3 areaTotal = cross(x1-x0,x0-x2);
	
	//cálculo de las áreas de los triangulos internos.
	
	glm::vec3 a0 = cross(x2-x,x1-x);
	glm::vec3 a1 = cross(x0-x,x2-x);
	glm::vec3 a2 = cross(x1-x,x0-x);
	
	//cálculo de los pesos que infieren por cada punto sobre X
	
	float p0 = 0;
	float p1 = 0;
	float p2 = 0;
	float cuadradoAreaTotal = dot(areaTotal,areaTotal);
	
	p0 = dot(a0,areaTotal) / cuadradoAreaTotal;
	p1 = dot(a1,areaTotal) / cuadradoAreaTotal;
	p2 = dot(a2,areaTotal) / cuadradoAreaTotal;	
	
	return {p0,p1,p2}; //el orden de los puntos debe coincidir con los parametros de entrada
		
}
