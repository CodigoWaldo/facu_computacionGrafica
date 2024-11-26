# version 330 core

in vec2 fragTexCoords;

uniform sampler2D flareTexture;

out vec4 fragColor;

void main() {	
	fragColor = texture(flareTexture,fragTexCoords);
}

