# version 330 core

in vec3 fragNormal;

out vec4 fragColor;

void main() {
	if (fragNormal.x==0 && fragNormal.y==0) discard;
	fragColor = vec4(0.5,0.5,0.5,0.5);
}
