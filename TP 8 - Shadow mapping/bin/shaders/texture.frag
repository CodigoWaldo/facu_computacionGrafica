# version 330 core

uniform sampler2D colorTexture;

#include "funcs/calcColor.frag"

void main() {
	vec4 base_color = texture(colorTexture,fragTexCoords);
	fragColor = calcColor(vec3(base_color));
}

