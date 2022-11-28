#version 330 core
//datos que vienen del modelo (obj)
in vec3 vertexPosition;
in vec3 vertexNormal;

//datos de la aplicacion main
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

//Multiples lightPosition
uniform vec4 lightPosition_0;
uniform vec4 lightPosition_1;
uniform vec4 lightPosition_2;
uniform vec4 lightPosition_3;
uniform vec4 lightPosition_4;
uniform vec4 lightPosition_5;
uniform vec4 lightPosition_6;
uniform vec4 lightPosition_7;


//salidas del shader

out vec4 lightVSPosition_0;
out vec4 lightVSPosition_1;
out vec4 lightVSPosition_2;
out vec4 lightVSPosition_3;
out vec4 lightVSPosition_4;
out vec4 lightVSPosition_5;
out vec4 lightVSPosition_6;
out vec4 lightVSPosition_7;

out vec3 fragPosition;
out vec3 fragNormal;

void main() {
	
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(vertexPosition,1.f);
	fragPosition = vec3(vmp)/vmp.w;
	gl_Position = projectionMatrix * vmp;
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal;
	
	//Multiples lightPosition
	lightVSPosition_0 = viewMatrix * lightPosition_0;
	lightVSPosition_1 = viewMatrix * lightPosition_1;
	lightVSPosition_2 = viewMatrix * lightPosition_2;
	lightVSPosition_3 = viewMatrix * lightPosition_3;
	lightVSPosition_4 = viewMatrix * lightPosition_4;
	lightVSPosition_5 = viewMatrix * lightPosition_5;
	lightVSPosition_6 = viewMatrix * lightPosition_6;
	lightVSPosition_7 = viewMatrix * lightPosition_7;
	
}
