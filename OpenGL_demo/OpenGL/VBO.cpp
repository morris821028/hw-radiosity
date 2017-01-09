#include "VBO.h"
#include <stdio.h>
#include <iostream>
using namespace std;

VBO::VBO(GLint step, GLenum type){
	this->step = step;
	this->type = type;
	glGenBuffers(1, &vbo);
	offset.push_back(0);
}


VBO::~VBO(){
	glDeleteBuffers(1, &vbo);
}

void VBO::Alloc(GLsizeiptr memsize){
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, memsize, NULL, GL_DYNAMIC_DRAW);
}

void VBO::Copy(GLsizeiptr memsize, GLvoid* data){
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, memsize, data, GL_STATIC_DRAW);
}

void VBO::Append(GLsizeiptr memsize, GLvoid* data){
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLsizeiptr loadsize = offset[offset.size()-1];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, loadsize, memsize, data);
	loadsize += memsize;
	offset.push_back(loadsize);	
}
