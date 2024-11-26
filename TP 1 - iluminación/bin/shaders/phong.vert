#version 330 core

//ENTRADA de información del vertice
in vec3 vertexPosition;
in vec3 vertexNormal;
//ENTRADA  DE VALORES QUE SE COMPARTEN, ej la pos de la luz del sol
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
//SALIDA al fragment
out vec3 fragPosition;
out vec3 fragNormal;

void main() {
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(vertexPosition,1.f);
	gl_Position = projectionMatrix * vmp;
	
	vec4 aux = modelMatrix*vec4(vertexPosition,1.f);
	fragPosition = vec3(aux)/aux.w;
	fragNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
}
