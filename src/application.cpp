
// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"


using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uLightPos"), 1, value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shader, "uLightColor"), 1, value_ptr(lightColor));
	glUniform1f(glGetUniformLocation(shader, "uAmbientStrength"), ambientStrength);
	glUniform1f(glGetUniformLocation(shader, "uDiffuseStrength"), diffuseStrength);
	glUniform1f(glGetUniformLocation(shader, "uSpecularStrength"), specularStrength);
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	glUniform1f(glGetUniformLocation(shader, "uRoughness"), roughness);
	glUniform1f(glGetUniformLocation(shader, "uRefraction"), refraction);
	for (auto &mesh : meshs){
		mesh.draw(); // draw
	}
}


Application::Application(GLFWwindow *window) : m_window(window) {
	
	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	sphereLatlong();
}


void Application::render() {
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height); 

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = translate(mat4(1), vec3(0, 0, -m_distance))
		* rotate(mat4(1), m_pitch, vec3(1, 0, 0))
		* rotate(mat4(1), m_yaw,   vec3(0, 1, 0));


	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);

	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(340, 300), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// display current camera parameters
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderFloat("Pitch", &m_pitch, -pi<float>() / 2, pi<float>() / 2, "%.2f");
	ImGui::SliderFloat("Yaw", &m_yaw, -pi<float>(), pi<float>(), "%.2f");
	ImGui::SliderFloat("Distance", &m_distance, 0, 100, "%.2f", 2.0f);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);

	
	ImGui::Separator();


	ImGui::Text("Geometry Settings");
	int subdivMin = geometryMode == 0 ? 4 : 1;
	if (subdiv < subdivMin) { 
		subdiv = subdivMin; 
		drawGeometry();
	}
	if (ImGui::Combo("Mode", &geometryMode, "Core\0Complection\0Challenge"))
		drawGeometry(); 
	if (ImGui::SliderInt("subdiv Count", &subdiv, subdivMin, 100))
		drawGeometry();
	if (ImGui::SliderInt("Radius", &radius, 1, 50))
		drawGeometry();

	ImGui::Text("Lighting Settings");
    ImGui::SliderFloat3("Light Color", value_ptr(m_model.lightColor), 0, 1, "%.2f");
    ImGui::SliderFloat3("Light Position", value_ptr(m_model.lightPos), -100, 100, "%.2f");
    ImGui::SliderFloat("Ambient Strength", &m_model.ambientStrength, 0, 1, "%.3f");
    ImGui::SliderFloat("Diffuse Strength", &m_model.diffuseStrength, 0, 1, "%.3f");
    ImGui::SliderFloat("Specular Strength", &m_model.specularStrength, 0, 1, "%.3f");

	ImGui::Text("Material Settings");
	ImGui::SliderFloat3("Model Color", value_ptr(m_model.color), 0, 1, "%.2f");
	ImGui::SliderFloat("Roughness", &m_model.roughness, 0, 1, "%.3f");
	ImGui::SliderFloat("Refraction", &m_model.refraction, 0, 100, "%.3f");
    

	// finish creating window
	ImGui::End();
}

void Application::drawGeometry(){
	if (geometryMode == 0){
		// Sphere latlong
		sphereLatlong();
	}
	else if (geometryMode == 1){
		// Cube to Sphere
		sphereFromCube();
	}
	else if (geometryMode == 2){
		// Torus latlong
	}
}

void Application::sphereLatlong(){
	mesh_builder mb;
	
	// Generate Vertex's
	for (int lon = 0; lon < subdiv + 1; lon ++){
		double phi = map(lon, 0, subdiv, 0, PI);
		for (int lat =0; lat < subdiv + 1; lat ++){
			double theta = map(lat, 0, subdiv, 0, 2 * PI);
			// Calculate Positions
			double x = radius * cos(theta) * sin(phi);
			double z = radius * sin(theta) * sin(phi);
			double y = radius * cos(phi);

			// Calculate Normals
			double nx = x / radius;
			double ny = y / radius;
			double nz = z / radius;

			// Calculate UV's
			double uv1 = 0;
			double uv2 = 0;

			mb.push_vertex({{x,y,z},{nx, ny, nz},{uv1,uv2}});
		}
	}

	// Generate Indicies
	for (int lon = 0; lon < subdiv; lon ++){
		for (int lat =0; lat < subdiv; lat++){
			int k1 = lat * (subdiv + 1) + lon;
			int k2 = k1  +  subdiv + 1;

			// Triangle 1
			mb.push_index(k1);
			mb.push_index(k2);
			mb.push_index(k1 + 1);

			// Triangle 2
			mb.push_index(k2);
			mb.push_index(k2 + 1);
			mb.push_index(k1 + 1);
		}
	}

	m_model.meshs.clear();
	m_model.meshs.push_back(mb.build());
}

void Application::sphereFromCube(){
	mesh_builder mb;
	mesh_builder faces[6] = {};
	glm::mat3 faceMatrix[6] = {
		glm::mat3(vec3(1,0,-0.5), vec3(0,1,-0.5), vec3(0,0,0.5)),
		glm::mat3(vec3(1,0,-0.5), vec3(0,1,-0.5), vec3(0,0,-0.5)),
		glm::mat3(vec3(1,0,-0.5), vec3(0,0,-0.5), vec3(0,1,-0.5)),
		glm::mat3(vec3(1,0,-0.5), vec3(0,0,0.5), vec3(0,1,-0.5)),
		glm::mat3(vec3(0,0,-0.5), vec3(0,1,-0.5), vec3(1,0,-0.5)),
		glm::mat3(vec3(0,0,0.5), vec3(0,1,-0.5), vec3(1,0,-0.5)),
	};
	m_model.meshs.clear();
	for (int i = 0; i < 6; i++){
		generateCubeFace(faces + i, faceMatrix[i]);
		m_model.meshs.push_back(faces[i].build());
	}
}

void Application::generateCubeFace(mesh_builder *mb, glm::mat3 transformMatrix){
	float jumpSize = (float)radius / subdiv;

	// Generate vertices
	for (int y = 0; y < subdiv + 1; y++){
		for (int x = 0; x < subdiv + 1; x++){
			mesh_vertex mv;
			vec3 pos = vec3(x * jumpSize, y * jumpSize, radius);
			mv.pos = pos * transformMatrix;
			mv.pos = normalize(mv.pos);
			mv.pos *= radius;
			mv.norm = mv.pos;
			mb->push_vertex(mv);
			
		}
	}

	// Generate indicies
	for (int y = 0; y < subdiv; y++){
		for (int x = 0; x < subdiv; x++){
			int k1 = x + y * (subdiv + 1);
			int k2 = x + (y + 1) * (subdiv + 1);
			mb->push_index(k1);
			mb->push_index(k2);
			mb->push_index(k2+1);

			mb->push_index(k2+1);
			mb->push_index(k1);
			mb->push_index(k1+1);
		}
	}
}

double Application::map(double value, double inMin, double inMax, double outMin, double outMax) {
    return ((value - inMin) / (inMax - inMin)) * (outMax - outMin) + outMin;
}

void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_leftMouseDown) {
		vec2 whsize = m_windowsize / 2.0f;

		// clamp the pitch to [-pi/2, pi/2]
		m_pitch += float(acos(glm::clamp((m_mousePosition.y - whsize.y) / whsize.y, -1.0f, 1.0f))
			- acos(glm::clamp((float(ypos) - whsize.y) / whsize.y, -1.0f, 1.0f)));
		m_pitch = float(glm::clamp(m_pitch, -pi<float>() / 2, pi<float>() / 2));

		// wrap the yaw to [-pi, pi]
		m_yaw += float(acos(glm::clamp((m_mousePosition.x - whsize.x) / whsize.x, -1.0f, 1.0f))
			- acos(glm::clamp((float(xpos) - whsize.x) / whsize.x, -1.0f, 1.0f)));
		if (m_yaw > pi<float>()) m_yaw -= float(2 * pi<float>());
		else if (m_yaw < -pi<float>()) m_yaw += float(2 * pi<float>());
	}

	// updated mouse position
	m_mousePosition = vec2(xpos, ypos);
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset; // currently un-used
	m_distance *= pow(1.1f, -yoffset);
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)key, (void)scancode, (void)action, (void)mods; // currently un-used
}


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}
