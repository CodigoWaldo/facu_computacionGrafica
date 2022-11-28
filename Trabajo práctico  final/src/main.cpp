/*
Descripción:
Este juego fue desarrollado con el propósito de divertir como a la vez  demostrar las técnicas de bloom y 
detección de colisiones.
El código original fué escrito por el docente Pablo Novara para un trabajo práctico de la clase computación 
gráfica, luego fué modificado por el alumno Voegeli Walter (el autor de este texto).
Originalmente el juego era un auto de formula 1 recorriendo una pista y fué modificado para ser un juego tipo
"endless run", en el cual la nave debe recorrer la mayor distancia posible esquivando obstaculos y evitar
que la energia llegue a 0.

El juego implementa y destaca la técnica de efecto de iluminación bloom y diversos parámetros para modificar 
en tiempo real y observar el impacto en la visual del juego.

Los códigos y teoria implementada fue obtenida de la página: https://learnopengl.com/Advanced-Lighting/Bloom
GitHub Autor del mod: https://github.com/CodigoWaldo
*/

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Model.hpp"
#include "Window.hpp"
#include "Callbacks.hpp"
#include "Debug.hpp"
#include "Shaders.hpp"
#include "Car.hpp"
#define VERSION 20220901.2
#include <cmath>
#include "obstacle.hpp"
#include <cstdlib>
#include <ctime>
#include "LightModel.hpp"
#include <GL/gl.h>


// variables globales (no es lo mejor pero es cómodo)
// -----------------
bool top_view = false;
bool gamePause = false;
float LightDistTravelReduction = 0.1f;
bool MotionBlur = false;

//bloom options
bool Bloom= false;
bool showOnlyBrightParts= false;
int blurTipe= 0;
int amountBlurIt = 6;
float LightExposure = 0.7f;
float brightLimit = 2.5f;
float gammaCorrection = 2.2f;

// extra callbacks (atajos de teclado para cambiar de modo y camara)
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

// función para sensar joystick o teclado (si no hay joystick) y definir las 
// entradas para el control del auto
std::tuple<float,float,bool> getInput(GLFWwindow *window);

// matrices que definen la camara
glm::mat4 projection_matrix, view_matrix;

// función que renderiza la pista
// -----------------
void RenderTrack() {
	static Model track = Model::loadSingle("track",Model::fDontFit);
	static Shader shader("shaders/texture"); //se activa el shader	
	shader.use();	
	shader.setMatrixes(glm::mat4(1.f),view_matrix,projection_matrix);
	shader.setMaterial(track.material);	
	shader.setBuffers(track.buffers);
	track.texture.bind();
	static float aniso = -1.0f;
	if (aniso<0) glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); 
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	track.buffers.draw();
}
// función que actualiza las matrices que definen la cámara
// -----------------
void setViewAndProjectionMatrixes(const Car &car) {
	projection_matrix = glm::perspective( glm::radians(view_fov), float(win_width)/float(win_height), 0.1f,180.f );	
	glm::vec3 pos_auto = {car.x, 0.f, car.y};	
	if (!top_view) {		
		//Se establece un vector con la posición del auto en el plano 3D, donde Y es 0 ya que no varia.
		glm::vec3 pos_auto = {car.x, 0.f, car.y};			
		//Se descompone la ubicación de la cámara en 'x', 'y' y 'z'.			
		float camx = -1*cos(car.ang)*8; 
		float camy = 2.50f; //altura
		float camz = -1*sin(car.ang)*8;		
		view_matrix = glm::lookAt( pos_auto +glm::vec3{-5.f,2.f,0.f}, //eye camera pos
								  pos_auto+glm::vec3{8.f,0.0f,0.f}, //target position
								  glm::vec3{0.f,1.0f,0.f} ); //Up vector
		
	} else {		
		view_matrix = glm::lookAt( pos_auto + glm::vec3{-0.1f,0.5f,0.0f},
								  pos_auto+ glm::vec3{100.f,0.f,0.0f},
								  glm::vec3{cos(car.ang),0.f,sin(car.ang)} );	
	}	
}
//Algoritmo "Axis aligned bounding box" detector de colisiones 2D
// -----------------
bool CheckCollision(Car &one, obstacle &two){  
	// colision x-axis?
	bool collisionX = (one.x + one.sizeX >= two.x) && (two.x + two.sizeX >= one.x);
	// colisiony-axis?
	bool collisionY = (one.y + one.sizeY >= two.y) && (two.y + two.sizeY >= one.y);	
	// colision solo si se realiza en ambos ejes	
	if(collisionX && collisionY){two.colisionado=1;}	
	else{two.colisionado=0;}	
	return collisionX && collisionY;
}  
//Función para renderizar faros
// -----------------
void renderFaro(const LightModel &faro, std::vector<Model> &v_model,Shader &shader){
	shader.use();
	for(const Model &model : v_model){
		glm::mat4 move = glm::mat4 ( 1 ,     0.0f,    0, 0.0f, 
									0.0f,     1, 0.0f, 0.0f,
									0,     0.0f,    1, 0.0f,
									faro.xPos, 	0.0f,faro.yPos, 1.0f);
		
		glm::mat4 mod = glm::mat4(0.2f, 0.0f, 0.0f, 0.0f,
								  0.0f, 0.20f, 0.0f, 0.0f,
								  0.0f, 0.0f, 0.20f, 0.0f,
								  0.0f, 0.0f, 0.0f, 1.0f );				
		shader.setMatrixes(move*mod,view_matrix,projection_matrix);			
		// Configurar luces y material	
		shader.setLight(glm::vec3{1,1,1}, 0.0f);		
		shader.setMaterial(model.material);		
		// Enviar geometria
		shader.setBuffers(model.buffers);		
		model.buffers.draw();
	}
}
//Función para renderizar bstaculos
// -----------------
void renderObstacle(const obstacle &obs, std::vector<Model> &v_model,bool isReflected,Shader &shader){		
	shader.use();		
	for(const Model &model : v_model){					
		glm::mat4 move = glm::mat4 (  0.5f   ,     0.0f,    0, 0.0f, 
									0.0f,     0.50f, 0.0f, 0.0f,
									0,     0.0f,    0.50f, 0.0f,
									obs.x, 	0.5f,obs.y, 1.0f);			
		glm::mat4 reflectionMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
		if(isReflected){move = reflectionMatrix * move;}	//matrix de reflección			
		shader.setMatrixes(move,view_matrix,projection_matrix);				
		// configurar luces y material	  
		glm::vec3 luzObs = glm::vec3{1,1,1};
		if(obs.colisionado==1){luzObs = glm::vec3{1,0,0};}			
		shader.setLight( luzObs, 0.2f);		//pos (X Y Z), color, intesidad
		shader.setMaterial(model.material);		
		//Enviar geometria
		shader.setBuffers(model.buffers);				
		model.buffers.draw();		
	}
}
//Función para renderizar un obstaculo
// -----------------
void renderShip(const Car &car, const std::vector<Model> &shipModel, bool isReflected,Shader &shader) {			
	shader.use(); //se activa el shader				
	for(const Model &model : shipModel){				
		glm::mat4 move = glm::mat4 (	          1, 	0.0f, 	(sin(car.ang)), 	0.0f, 
									0.0f,	1.0f, 		      0.0f,     0.0f,
									-(sin(car.ang)),    0.0f,                1,     0.0f,
									car.x, 	0.3f, 			 car.y,     1.0f);					
		//matrix de reflección
		glm::mat4 reflectionMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
		if(isReflected){move = reflectionMatrix * move;}				
		shader.setMatrixes(move,view_matrix,projection_matrix);			
		// setup light and material	
		shader.setLight( glm::vec3{1,1,1}, 0.2f);				
		shader.setMaterial(model.material);					
		// send geometry
		shader.setBuffers(model.buffers);		
		model.buffers.draw();
	}	
}
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
// main: crea la ventana, carga los modelos e implementa el bucle principal
int main() { 	
	// Inicializa la ventana y los callbacks
	// ------------------
	Window window(win_width,win_height,"Endless run game.",true);		
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback); 
	
	// configura OPENGL
	// ------------------	
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.f,0.f,0.f,0.3f);//COLOR DE FONDO
	
	// Carga de modelos
	// ------------------	
	std::vector<Model> shipModel = Model::load("body", Model::fDontFit);
	std::vector<Model> obstacleModel = Model::load("obstacle", Model::fDontFit);
	std::vector<Model> faroModel = Model::load("lowpoly", Model::fDontFit);	
	
	//carga pista
	Track track("mapa.png",100,100);	
	
	//"nave"
	Car car(-90,0,0); //posicion inicial
	Car car2(-90,0,0); //reflejo	
	
	//Posicionamiento inicial de obstaculos
	// ------------------	
	int cantidadObstaculos = 40;
	obstacle obstaculos[cantidadObstaculos]; //ARREGLO OBSTACULOS	
	obstacle obstaculosReflex[cantidadObstaculos]; //ARREGLO OBSTACULOS REFLEJADOS
	float xRandPos=-100.f;
	float zRandPos=0.f;	
	srand(time(NULL));		
	for(int i = 0; i < cantidadObstaculos; ++i) {	
		zRandPos= -6.f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.f+6.f))); //Pos aleatoria en z
		obstaculos[i].posInicial(xRandPos,zRandPos);	
		obstaculosReflex[i].posInicial(xRandPos,zRandPos);	
		xRandPos+=10;
	}		
	//RE-Acomodamiento incial de obstaculos a modo tutorial	
	for(int i=0;i<10;i++) { 
		obstaculos[i].Move(100.f);	
		obstaculosReflex[i].Move(100.f);
	}
	obstaculos[10].Move(0.f);
	obstaculosReflex[10].Move(0.f);
	
	//Posicion de FAROS
	// ------------------	
	LightModel faros[8]; 	
	float xFaroPos = -75.f;
	float yFaroPos = -7.5f;	
	for(int i=0;i<8;i++) { 		
		faros[i].Position(xFaroPos,yFaroPos);		
		yFaroPos=yFaroPos*-1;
		faros[++i].Position(xFaroPos,yFaroPos);		
		xFaroPos+=100;		
	}
	
	// Declaracion de la posicion de las luces (coinciden con los faros)
	// ------------------
	glm::vec4 lightPosition_0 = glm::vec4 {-125.5f,2.2f,-7.0f,1.0f};
	glm::vec4 lightPosition_1 = glm::vec4 {-125.5f,2.2f,7.0f,1.0f};
	glm::vec4 lightPosition_2 = glm::vec4 {-75.5f,2.2f,-7.0f,1.0f};
	glm::vec4 lightPosition_3 = glm::vec4 {-75.5f,2.2f,7.0f,1.0f};
	glm::vec4 lightPosition_4 = glm::vec4 {24.5f,2.2f,-7.0f,1.0f};
	glm::vec4 lightPosition_5 = glm::vec4 {24.5f,2.2f,7.0f,1.0f};
	glm::vec4 lightPosition_6 = glm::vec4 {124.5f,2.2f,-7.0f,1.0f};
	glm::vec4 lightPosition_7 = glm::vec4 {124.5f,2.2f,7.0f,1.0f};	
	
	// Inicializacion de shaders
	// ------------------
	Shader shaderPhong("shaders/phong");			
	Shader shaderBloom("shaders/Bloom");	
	Shader shaderBlur("shaders/Blur");			
	
	// Configuración de framebufffers
	// Requeridos para bloom
	// -------------------
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	
	// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win_width, win_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	
	// create and attach depth buffer (renderbuffer)	
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,win_width, win_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	
	//array of color attachment enums
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// ping-pong-framebuffer for blurring
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,win_width, win_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
	
	// Variables de tiempo
	// ------------------
	FrameTimer ftime;
	double accum_dt = 0.0;
	double lap_time = 0.0;
	double last_lap = 0.0;		
	float LIFE=200.f;
	
	// loop de ejecución
	// ------------------
	do {		
		// framebuffers config 	
		// -------------------	
		
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);				
		// Shader Set
		// -------------			
		shaderPhong.use();
		shaderPhong.setUniform("BrilloMax",brightLimit);
		shaderPhong.setUniform("LightDistTravelReduction",LightDistTravelReduction);	
		
		
		//render en el offscreen framebuffer
		// -------------------	
		{ //scope para minimizar en el IDE
			
			//Tiempo
			double elapsed_time=0;			
			elapsed_time = ftime.newFrame();				
			accum_dt += elapsed_time;
			if(LIFE>0){
				if(!gamePause){
					lap_time += elapsed_time;	
				}
			}
			auto in = getInput(window);
			
			//renderizado de los fragmentos	EN ORDEN 
			// -------------------				
			//"reflejo" de la nave
			renderShip(car2,shipModel,1,shaderPhong);     
			// OBSTACULOS reflejados
			for(int i = 0; i < cantidadObstaculos; ++i){renderObstacle(obstaculosReflex[i],obstacleModel,true,shaderPhong);}
			//Pista
			RenderTrack(); 	
			//faros
			for(int i = 0; i < 8; ++i) {renderFaro(faros[i],faroModel,shaderPhong);}
			//Obstaculos
			for(int i = 0; i < cantidadObstaculos; ++i){renderObstacle(obstaculos[i],obstacleModel,false,shaderPhong);}
			//nave	
			renderShip(car,shipModel,0,shaderPhong); 		
			
			//movimiento del personaje
			// -------------------	
			while (accum_dt>1.0/60.0 ) { 					
				if(!gamePause){	
					car.Move(track, std::get<0>(in), std::get<1>(in), std::get<2>(in));     // main character
					car2.Move(track, std::get<0>(in), std::get<1>(in), std::get<2>(in));	//reflejo				
				}
				accum_dt-=1.0/60.0;				
				setViewAndProjectionMatrixes(car);
			}			
			if(!gamePause){	
				//actualizar posición de obstaculos
				int obstacleToChange=(round((car.x)+100) / 10)-1;			
				//if(obstacleToChange<=1) obstacleToChange=21;			
				zRandPos= -6.f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.f+6.f)));			
				obstaculos[obstacleToChange].Move(zRandPos);				
				obstaculosReflex[obstacleToChange].Move(zRandPos);					
				if(obstacleToChange<20){
					obstaculos[obstacleToChange+20].Move(zRandPos);
					obstaculosReflex[obstacleToChange+20].Move(zRandPos);
				}		
			}
			
			//seteo de las posiciones de las luces.
			// ------------------
			shaderPhong.setPosition_0(lightPosition_0); 	
			shaderPhong.setPosition_1(lightPosition_1); 
			shaderPhong.setPosition_2(lightPosition_2); 
			shaderPhong.setPosition_3(lightPosition_3); 
			shaderPhong.setPosition_4(lightPosition_4); 
			shaderPhong.setPosition_5(lightPosition_5); 
			shaderPhong.setPosition_6(lightPosition_6); 
			shaderPhong.setPosition_7(lightPosition_7); 
			
			// Comprobar colisiones
			// -------------------	
			for(int i=0;i<cantidadObstaculos;i++) { 
				if(CheckCollision(car,obstaculos[i])){	
					LIFE-=1;			
				};			
			}	
			
			// Modificador de dificultad
			// -------------------	
			if( car.distanciaRecorrida < 1.f){	
				
			}else if(2.f > car.distanciaRecorrida && car.distanciaRecorrida >= 1.f){
				car.updateTopSpeed(133.f);
				car2.updateTopSpeed(133.f);
			}else if(3.f > car.distanciaRecorrida && car.distanciaRecorrida >= 2.f){
				car.updateTopSpeed(166.f);
				car2.updateTopSpeed(166.f);
			}else {
				car.updateTopSpeed(200.f);
				car2.updateTopSpeed(200.f);
			}
			
			//GAME OVER	
			if(LIFE<=0){
				LIFE=0;
				car.updateTopSpeed(0.01f);
				car2.updateTopSpeed(0.01f);
			}
		}		
		
		// blur bright fragments with two-pass Gaussian Blur 
		// --------------------------------------------------		
		bool horizontal = true;
		bool first_iteration = true;		
		shaderBlur.use();	
		
		switch(blurTipe){	//Elección del tipo de blur
		case 0:
			for (unsigned int i = 0; i < amountBlurIt; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				shaderBlur.setUniform("horizontal", horizontal?1:0);
				glBindTexture(GL_TEXTURE_2D, i==0 ? colorBuffers[1] : pingpongColorbuffers[horizontal?0:1]);  // bind texture of other framebuffer (or scene if first iteration)
				renderQuad();
				horizontal = !horizontal;
			}				
			break;
		case 1:
			for (unsigned int i = 0; i < amountBlurIt; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				shaderBlur.setUniform("horizontal", 0);
				glBindTexture(GL_TEXTURE_2D, i==0 ? colorBuffers[1] : pingpongColorbuffers[horizontal?0:1]);  // bind texture of other framebuffer (or scene if first iteration)
				renderQuad();
				horizontal = !horizontal;				
			}	
			break;
		case 2:
			for (unsigned int i = 0; i < amountBlurIt; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				shaderBlur.setUniform("horizontal", 1);
				glBindTexture(GL_TEXTURE_2D, i==0 ? colorBuffers[1] : pingpongColorbuffers[horizontal?0:1]);  // bind texture of other framebuffer (or scene if first iteration)
				renderQuad();
				horizontal = !horizontal;				
			}	
			break;
		default:{}
		}
		
		// Se establece el framebuffer default 
		// y se muestran las texturas procesadas y almacenadas en los buffers
		// --------------------------------	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderBloom.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		if(!showOnlyBrightParts){ glActiveTexture(GL_TEXTURE1);} //SOLAMENTE MUESTRA LAS PARTES BRILLOSAS
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		shaderBloom.setUniform("scene", 0);
		shaderBloom.setUniform("bloomBlur", 1);
		shaderBloom.setUniform("bloom", Bloom);
		shaderBloom.setUniform("exposure", LightExposure);
		shaderBloom.setUniform("gammaCorrection", gammaCorrection);		
		renderQuad();
		
		// Menú de opciones
		// -------------------		
		window.ImGuiDialog("Control",[&](){			
			ImGui::LabelText("","A = izq.",0.f);	
			ImGui::LabelText("","D = der.",0.f);	
			ImGui::LabelText("","Esc = Cerrar.",0.f);	
			ImGui::Checkbox("Camara primera persona",&top_view);	
			ImGui::Checkbox("PAUSA (P)",&gamePause);	
			ImGui::LabelText("","",0.f);	
			ImGui::Separator();
			
			ImGui::LabelText("","---------JUEGO----------",0.f);		
			ImGui::LabelText("","Tiempo: %f s",lap_time);	
			ImGui::LabelText("","velocidad: %f",car.vel);	
			ImGui::LabelText("","kilometros: %f",car.distanciaRecorrida);
			ImGui::LabelText("","Energia: %f",LIFE);	
			ImGui::LabelText("","",0.f);	
			ImGui::Separator();			
			
			ImGui::LabelText("","Opciones visuales",0.f);				
			ImGui::SliderFloat("Exposicion de luz",&LightExposure,0.f,2.f);
			ImGui::SliderFloat("Correccion de gamma",&gammaCorrection,0.f,5.f);
			ImGui::SliderFloat("Reductor de distancia del vector luz",&LightDistTravelReduction,0.01f,5.f);
			ImGui::LabelText("","",0.f);	
			ImGui::Separator();
			
			ImGui::LabelText("","Bloom",0.f);	
			ImGui::Checkbox("Bloom (B)",&Bloom);		
			ImGui::SliderFloat("Limite de brillo",&brightLimit,0.f,10.f);
			ImGui::Checkbox("Mostrar solo partes brillantes",&showOnlyBrightParts);	
			ImGui::LabelText("","Opciones two steps gaussian blur:",0.f);	
			ImGui::SliderInt("Blur iteraciones",&amountBlurIt,1.f,100.f);
			ImGui::SliderInt("Blur tipo",&blurTipe,0,2);
			ImGui::LabelText("","0: Blur combinado",0.f);
			ImGui::LabelText("","1: Vertical Blur",0.f);
			ImGui::LabelText("","2: Horizontal Blur",0.f);
			ImGui::LabelText("","",0.f);
			ImGui::Separator();
			
			//ImGui::LabelText("","---------Motion blur options----------",0.f);	
			//ImGui::Checkbox("Motion Blur",&MotionBlur);			
			
			ImGui::LabelText("","'Ship' posicion",0.f);				
			ImGui::LabelText("","x  pos: %f",car.x);
			ImGui::LabelText("","z  pos: %f",car.y); 
			ImGui::LabelText("","",0.f);
			ImGui::Separator();
			
			ImGui::LabelText("","Version: 11/2022",0.f);	
			
		});	
		
		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW eventsglfwSwapBuffers(window);
		glfwPollEvents();	
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window));	
}
//Detección de pulsación de teclas adicionales al movimiento
// -------------
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) 
{
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'P': gamePause = !gamePause; break; 		
		case 'B': Bloom = !Bloom; break; 		
		case 'C': top_view = !top_view; break; 		
		}
	}
}
//Detección de pulsación de teclas de movimiento
// -------------
std::tuple<float,float,bool> getInput(GLFWwindow *window)
{
	float acel = 0.f, dir = 0.f; bool analog = false;
	//if (glfwGetKey(window,GLFW_KEY_UP)   ==GLFW_PRESS) acel += 0.1f;
	//if (glfwGetKey(window,GLFW_KEY_UP)   ==GLFW_PRESS) acel += 1.f;
	if (glfwGetKey(window,GLFW_KEY_DOWN) ==GLFW_PRESS) acel -= 1.f;
	if (glfwGetKey(window,GLFW_KEY_RIGHT)==GLFW_PRESS) dir = 1.f;
	if (glfwGetKey(window,GLFW_KEY_LEFT) ==GLFW_PRESS) dir = -1.0f;
	
	int count;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
	if (count) { dir = axes[0]; acel = -axes[1]; analog = true; }
	return std::make_tuple(acel,dir,analog);
}
