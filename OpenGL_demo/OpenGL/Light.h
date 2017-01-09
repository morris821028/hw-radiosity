#ifndef LIGHT_H
#define LIGHT_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

class Light
{
public:
	Light();
	Light(glm::vec3 pos, glm::vec3 color);
	~Light();
	glm::vec3 pos;
	glm::vec3 color;
};

#endif

