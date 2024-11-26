#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragNormal;

void main() {
	fragNormal = mat3(transpose(inverse(viewMatrix*modelMatrix))) * vertexNormal;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition,1.f);
}
