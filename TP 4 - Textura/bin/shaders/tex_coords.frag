# version 330 core
in vec2 fragTexCoords;

out vec4 fragColor;

void main() {
	/// @@TODO: Use tex coords as color

	
	
    /// clamp devuelve un valor entre lo establecido, en este caso 0 y 1
	float s = clamp(fragTexCoords.x, 0.0f, 1.0f);
	float t = clamp(fragTexCoords.y, 0.0f, 1.0f);
	
	
	
	///mapear s->R, t->G, B=0, alfa=1
    
	fragColor = vec4(s, t, 0.0f, 1.0f);
		
	
}
