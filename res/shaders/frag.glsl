#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

uniform vec3 uColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform float uAmbientStrength;
uniform float uDiffuseStrength;
uniform float uSpecularStrength;
uniform float uRoughness;
uniform float uRefraction;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;


/// Implement Cook-Torrence shading model
void main() {
	vec3 norm = normalize(f_in.normal);
	vec3 lightDir = normalize(uLightPos - f_in.position);
	vec3 viewDir = normalize(-f_in.position);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// Calculate the Fresnel term (Schlick approximation)
	float F = uRefraction + (1.0 - uRefraction) * pow(1.0 - dot(norm, viewDir), 5.0);

	// Calculate the microfacet distribution using GGX/Trowbridge-Reitz
	float D = (exp(-pow(tan(acos(dot(norm, halfwayDir))), 2) / pow(uRoughness, 2))) / (3.14 * pow(uRoughness, 2) * pow(cos(acos(dot(norm, halfwayDir))), 4));

	float G = dot(norm, lightDir) * dot(norm, viewDir);

	// Calculate the specular component
	vec3 specular = (D * G * F) / (4.0 * dot(norm, lightDir) * dot(norm, viewDir)) * uLightColor;

	// Calculate the diffuse component
	vec3 diffuse = uDiffuseStrength * uLightColor * dot(norm, lightDir);

	// Calculate the ambient component
	vec3 ambient = uAmbientStrength * uLightColor;

	// Combine ambient, diffuse, and specular components
	vec3 result = ambient + diffuse + uSpecularStrength * specular;

	// Apply Cook-Torrance shading and output the final color
	fb_color = vec4(result * uColor, 1.0);
}