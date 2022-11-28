#include <algorithm>
#include <stdexcept>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "ObjMesh.hpp"
#include "Shaders.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Model.hpp"

#define VERSION 20221019
#define M_PI 3.141592
#include <iostream>
#include <cmath>
using namespace std;

std::vector<glm::vec2> generateTextureCoordinatesForBottle(const std::vector<glm::vec3> &v) {
	/// @todo: generar el vector de coordenadas de texturas para los vertices de la botella
	
	/*
	reciben como argumento de entrada un vector con los nodos (coordenadas)
	de la malla (std::vector<glm::vec3>), y deben retornar un nuevo vector del mismo tamaño con las coordenadas
	de textura para dichos nodos (std::vector<glm::vec2>). Cada elemento del vector de salida se corresponde con
	el elemento del vector de entrada de la misma posición (índice).		
	*/
		
	//se considera lo más adecuado aplicar el mapeo en dos partes
	
	//valores coordenadas (s,t) del texel de la imágen 
	float s=0.f;
    float t=0.f;
	
	//vector de coordenadas que se le asignan a los texels
	std::vector<glm::vec2> texturesCoordinates;
		
	//acá se declaran los valores de s y t para luego hacer un push al vector de coordenadas		
	for(int i=0;i<v.size();i++) { 		
		double theta = atan2(v[i].x,v[i].z);			
	    s=theta/(M_PI); //esta división permite variar a s entre 0 y 1		
		t=v[i].y*2.4 + 0.4; //formula del plano bY+d tal que b=grosor ,d = desplazamiento	
		glm::vec2 vec2Aux(s,t);
		//cout<<s<<" "<<t<<endl;		
		texturesCoordinates.push_back(vec2Aux);	
	}	
	return texturesCoordinates;
	
}

std::vector<glm::vec2> generateTextureCoordinatesForLid(const std::vector<glm::vec3> &v) {
	/// @todo: generar el vector de coordenadas de texturas para los vertices de la tapa
	float s=0.f;
	float t=0.f;
	
	std::vector<glm::vec2> texturesCoordinates;	
	
	for(int i=0;i<v.size();i++) { 		
		
		//se considera adecuado aplicar el método de mapeo plano
		s=5*v[i].x +0.5;
		t=-5*v[i].z +0.5;
		glm::vec2 vec2Aux(s,t);
		
		texturesCoordinates.push_back(vec2Aux);	
	}	
	return texturesCoordinates;
}

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Texturas");
	setCommonCallbacks(window);
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); glad_glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.6f,0.6f,0.8f,1.f);
	Shader shader("shaders/texture");
	
	// load model and assign texture
	auto models = Model::load("bottle",Model::fKeepGeometry);
	Model &bottle = models[0], &lid = models[1];
	bottle.buffers.updateTexCoords(generateTextureCoordinatesForBottle(bottle.geometry.positions),true);
	bottle.texture = Texture("models/label.png",true,false); //direccion, true = repeat, false =clamping
	lid.buffers.updateTexCoords(generateTextureCoordinatesForLid(lid.geometry.positions),true);
	lid.texture = Texture("models/lid.png",false,false);

	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		setMatrixes(shader);
		shader.setLight(glm::vec4{2.f,-2.f,-4.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.15f);
		for(Model &mod : models) {
			mod.texture.bind();
			shader.setMaterial(mod.material);
			shader.setBuffers(mod.buffers);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			mod.buffers.draw();
		}
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

