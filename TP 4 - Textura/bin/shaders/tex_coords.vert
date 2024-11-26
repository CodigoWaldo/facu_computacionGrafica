#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 fragTexCoords;

void main() {
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition,1.f);
	fragTexCoords = vertexTexCoords;
}
