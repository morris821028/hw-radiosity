#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "Shader.h"
using namespace std;
using namespace glm;
Shader::Shader(){
}
Shader::~Shader(){
	glDeleteProgram(program);
}
void Shader::CreateProgram(const char * vertfile,const char * fragfile){

	// Create the shaders
	GLuint vertshader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragshader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vertcode;
	std::ifstream vertshaderstr(vertfile, std::ios::in);
	if(vertshaderstr.is_open()){
		std::string Line = "";
		while(getline(vertshaderstr, Line))
			vertcode += "\n" + Line;
		vertshaderstr.close();
	}else
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertfile);

	// Read the Fragment Shader code from the file
	std::string fragcode;
	std::ifstream fragshaderstr(fragfile, std::ios::in);
	if(fragshaderstr.is_open()){
		std::string Line = "";
		while(getline(fragshaderstr, Line))
			fragcode += "\n" + Line;
		fragshaderstr.close();
	}



	GLint result = GL_FALSE;
	int infologlen;



	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertfile);
	char const * vertexsrcptr = vertcode.c_str();
	glShaderSource(vertshader, 1, &vertexsrcptr , NULL);
	glCompileShader(vertshader);

	// Check Vertex Shader
	glGetShaderiv(vertshader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertshader, GL_INFO_LOG_LENGTH, &infologlen);
	if ( infologlen > 0 ){
		std::vector<char> verterrmsg(infologlen+1);
		glGetShaderInfoLog(vertshader, infologlen, NULL, &verterrmsg[0]);
		printf("%s\n", &verterrmsg[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragfile);
	char const * FragmentSourcePointer = fragcode.c_str();
	glShaderSource(fragshader, 1, &FragmentSourcePointer , NULL);
	glCompileShader(fragshader);

	// Check Fragment Shader
	glGetShaderiv(fragshader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragshader, GL_INFO_LOG_LENGTH, &infologlen);
	if ( infologlen > 0 ){
		std::vector<char> fragerrmsg(infologlen+1);
		glGetShaderInfoLog(fragshader, infologlen, NULL, &fragerrmsg[0]);
		printf("%s\n", &fragerrmsg[0]);
	}



	// Link the program
	printf("Linking program\n");
	program = glCreateProgram();
	glAttachShader(program, vertshader);
	glAttachShader(program, fragshader);
	glLinkProgram(program);

	// Check the program
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologlen);
	if ( infologlen > 0 ){
		std::vector<char> progerrmsg(infologlen+1);
		glGetProgramInfoLog(program, infologlen, NULL, &progerrmsg[0]);
		printf("%s\n", &progerrmsg[0]);
	}

	glDeleteShader(vertshader);
	glDeleteShader(fragshader);
}

void Shader::GetUniformLocation(string var){
	handle[var] = glGetUniformLocation(program, var.c_str());
}

void Shader::GetAttributeLocation(string var){
	handle[var] = glGetAttribLocation(program, var.c_str());
}

void Shader::Draw(GLenum mode, GLint first, GLsizei count){
	glUseProgram(program);
	// send vertex buffer object to GPU
	for(map<VBO*, string>::iterator it = hvbo.begin(); it != hvbo.end(); ++it){
		VBO* vboptr = it->first;
		int h = handle[hvbo[vboptr]];
		//cout << "vertex handle:"<<h<<", attribute:"<<hvbo[vboptr]<<endl;
		glEnableVertexAttribArray(h);
		glBindBuffer(GL_ARRAY_BUFFER, vboptr->vbo);
		glVertexAttribPointer(
		h, // The attribute we want to configure
		vboptr->step,                  // size
		vboptr->type,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*) 0           // array buffer offset
		);	
	}
	for(map<mat4*, string>::iterator it = hmat.begin(); it != hmat.end(); ++it){
		mat4* matptr = it->first;
		int h = handle[hmat[matptr]];
		glUniformMatrix4fv(h, 1, GL_FALSE, &(*matptr)[0][0]);
	}
	for(map<vec3*, string>::iterator it = hvec.begin(); it != hvec.end(); ++it){
		vec3* vecptr = it->first;
		int h = handle[hvec[vecptr]];
		glUniform3f(h, (*vecptr).x, (*vecptr).y, (*vecptr).z);
	}
	
	glDrawArrays(mode, first, count);
	for(map<VBO*, string>::iterator it = hvbo.begin(); it != hvbo.end(); ++it){
		VBO* vboptr = it->first;
		int h = handle[hvbo[vboptr]];
		glDisableVertexAttribArray(h);
	}
}

void Shader::BindVBO(VBO* vboptr, string var){
	hvbo[vboptr] = var;
}
void Shader::BindMatrix(mat4* matptr, string var){
	hmat[matptr] = var;
}
void Shader::BindVector(vec3* vecptr, string var){
	hvec[vecptr] = var;
}


