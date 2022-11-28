#ifndef OBSTACLE_H
#define OBSTACLE_H
#include "Track.hpp"

class obstacle {

	
	
	
public:	
	float x,y;
	float sizeX=1.f;
	float sizeY=1.f;
	void posInicial(float x0, float y0);
	bool colisionado = 0;

	void Move(float yPos);
	
	
};

#endif

