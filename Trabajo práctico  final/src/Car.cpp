#include <cmath>
#include "Car.hpp"

constexpr float PI = 3.14159265359f;
constexpr float G2R = PI/180.f;

void Car::Move(const Track &track, float acel, float dir, bool analog) {

	acel=1.f;
	// aplicar los controles
	double target_dir = 0.f*0.006*float(10*vel+70*(top_speed-vel))/top_speed;
	rang1 = analog ? target_dir : ((8*rang1+target_dir)/9);
	if (!acel) acel = -.03;
	
	// mover el auto	
	vel += acel*.60-.10;
	if (vel<0) vel=0;
	else if (vel>top_speed) vel=top_speed;	
	
	ang=2*PI;
	x += vel*1/100;
    y += vel*std::sin(ang)/100;
	
	
	
	
	if(dir==1){
		if(y<=6.f){y+=0.5;}		
	}else if(dir==-1){
		if(y>=-6.f){y-=0.5;}		
	}
	
	
	
	
	// la pista es ciclica
	if (x>track.Width()){
		x -= track.Width()*2;
		distanciaRecorrida+=0.1;
	}
	
	
	
	ang += rang1*vel/150;
	rang2 += vel/10;	
}


void Car::updateTopSpeed(float value){
	top_speed = value;
};
