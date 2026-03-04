#version 330 core
#define NR_OF_POINT_LIGHTS 4

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float innerCutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_OF_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform vec3 viewPos;

uniform bool flashLight;

// functions
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	// output color value
	vec3 result = vec3(0.0);

	// properties
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	// add directional light
	result += calcDirLight(dirLight, normal, viewDir);

	// add point lights
	for (int i = 0; i < NR_OF_POINT_LIGHTS; i++) 
		result += calcPointLight(pointLights[i], normal, FragPos, viewDir);
	
	// add spotlights
	if (flashLight)
		result += calcSpotLight(spotLight, normal, FragPos, viewDir);

	FragColor = vec4(result, 1.0f);
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	// negation - vector ponting TOWARDS the light
	vec3 lightDir = normalize(-light.direction);

	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// result
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// attentuation
	float distance = length(light.position - fragPos);
	float attentuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// result
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
	return (attentuation * (ambient + diffuse + specular));
}


vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// cutoff calculations
	float theta = dot(lightDir, normalize(-light.direction)); // angle between fragment and spotlight direction
	float epsilon = light.innerCutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	if (theta > light.outerCutOff)
	{
		// diffuse
		float diff = max(dot(normal, lightDir), 0.0);
		
		// specular
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		
		// result
		vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
		vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)) * intensity;
		vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)) * intensity;

		return (ambient + diffuse + specular);
	}
	else return (light.ambient *  vec3(texture(material.diffuse, TexCoords)));
}