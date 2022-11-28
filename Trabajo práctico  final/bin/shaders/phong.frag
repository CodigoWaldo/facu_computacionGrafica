# version 330 core

in vec3 fragNormal;
in vec3 fragPosition;

in vec4 lightVSPosition_0;
in vec4 lightVSPosition_1;
in vec4 lightVSPosition_2;
in vec4 lightVSPosition_3;
in vec4 lightVSPosition_4;
in vec4 lightVSPosition_5;
in vec4 lightVSPosition_6;
in vec4 lightVSPosition_7;

//multiple point lights
uniform vec3 pointLight_0;
uniform vec3 pointLight_1;
uniform vec3 pointLight_2;
uniform vec3 pointLight_3;
uniform vec3 pointLight_4;
uniform vec3 pointLight_5;
uniform vec3 pointLight_6;
uniform vec3 pointLight_7;


// propiedades del material
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;
uniform vec3 emissionColor;
uniform float opacity;
uniform float shininess;

// propiedades de la luz
uniform float ambientStrength;
uniform float BrilloMax;
uniform float LightDistTravelReduction;
uniform vec3 lightColor;


layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BrightColor;



#include "funcs/calcPhong.frag"

void main() {
	vec3 phong_0 = calcPhong(lightVSPosition_0, lightColor,ambientColor, diffuseColor, specularColor, shininess);	
	vec3 phong_1 = calcPhong(lightVSPosition_1, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_2 = calcPhong(lightVSPosition_2, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_3 = calcPhong(lightVSPosition_3, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_4 = calcPhong(lightVSPosition_4, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_5 = calcPhong(lightVSPosition_5, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_6 = calcPhong(lightVSPosition_6, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	vec3 phong_7 = calcPhong(lightVSPosition_7, lightColor,ambientColor, diffuseColor, specularColor, shininess);
	
	// outputs final color
	fragColor =  vec4(phong_0+phong_1+phong_2+phong_3+phong_4+phong_5+phong_6+phong_7+emissionColor,opacity);
	
	// check whether fragment output is higher than threshold, if so output as brightness color	
	float brightness = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	
	if(brightness > BrilloMax)
		BrightColor = vec4(fragColor.rgb, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	
}
