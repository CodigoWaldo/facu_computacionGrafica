# version 330 core

uniform vec2 flarePosition;
uniform vec2 screenSize;

uniform float flareSize;

out vec4 fragColor;

void main() {
	float halfSize = flareSize*0.5f;
	
	// Vec2 factor, 1 = inside flare, 0 = outside flare
	float xFactor = 1.f - trunc(min(abs(gl_FragCoord.x - flarePosition.x)/halfSize, 1.f));
	float yFactor = 1.f - trunc(min(abs(gl_FragCoord.y - flarePosition.y)/halfSize, 1.f));
	
	// 0 - fragment outside flare dimensions
	// 1 - fragment inside flare dimensions
	float opacity = 0.3f * xFactor * yFactor;
	
	// Paint a square in the area of screen which involves the flare
	fragColor = vec4(1.f,  0.f, 0.f, opacity);
}

