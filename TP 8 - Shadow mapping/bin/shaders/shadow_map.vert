#version 330 core

in vec3 vertexPosition;

uniform mat4 modelMatrix;

uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;

///When we render the scene from the light's perspective we'd much rather 
///use a simple shader that only transforms the vertices to light space and not much more.

void main() {
	mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
		
	gl_Position = lightSpaceMatrix * modelMatrix * vec4(vertexPosition,1.f);
}
