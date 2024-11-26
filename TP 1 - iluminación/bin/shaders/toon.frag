#version 330 core

// propiedades del material
uniform vec3 objectColor;
uniform float shininess;
uniform float specularStrength;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;
uniform vec4 lightPosition;

// propiedades de la camara
uniform vec3 cameraPosition;

// propiedades del fragmento interpoladas por el vertex shader
in vec3 fragNormal;
in vec3 fragPosition;

// resultado
out vec4 fragColor;

// phong simplificado
void main() {
	
	// ambient
	vec3 ambient = lightColor * ambientStrength * objectColor ;
	
	// diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(vec3(lightPosition)); // luz directional (en el inf.)
	//vec3 diffuse = lightColor * objectColor * max(dot(norm,lightDir),0.f);
	
	
	//versión con if	
	/*
	float prodPunto = dot(norm,lightDir);		
	if (prodPunto <= 0.33){		
	prodPunto = 0.33;
	}else if(prodPunto <= 0.66){
	prodPunto = 0.66;
	}else{
	prodPunto = 1.f;
	}	
	
	vec3 diffuse = lightColor  *objectColor * max( prodPunto ,0.f);
	*/
	//VERSION CON STEP . step(edge,stepvalue)  si edge > stepvalue => retorna 0
	float stepValue = max(dot(norm,lightDir),0.f);
	float lightStrength = 0.3*(step(0.2,stepValue) + step(0.4,stepValue) + step(0.7,stepValue));
	vec3 diffuse = lightColor * objectColor * lightStrength;
	
	
	// specular
	vec3 specularColor =   vec3(1.f,1.f,1.f) * specularStrength;
	vec3 viewDir = normalize(cameraPosition-fragPosition);
	vec3 halfV = normalize(lightDir + viewDir); // blinn
	
	vec3 specular = lightColor * specularColor  * step(0.1, max(pow( dot(norm,halfV),shininess),0))  ;
	
	// result
	fragColor = vec4(ambient+diffuse+specular,1.f);
}

