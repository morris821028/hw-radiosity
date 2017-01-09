#ifndef SHADER_H
#define SHADER_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include "VBO.h"
#include <map>
#include <string>


class Shader{
public:
	Shader();
	~Shader();
	void CreateProgram(const char* vertfile, const char* fragfile);
	void GetUniformLocation(std::string var);
	void GetAttributeLocation(std::string var);
	void BindVBO(VBO* vboptr, std::string var);
	void BindMatrix(glm::mat4* mat, std::string var);
	void BindVector(glm::vec3* vec, std::string var);
	void Draw(GLenum mode, GLint first, GLsizei count);
	GLuint program;
	std::map<std::string, int> handle;
	std::map<VBO*, std::string> hvbo;
	std::map<glm::mat4*, std::string> hmat;
	std::map<glm::vec3*, std::string> hvec;
};

#endif