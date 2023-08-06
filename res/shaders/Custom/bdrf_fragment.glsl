#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

// Light variables
uniform vec3 uColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform float uAmbientStrength;
uniform float uDiffuseStrength;
uniform float uSpecularStrength;
uniform float uRoughness;
uniform vec3 uRefraction;

vec3 norm;
vec3 lightDir;
vec3 viewDir;
vec3 halfwayDir;

float dotViewHalfDir;
float dotNormHalfDir;
float dotNormViewDir;
float dotNormLightDir;

float roughness2;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

// Calculate Ambient
vec3 ambient(){
	return uAmbientStrength * uLightColor;
}

vec3 diffuse(){
	// Oren-Nayar diffuse model implementation
	float A = 1.0 - 0.5 * (roughness2 / (roughness2 + 0.57));
	float B = 0.45 * (roughness2 / (roughness2 + 0.09));

	float cosThetaLightView = dot(lightDir, viewDir);
	float cosThetaLightNorm = dot(lightDir, norm);
	float cosThetaViewNorm = dot(viewDir, norm);

	// Calculate the Oren-Nayar component
	float alpha = max(cosThetaLightView, cosThetaViewNorm);
	float beta = min(cosThetaLightView, cosThetaViewNorm);

	float angleDiff = acos(cosThetaLightNorm);
	float angleView = acos(cosThetaViewNorm);

	float orenDiffuse = max(0.0, cosThetaLightNorm - A * B * max(0.0, sin(angleView) * tan(angleDiff)));
	vec3 final = vec3(uDiffuseStrength * orenDiffuse);
	return final;
}

vec3 specular(){
	// Calculate Freshnel effect with Schlick's approximation
	vec3 F = uRefraction + (1.0 - uRefraction) * pow(1.0 - dotViewHalfDir, 5.0);
	// Calculate Geometric Attenuation with Cook-Torrance
	float G1 = (2 * dotNormHalfDir * dotNormViewDir) / dotViewHalfDir;
	float G2 = (2 * dotNormHalfDir * dotNormLightDir) / dotViewHalfDir;
	float G = min(1.0, min(G1, G2));
	// Calculate Distribution with GGX
	float D = (roughness2 / (3.14159265359 * pow(dotNormHalfDir * dotNormHalfDir * (roughness2 - 1.0) + 1.0, 2.0)));
	vec3 specular = F * G * D / (4 * dotNormViewDir * dotNormLightDir);
	return uSpecularStrength * specular;
}

void main() {
	norm = normalize(f_in.normal);
	lightDir = normalize(uLightPos);
	viewDir = normalize(-f_in.position);
	halfwayDir = normalize(lightDir + viewDir);

	dotViewHalfDir = dot(viewDir, halfwayDir);
	dotNormHalfDir = dot(norm, halfwayDir);
	dotNormViewDir = dot(norm, viewDir);
	dotNormLightDir = dot(norm, lightDir);

	roughness2 = pow(uRoughness, 2);

	vec3 color = uColor;
	vec3 result = ambient() * color + diffuse() * color + specular();
	fb_color = vec4(result , 1.0);
}

