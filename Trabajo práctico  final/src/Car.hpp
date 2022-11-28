#ifndef CAR_HPP
#define CAR_HPP
#include "Track.hpp"

using Angulo = float; // los ángulos están en radianes

struct Car {
	Car(float x0, float y0, float ang0) : x(x0), y(y0), ang(ang0) { }
	float x, y, ang; // posicion y orientacion en la pista
	float vel = 0; // velocidad actual
	Angulo rang1 = 0; // direccion de las ruedas delanteras respecto al auto (eje x del mouse) 
	Angulo rang2 = 0; // giro de las ruedas sobre su eje, cuando el auto avanza 
	float top_speed = 100; // velociad máxima
	float distanciaRecorrida = 0; // velociad máxima
	float sizeX=2.f;
	float sizeY=1.f;
	void updateTopSpeed(float value);
	void Move(const Track &track, float acel, float dir, bool analog=false); // función que aplica la "física" y actualiza el estado
};

#endif
