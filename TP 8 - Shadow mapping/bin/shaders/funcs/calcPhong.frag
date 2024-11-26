vec3 calcPhong(vec4 lightPosition, vec3 lightColor, float ambientStrength,
			   vec3 ambientColor, vec3 diffuseColor, vec3 specularColor, float shininess)
{
	vec3 ambient = ambientColor * lightColor * ambientStrength;
	
	vec3 viewDir = normalize(-fragPosition);
	vec3 norm = normalize(dot(fragNormal,viewDir)<0? -fragNormal : fragNormal);
	vec3 lightDir = normalize(vec3(lightPosition)-fragPosition*lightPosition.w); // w=0 => directional
	vec3 diffuse = diffuseColor * max(dot(norm,lightDir),0.f) * lightColor;
	
	vec3 halfV = normalize(lightDir + viewDir);
	vec3 specular = specularColor * pow(max(dot(norm,halfV),0.f),shininess) * lightColor; // blinn
	
	return ambient+diffuse+specular;
}
