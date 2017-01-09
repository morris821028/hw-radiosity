#ifndef VBO_H
#define VBO_H
#include <GL/glew.h>
#include <vector>

using namespace std;


class VBO{
public:
	VBO(GLint step, GLenum type);
	~VBO();
	void Alloc(GLsizeiptr bytes);
	void Append(GLsizeiptr bytes, GLvoid* data);
	void Copy(GLsizeiptr bytes, GLvoid* data);
	vector<GLsizeiptr> offset;
	GLuint vbo;
	//used to describe the vbo content;
	GLint step; 
	GLenum type;

};

#endif

