#ifndef CONTROLS_HPP
#define CONTROLS_HPP
#include <GL/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct CtrlParam{
	float size;
	glm::vec3 trans;
	glm::vec3 rot;
	glm::vec3 rotview;
	float dfov;
};

void CatchEvent(GLFWwindow* window);
void UpdateCtrlParams(CtrlParam& ctrlparam);

#endif