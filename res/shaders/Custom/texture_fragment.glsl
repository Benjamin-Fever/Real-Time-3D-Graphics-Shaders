#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;
uniform sampler2D uTexture;
uniform sampler2D uNormalMap;
uniform bool uUseNormalMap;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;

// framebuffer output
out vec4 fb_color;

// Map texture to the model
void main() {
	// calculate texture
	vec4 tex = texture(uTexture, f_in.textureCoord);
	vec4 norm = texture(uNormalMap, f_in.textureCoord);
	

	// calculate lighting (hack)
	vec3 normal = normalize(f_in.normal);
	if (uUseNormalMap)
		normal = normalize(norm.xyz * 2.0f - 1.0f);
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normal, eye));
	vec3 color = mix(uColor / 4, uColor, light) * tex.rgb;

	// output to the frambuffer
	fb_color = vec4(color, 1);
}