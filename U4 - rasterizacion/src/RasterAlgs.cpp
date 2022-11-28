#include <forward_list>
#include <iostream>
#include <GL/gl.h>
#include <cmath>
#include "RasterAlgs.hpp"
using namespace std;

void drawSegment(paintPixelFunction paintPixel, glm::vec2 p0, glm::vec2 p1) {
	/// @todo: implementar algun algoritmo de rasterizacion de segmentos 
	// alumno: Se implementa el algoritmo DDA para segmentos rectos.

	//se guardan las coordenadas 'x' e 'y' en variables separadas
	float p0x=p0.x;
	float p0y=p0.y;
	float p1x=p1.x;
	float p1y=p1.y;

	//se calcula la distancia en 'x' y en 'y' de cada punto	
	float dx = p1.x - p0.x; 
	float dy = p1.y - p0.y;
	
	/*
	fabs(value) calcula el valor absoluto.
	primer condición del if: si la distancia en 'x' es mayor o igual que en 'y' 
	=> tendencia horizontal, aumenta de a un paso en 'x'
	*/
	
	if( fabs(dx) >= fabs(dy) ){ // |dx|>=|dy|		
		//se redondean las coordenadas.
		float x0=round(p0x),
			  y0=round(p0y),			
			  x1=round(p1x); 		
		glm::vec2 aux(x0,y0);	
		
		if(x1==x0) return; //si los puntos coinciden en x corta
		
		float pendiente = dy/dx; //pendiente de la recta	
		
		//acá se indica si la recta aumenta o decrece en el eje x.
		//si es mayor a 0 el dx significa que la recta aumenta en x, si no disminuye.
		
		if(dx>0){  //si x1 se encuentra hacia la derecha de x0	
			while((++x0)<x1){ //pre incremento x0 en 1				
				glm::vec2 aux(x0,y0);				
				//se dibuja el pixel en la coordenada auxiliar
				paintPixel (aux); 				
				//la coordenada Y aumenta en relación a la pendiente y se redondea
				y0 += pendiente; 
				round(y0);
			}					
		}else{ //si x1 se encuentra hacia la izquierda de x0
			while((--x0)>x1){
				glm::vec2 aux(x0,y0);
				paintPixel (aux);
				y0 -= pendiente;
				round(y0);								
			}
		}			
	}else{    //|dy| > |dx|
		/*
		En cambio,si no es tendencia horizontal se dice que es tendencia vertical por lo que
		aumenta de a un paso en 'y'.
		*/		
		float x0=round(p0x),
			  y0=round(p0y), 
			  y1=round(p1y);		
		glm::vec2 aux(x0,y0);			
		float m=dx/dy;		
		if(dy>0){ 		//si el p1 está por encima del p0	
			while(++y0<y1){ //se aumenta en 1 y0 mientras sea menor a y1	
				glm::vec2 aux(x0,y0);
				paintPixel (aux);					
				x0 += m; //x0 aumenta en el valor de la pendiente por cada paso de y0
				round(x0); //se redondea el valor de x0
			}			
		}else{  //si el p0 está por encima del p1, el paso inverso al if con entrada true.	
			while((--y0)>y1){
			glm::vec2 aux(x0,y0);
			paintPixel (aux);
			x0-=m;
			round(x0);				
			}			
		}
	}
}
void drawCurve(paintPixelFunction paintPixel, curveEvalFunction evalCurve) {
	/// @todo: implementar algun algoritmo de rasterizacion de curvas
	
	/* 
	- evalCurve es una función que representa a la curva paramétrica
	- retorna el punto de la curva y su derivada para ese valor
	 El tipo de retorno de la función f es un struct con dos campos, p y d
	que se corresponden a punto y derivada respectivamente
	- El parámetro t va de 0 a 1
	*/	
	
	//se establece el parámetro t en 0.	
	//se crea una variable r de tipo auto que contiene un punto y la derivada
	
#if 0
	//VERSION CON ERROR USANDO SUBDIVISIONES Y DDA
	float t=0.f;
	auto r = evalCurve (t);	
	
	glm::vec2 p0;
	glm::vec2 p1;
	
	while(t<1){	
		r = evalCurve (t);
		float p0x = r.p.x;
		float p0y = r.p.y;
		glm::vec2 p0(p0x,p0y);
		
		t+=0.05; //20 subdivisiones
		
		r = evalCurve (t);		
		float p1x =r.p.x;
		float p1y = r.p.y;
		glm::vec2 p1(p1x,p1y);
		
		drawSegment(paintPixel,p0,p1);		
	}	
#else
	//version con goto
	float t = 0.f; // parámetro de avance t
	float dt= 0.f; // valor de derivada que utilizaremos para aumentar en t
	
	while(t < 1.f) {  // 0<= t <1
		
		auto r=evalCurve(t);
		//derivadas x(t) e y(t)
		float dyt=r.d.y;
		float dxt=r.d.x;		
		// queremos saber hacia dónde se movio más para dimensionar dt y cubrir enteramente la distancia
		if(fabs(dxt) < fabs(dyt)){ 
			dt=1/fabs(r.d.y);} 
		else{
			dt=1/fabs(r.d.x);
		}		
		
		next_point: //etiqueta del cálculo del punto siguiente
		
		//Se guarda el punto siguiente a la instancia t actual
		auto next_r=evalCurve(t+dt);  //el siguiente r será t + dt
		glm::vec2 next_p=next_r.p; //punto next_p
		//se localiza el punto intermedio entre el punto actual y el siguiente
		auto diff=round(next_p) - round(r.p);		
		/*si el pixel a pintar se alejo mas de una unidad tanto en x o en y, dt se reduce a la mitad
		y se vuelve a calcular el punto siguiente y el punto intermedio.
		esto es para evitar que haya pixeles sin pintar ya que sobrepasa el valor del entero 1.
		*/			
		if( fabs(diff.x)>1.f || fabs(diff.y)>1.f){		
			dt=dt*0.5f;
			goto next_point;//vuelve a recalcular el punto siguiente con el nuevo dt
		}				
		//si el punto está dentro del pixel, se avanza el t en dt unidad
		t+=dt;		
		//forma de evitar que se pinte dos veces un mismo pixel de un punto coincidente contiguo
		if(round(r.p)!=round(next_p)){
			paintPixel(round(r.p));			
		}		
	}
#endif	
	/*
	//VERSION CORREGIDA 
	//version sin goto
	
	auto t = 0.f; // parámetro de avance t
	auto dt= 0.f; // derivada
	
	while(t<1.f) {  //t<t1
		
		auto r=evalCurve(t);
		//derivadas parciales en x e y
		
		// fabs()= valor absoluto
		if(fabs(r.d.x) < fabs(r.d.y)){ 
			dt=1/fabs(r.d.y);} 
		else{
			dt=1/fabs(r.d.x);
		}		
		
		//Se guarda el punto siguiente a la instancia t actual
		auto next_r=evalCurve(t+dt);  //el siguiente r será t + 
		glm::vec2 next_p=next_r.p; //punto next_p
		
		//se localiza el punto intermedio entre el punto actual y el siguiente
		auto diff=round(next_p) - round(r.p); 
		
		/*si el pixel a pintar se alejo mas de una unidad tanto en x o en y, dt se reduce a la mitad
		y se vuelve a calcular el punto siguiente y el punto intermedio.
			
		
		while( fabs(diff.x)>1.f || fabs(diff.y)>1.f){		
			dt=dt*0.5f;
			next_r=evalCurve(t+dt);  
			glm::vec2 next_p=next_r.p;			
			diff=round(next_p) - round(r.p);
		}			
		
		t+=dt;
		
		//forma de evitar que se pinte dos veces un mismo pixel de un punto coincidente contiguo
		if(round(r.p)!=round(next_p)){
			paintPixel(round(r.p));			
		}
		
	}	
*/
	
	
	
}






