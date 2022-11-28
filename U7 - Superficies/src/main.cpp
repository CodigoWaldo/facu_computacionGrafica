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
#include "SubDivMesh.hpp"
#include "SubDivMeshRenderer.hpp"

#define VERSION 20221013
#include <iostream>




// models and settings
std::vector<std::string> models_names = { "cubo", "icosahedron", "plano", "suzanne", "star" };
int current_model = 0;
bool fill = true, nodes = true, wireframe = true, smooth = false, 
	 reload_mesh = true, mesh_modified = false;

// extraa callbacks
void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods);

SubDivMesh mesh;
void subdivide(SubDivMesh &mesh);

int main() {
	
	// initialize window and setup callbacks
	Window window(win_width,win_height,"CG Demo",true);
	setCommonCallbacks(window);
	glfwSetKeyCallback(window, keyboardCallback);
	view_fov = 60.f;
	
	// setup OpenGL state and load shaders
	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); 
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.8f,0.8f,0.9f,1.f);
	Shader shader_flat("shaders/flat"),
	       shader_smooth("shaders/smooth"),
		   shader_wireframe("shaders/wireframe");
	SubDivMeshRenderer renderer;
	
	// main loop
	Material material;
	material.ka = material.kd = glm::vec3{.8f,.4f,.4f};
	material.ks = glm::vec3{.5f,.5f,.5f};
	material.shininess = 50.f;
	
	FrameTimer timer;
	do {
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		if (reload_mesh) {
			mesh = SubDivMesh("models/"+models_names[current_model]+".dat");
			reload_mesh = false; mesh_modified = true;
		}
		if (mesh_modified) {
			renderer = makeRenderer(mesh);
			mesh_modified = false;
		}
		
		if (nodes) {
			shader_wireframe.use();
			setMatrixes(shader_wireframe);
			renderer.drawPoints(shader_wireframe);
		}
		
		if (wireframe) {
			shader_wireframe.use();
			setMatrixes(shader_wireframe);
			renderer.drawLines(shader_wireframe);
		}
		
		if (fill) {
			Shader &shader = smooth ? shader_smooth : shader_flat;
			shader.use();
			setMatrixes(shader);
			shader.setLight(glm::vec4{2.f,1.f,5.f,0.f}, glm::vec3{1.f,1.f,1.f}, 0.25f);
			shader.setMaterial(material);
			renderer.drawTriangles(shader);
		}
		
		// settings sub-window
		window.ImGuiDialog("CG Example",[&](){
			if (ImGui::Combo(".dat (O)", &current_model,models_names)) reload_mesh = true;
			ImGui::Checkbox("Fill (F)",&fill);
			ImGui::Checkbox("Wireframe (W)",&wireframe);
			ImGui::Checkbox("Nodes (N)",&nodes);
			ImGui::Checkbox("Smooth Shading (S)",&smooth);
			if (ImGui::Button("Subdivide (D)")) { subdivide(mesh); mesh_modified = true; }
			if (ImGui::Button("Reset (R)")) reload_mesh = true;
			ImGui::Text("Nodes: %i, Elements: %i",mesh.n.size(),mesh.e.size());
		});
		
		// finish frame
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	} while( glfwGetKey(window,GLFW_KEY_ESCAPE)!=GLFW_PRESS && !glfwWindowShouldClose(window) );
}

void keyboardCallback(GLFWwindow* glfw_win, int key, int scancode, int action, int mods) {
	if (action==GLFW_PRESS) {
		switch (key) {
		case 'D': subdivide(mesh); mesh_modified = true; break;
		case 'F': fill = !fill; break;
		case 'N': nodes = !nodes; break;
		case 'W': wireframe = !wireframe; break;
		case 'S': smooth = !smooth; break;
		case 'R': reload_mesh=true; break;
		case 'O': case 'M': current_model = (current_model+1)%models_names.size(); reload_mesh = true; break;
		}
	}
}

// La struct Arista guarda los dos indices de nodos de una arista
// Siempre pone primero el menor indice, para facilitar la búsqueda en lista ordenada;
//    es para usar con el Mapa de más abajo, para asociar un nodo nuevo a una arista vieja
struct Arista {
	int n[2];
	
	Arista(int n1, int n2) {
		n[0]=n1; n[1]=n2;
		if (n[0]>n[1]) std::swap(n[0],n[1]);
	}
	Arista(Elemento &e, int i) { // i-esima arista de un elemento
		n[0]=e[i]; n[1]=e[i+1];
		if (n[0]>n[1]) std::swap(n[0],n[1]); // pierde el orden del elemento
	}
	const bool operator<(const Arista &a) const {
		return (n[0]<a.n[0]||(n[0]==a.n[0]&&n[1]<a.n[1]));
	}
};

// Mapa sirve para guardar una asociación entre una arista y un indice de nodo (que no es de la arista)
using Mapa = std::map<Arista,int>;

void subdivide(SubDivMesh &mesh) {	
	/*funcion SubDivMesh:	
	vector<Nodo> mesh.n : lista de nodos	
	vector<Elemento> mesh.e : lista de elementos
		cada elemento puede ser de 3 o 4 vertices
				
	mesh.agregarElemento
	mesh.reemplazarElemento		
	--------------------------------------------------	
	The Catmull–Clark algorithm is a technique used in 3D computer graphics to create 
	curved surfaces by using subdivision surface modeling.	
	
	Catmull–Clark surfaces are defined recursively, using the following steps	
	*/
	
	//  Los nodos originales estan en las posiciones 0 a #n-1 de m.n,
	//  Los elementos orignales estan en las posiciones 0 a #e-1 de m.e
	
	//  1) Por cada elemento, agregar el centroide (nuevos nodos: #n a #n+#e-1)	
	
	int n_n = mesh.n.size(); // número de nodos originales
	int n_e = mesh.e.size(); // número de elementos originales	 (CARAS)
	
	for(int j=0;j<mesh.e.size();j++) { //se recorre la lista de elementos
		Elemento e = mesh.e[j];        //se crea una copia del elemento j
		glm::vec3 suma = glm::vec3(0.f,0.f,0.f); //auxiliar vec3 suma
		for(int i=0;i<e.nv	;i++) {    //se recorre la cantidad de vertices de ese elemento (3 o 4)
			auto n= mesh.n[e.n[i]];  //se recorren los nodos  y luego se suman sus coordenadas
			suma+= n.p;
		}
		Nodo centroide= Nodo(suma/float(e.nv)); //se crea un nodo centroide promedio de las coordenadas
		mesh.n.push_back(centroide); //se agrega el centroide al vector de nodos de la malla
	}
	
	//  2) Por cada arista de cada cara, agregar un pto en el medio que es
	//      promedio de los vertices de la arista y los centroides de las caras 
	//      adyacentes. Aca hay que usar los elementos vecinos.
	//      En los bordes, cuando no hay vecinos, es simplemente el promedio de los 
	//      vertices de la arista
	//      Hay que evitar procesar dos veces la misma arista (como?) -> preguntando 
	//      Mas adelante vamos a necesitar determinar cual punto agregamos en cada
	//      arista, y ya que no se pueden relacionar los indices con una formula simple
	//      se sugiere usar Mapa como estructura auxiliar
	
	Mapa map_aux; //asociación entre una arista y un indice de nodo que no es de la arista
	
	for(int i=0; i<n_e; i++)  { 	 // se recorren la cantidad de caras originales (todavia no se generan nuevas)
		
		Elemento e = mesh.e[i];      // se guarda la cara en e
		for(int j=0;j<e.nv;j++){	 // se recorre cada vertice de esa cara (nv cant de vertices)
			Arista arista(e[j],e[j+1]);  //arista utilizando sus indices en el vector de nodos (extremos)
			if (map_aux.find(arista) == map_aux.end()){ //hay un indice asociado a la arista? existe en el mapa? ==end es no
				
				
				// NODOS FRONTERA: nodo frontera es un nodo que está en una arista del borde3 de la malla (que no tiene vecino)				
				Nodo nuevo = (mesh.n[e[j]].p + mesh.n[e[j+1]].p)/2.f; // promedio nodos arista
				
				// NODOS INTERIORES: el promedio entre los vértices de la arista y los centroides de sus dos elementos .
				if(e.v[j] != -1){ //si el elemento no es vecino
					Nodo centroideActual = mesh.n[n_n+ i]; 		 // centroide de la cara actual
					Nodo centroideVecino = mesh.n[n_n+ e.v[j]];   // centroide del vecino
					nuevo = (mesh.n[e[j]].p + mesh.n[e[j+1]].p + centroideVecino.p + centroideActual.p)/4.f; // promedio nodos arista y centroide
				}
				
				mesh.n.push_back(nuevo); //se agrega al vector de nodos el nodo nuevo resultante
				map_aux[arista]=mesh.n.size()-1; //se asignma al mapa el tamaño 
			}
		}
	}
		
	
	//  3) Armar los elementos nuevos
	//      Los quads se dividen en 4, (uno reemplaza al original, los otros 3 se agregan)
	//      Los triangulos se dividen en 3, (uno reemplaza al original, los otros 2 se agregan)
	//      Para encontrar los nodos de las aristas usar el mapa que armaron en el paso 2
	//      Ordenar los nodos de todos los elementos nuevos con un mismo criterio (por ej, 
	//      siempre poner primero al centroide del elemento), para simplificar el paso 4.
	
	for(int i=0; i<n_e; i++)  {   //cada elemento original
		
		Elemento e = mesh.e[i];  //se guarda el elemento en e
		
		
		int centroide =n_n+i;	 //su centroide será n_n + i por el orden
		
		int nodo0= e[0];		 //nodo 0 del elemento e
		int nodo1= map_aux[Arista(e[0],e[1])];  //nodo 1 de la cara buscado en el mapa por medio de su arista
		int nodo2= map_aux[Arista(e[0],e[-1])]; //nodo 2 de la cara buscado en el mapa por medio de su arista
		
		//centroide,
		mesh.reemplazarElemento(i,centroide,nodo2,nodo0,nodo1); //i es la posición en el arreglo de elementos
		//se reemplaza la cara original por la nueva más chica
		
		for(int j=1;j<e.nv;j++) { //por cada numero de vertices del elemento se calcula una nueva cara
			nodo0= e[j];
			nodo1= map_aux[Arista(e[j],e[j+1])];
			nodo2= map_aux[Arista(e[j],e[j-1])];
			
			mesh.agregarElemento(centroide,nodo2,nodo0,nodo1);
		}
		
	}
	
	/*
	Al modificar los elementos, se debe mantener en todo momento actualizado el arreglo e de cada nodo. Por ej, si se
	agrega un nuevo elemento, hay que registrar su índice en todos los nodos que lo componen. Para asegurarse de no
	omitir este paso y dejar la malla en un estado inconsistente, evite modificar directamente el arreglo de elementos de
	la malla, y utilice en cambio los métodos agregarElemento y reemplazarElemento que se aseguran de hacer ese
	”mantenimiento”.
	El otro ”mantenimiento” que podría ser necesario hacer al modificar los elementos es el de las relaciones de
	vecindad. En general es más complicado contemplarlo en cada modificación; por ello la clase permite no hacerlo
	con cada pequeña modificación, sino que provee un método makeVecinos que recalcula todas las vecindades
	*/
	
	mesh.makeVecinos();	//Actualiza el atributo v (lista de vecinos) de cada elemento y el atributo es_frontera de cada nodo
	
	
	//  4) Calcular las nuevas posiciones de los nodos originales
	//      Para nodos interiores: (4r-f+(n-3)p)/n
	//         f=promedio de nodos interiores de las caras (los agregados en el paso 1)
	//         r=promedio de los pts medios de las aristas (los agregados en el paso 2)
	//         p=posicion del nodo original
	//         n=cantidad de elementos para ese nodo
	//     Para nodos del borde: (r+p)/2
	//         r=promedio de los dos pts medios de las aristas
	//         p=posicion del nodo original
	//      Ojo: en el paso 3 cambio toda la SubDivMesh, analizar donde quedan en los nuevos 
	//      elementos (¿de que tipo son?) los nodos de las caras y los de las aristas 
	//      que se agregaron antes.
	
#if 1
	
	std::map<int,glm::vec3> prom_map; //mapa promedio de los pts medios de las aristas
	
	//del mapa auxiliar de aristas se le extrae 
	//it->first = arista , entonces (it->first).n[0] da como resultado el indice extremo (nodo) 0 
	//it->second = int , entonces mesh.n[it->second].p da como resultado las coordenadas del nodo indice apuntado
	
	for(auto it=map_aux.begin();it!=map_aux.end();it++) //se recorre el mapa aux de aristas originales
	{
		if( (mesh.n[(it->first).n[0]].es_frontera == mesh.n[(it->first).n[1]].es_frontera)){ //si ambos nodos son o no frontera
			prom_map[(it->first).n[1]] += mesh.n[it->second].p; //se suman las coordenadas 
			prom_map[(it->first).n[0]] += mesh.n[it->second].p; //se suman las coordenadas 
		}else{
			if(mesh.n[(it->first).n[0]].es_frontera){ //Si no coinciden frontera se pregunta si el nodo 0 es frontera
				prom_map[(it->first).n[1]]+= mesh.n[it->second].p;
			}
			else{
				prom_map[(it->first).n[0]]+= mesh.n[it->second].p;
			}
		}
	}

	for(int i=0;i<n_n;i++) { //se recorren los nodos originales 
		Nodo nodoActual= mesh.n[i]; //nodo posicion actual	
		auto p= nodoActual.p; //cooordenadas
		glm::vec3 nuevaPos= glm::vec3(0.f,0.f,0.f); //Aux
		glm::vec3 r=prom_map[i];  //vemos las coordenadas asociadas con el nodo indice i en el map
		
		if(nodoActual.es_frontera){
			r = r/2.f; 
			nuevaPos= (r+p)/2.f;
			
		}else{ //SI es un nodo interior
			
			r = r/float(nodoActual.e.size()); //dividido la cantidad de elementos que está ese nodo
			glm::vec3 f= glm::vec3(0.f,0.f,0.f); //inicialización de f
			
			for(int j=0;j<nodoActual.e.size();j++) { 
				Elemento e = mesh.e[nodoActual.e[j]]; //cara a la que pertenece
				f+=mesh.n[e[0]].p; //se suman las coordenadas de los nodos internos
			}
			
			auto n= float(nodoActual.e.size());	// n= cant elementos PARA ese nodo
			f=f/n; 
			nuevaPos= (4.f*r-f+(n-3.f)*p)/n;
		}
		mesh.n[i].p=nuevaPos;
	}
	
#endif	
	
	
	
	
	// tips:
	//   no es necesario cambiar ni agregar nada fuera de este método, (con Mapa como 
	//     estructura auxiliar alcanza)
	//   sugerencia: probar primero usando el cubo (es cerrado y solo tiene quads)
	//               despues usando la piramide (tambien cerrada, y solo triangulos)
	//               despues el ejemplo plano (para ver que pasa en los bordes)
	//               finalmente el mono (tiene mezcla y elementos sin vecinos)
	
	//   repaso de como usar un mapa:
	//     para asociar un indice (i) de nodo a una arista (n1-n2): elmapa[Arista(n1,n2)]=i;
	//     para saber si hay un indice asociado a una arista:  ¿elmapa.find(Arista(n1,n2))!=elmapa.end()?
	//     para recuperar el indice (en j) asociado a una arista: int j=elmapa[Arista(n1,n2)];
}
