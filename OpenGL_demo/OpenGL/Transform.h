#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glfw3.h>


class Transform{
public:
	Transform();
	~Transform();
	void SetProjectionMatrix(GLfloat dfov);
	void SetViewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up);
	void SetViewMatrix(glm::vec3 eye, glm::vec3 center, glm::vec3 up, glm::vec3 rotv);
	void SetModelMatrix(float size, glm::vec3 center, glm::vec3 trans, glm::vec3 rot);
	void UpdateNormalMatrix();
	void UpdatePVM();
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	//provide some common transformations for rasterization
	glm::mat4 pvm;
	glm::mat4 viewmat;
	glm::mat4 projmat;
	glm::mat4 modelmat;
	glm::mat4 normat;
};

#endif

