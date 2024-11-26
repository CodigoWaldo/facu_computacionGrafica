in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;
in vec4 fragPosLightSpace; //Posición del fragmento transformado al espacio de la luz

uniform sampler2D depthTexture;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 emissionColor;
uniform float shininess;
uniform float opacity;


uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "calcPhong.frag"

float calcShadow(vec4 fragPosLightSpace) {
	
	//Transformación de coordenadas del fragmento al espacio del shadow map
	vec3 projCoords = vec3(fragPosLightSpace) / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	if(projCoords.z > 1.0 || projCoords.z < 0.0) return -2.0; //si está fuera del rango no se procesa
	
	float currentDepth = projCoords.z; //profundidad del fragmento actual		
	float closestDepth = texture(depthTexture, projCoords.xy).r; // profundidad del shadow map con respecto a la posición del fragmento
	
	///------------------------ 
	
	/// SOLUCIÓN PARCIAL AL SHADOW ACNE 
	// se utiliza la solución del shadow bias.
	
	// Solución utilizando un bias fijo.
	// No es muy efectivo cuando el angulo de la normal de superficie y la luz es muy pronunciado (cercano a 90°)
	//float bias = 0.005;
	//float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  

	// Solución utilizando un bias variable.
	// El bias se hace mas pequeño a medida que el angulo entre la normal del fragmento y la luz se acerca a 0
	float bias = max(0.01 * (1.0 - dot(fragNormal, (lightVSPosition.xyz - fragPosition ))), 0.005); 
		
	
	/// SOLUCIÓN PARCIAL A LA SOMBRA SERRUCHADA
	// Combinación entre aumentar la resolución del mapa de profundidad y el método de alisado percentage-closer filtering (PCF).		
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthTexture, 0);	
	for(int x = -1; x <= 1; ++x) // -1 , 0 , 1
	{
		for(int y = -1; y <= 1; ++y) // -1 , 0 , 1
		{
			float pcfDepth = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize ).r; 
			shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0; // 3*3
	
	///------------------------	
	return currentDepth > closestDepth ? (1.0*shadow) : 0.0;

	//Comparación: si la profundidad del fragmento es mayor que la indicada por el shadow map entonces se asume que está en sombra
}

vec4 calcColor(vec3 textureColor) {
	vec3 phong = calcPhong(lightVSPosition, lightColor, ambientStrength,
						   ambientColor*textureColor, diffuseColor*textureColor,
						   specularColor, shininess);
	
	float shadow = calcShadow(fragPosLightSpace);
	if(shadow < 0) {
		if (shadow==-1) return vec4(1.f,0.f,0.f,1.f);
		else            return vec4(0.f,1.f,0.f,1.f);
	}
	
	vec3 ambientComponent = ambientColor*textureColor*ambientStrength;
	vec3 lighting = mix(phong, ambientComponent, shadow);
	return vec4(lighting+emissionColor,opacity);
}
