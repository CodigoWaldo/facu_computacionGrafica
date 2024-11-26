in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;
in vec4 fragPosLightSpace;

// propiedades del material
uniform sampler2D depthTexture; // ambient and diffuse components
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 emissionColor;
uniform float shininess;
uniform float opacity;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "calcPhong.frag"

float calcShadow(vec4 fragPosLightSpace) {
	// calcular la pos del fragmento desde el pto de vista de la luz
	vec3 projCoords = vec3(fragPosLightSpace) / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5; // mapear valores validos al intervalo [0..1]
	
	// obtener el valor de profundidad del fragmento desde el pto de vista de la luz
	if(projCoords.z > 1.0 || projCoords.z < 0.0) return -2.0; // fuera de znear/zfar
	float currentDepth = projCoords.z;
	
	// obtener el valor de profundidad de lo que "ve" la luz (que puede no ser este fragmento)
	float closestDepth = texture(depthTexture, projCoords.xy).r;
	
	// comparar la profundidad del fragmento con la que ve la luz para ver si es lo mismo
//	return currentDepth > closestDepth ? 1.0 : 0.0;
	
	// solucion 1: version con bias adaptable
	vec3 normal = normalize(fragNormal);
	vec3 lightDir = normalize(vec3(lightVSPosition) - fragPosition);
	float bias = max(0.025 * (1.0 - dot(normal, lightDir)), 0.0025);
//	return currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
//	 solucion 2: version con bias adaptable y suavizado de bordes 
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			vec2 syt = projCoords.xy + vec2(x, y) * texelSize;
			if(syt.x>1 || syt.x<0 ||syt.y<0 ||syt.y>1) return -1.0;
			float pcfDepth = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
		}
	}
	return shadow/9.0;
	
}

vec4 calcColor(vec3 textureColor) {
	// obtener el color segun el sombreado (phong)
	vec3 phong = calcPhong(lightVSPosition, lightColor, ambientStrength,
						   ambientColor*textureColor, diffuseColor*textureColor,
						   specularColor, shininess);
	// determinar si estamos en zona de luz o sombre
	float shadow = calcShadow(fragPosLightSpace);
	if(shadow < 0) { // fallo... fuera del frustrum que genero el shadow map
		if (shadow==-1) return vec4(1.f,0.f,0.f,1.f); // laterales -> rojo
		else            return vec4(0.f,1.f,0.f,1.f); // znear/zfar -> verde
	}
	// color final: shadow==0 -> luz (phong) ... shadow==1 -> sombra (solo ambiente)
	vec3 ambientComponent = ambientColor*textureColor*ambientStrength;
	vec3 lighting = mix(phong, ambientComponent, shadow);
	return vec4(lighting+emissionColor,opacity);
}
