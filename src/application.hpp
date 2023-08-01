
#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"

#define PI 3.14159265358979323846264338327950288419716939937510

// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model {
	GLuint shader = 0;
	std::vector<cgra::gl_mesh> meshs;
	glm::vec3 color {0.95, 0.65, 0.55};
	glm::mat4 modelTransform{1.0};
	GLuint texture;

	// Lighting Data
	glm::vec3 lightPos {-10, 10, 7};
	glm::vec3 lightColor {1};
	float ambientStrength = 0.055f;
	float diffuseStrength = 0.7f;
	float specularStrength = 0.3f;
	float roughness = 0.1f;
	float refraction = 0.5f;

	void draw(const glm::mat4 &view, const glm::mat4 proj);
};

// Main application class
//
class Application {
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow *m_window;

	// oribital camera
	float m_pitch = .86f;
	float m_yaw = -.86f;
	float m_distance = 20;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// geometry
	basic_model m_model;
	int subdiv = 20;
	int geometryMode = 0;
	int shaderMode = 0;
	int radius = 5;
	int outerRadius = 10;
	std::string shaderFrag[4] = {"//res//shaders//color_frag.glsl", "//res//shaders//Core//frag.glsl", "//res//shaders//Completion//frag.glsl", "//res//shaders//color_frag.glsl"};
	std::string shaderVert[4] = {"//res//shaders//color_vert.glsl", "//res//shaders//Core//vert.glsl", "//res//shaders//Completion//vert.glsl", "//res//shaders//color_vert.glsl"};

public:
	// setup
	Application(GLFWwindow *);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);

	// geometry
	void drawGeometry();
	void sphereLatlong();
	void sphereFromCube();
	void generateCubeFace(cgra::mesh_builder *mb, glm::mat3 transformMatrix);
	void torusLatLong();

	double map(double value, double inMin, double inMax, double outMin, double outMax);
};