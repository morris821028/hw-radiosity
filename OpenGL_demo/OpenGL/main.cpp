#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GL/glfw3.h>

#include "controls.hpp"
#include "TRIModel.h"
#include "Shader.h"
#include "Transform.h"
#include "Light.h"

enum{GAURAUD}; // enumerate the shading styles
#define SHADER_NUM 1 // determine number of shaders

using namespace glm;
using namespace std;

vector<TRIModel> models; // triangle model list

void loadModel(const char* filename){
	TRIModel model;
	model.loadFromFile(filename);
	models.push_back(model);
}

int main( void ){
    GLFWwindow* window;
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(1024, 768, "Demo", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
	if(glewInit() != GLEW_OK)
		cout << "Failed to initial GLEW"<<endl;
   

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 


	// Read the models
	loadModel("models/balls.tri");


	// Create and compile our GLSL program from the shaders
	Shader shader[SHADER_NUM];
	shader[GAURAUD].CreateProgram("vs.vertexshader", "fs.fragmentshader");
	//shader[GAURAUD].CreateProgram("gauraud_test.vertexshader", "gauraud_test.fragmentshader");
	// get associated handle for uniform variables and attribute variables in the shader program
	string univars[6]={"PVM", "V", "M", "normat", "ligposwld", "ligcolor"};
	string attrvars[3]={"vertposmdl", "vertnormdl", "vertcolor"};
	for(int s = 0; s < SHADER_NUM; ++s){
		for(int i = 0; i < 6; ++i)
			shader[s].GetUniformLocation(univars[i]);
		for(int i = 0; i < 3; ++i)
			shader[s].GetAttributeLocation(attrvars[i]);
	}


	// Create vertex buffer objects and bind them to attribute handle
	VBO vertbuf(3, GL_FLOAT), colorbuf(3, GL_FLOAT), normalbuf(3, GL_FLOAT);
	int numvert = 0;
	for(size_t i = 0; i < models.size(); ++i)
		numvert += models[i].vertices.size();
	GLsizeiptr reqmem = numvert * sizeof(vec3);
	vertbuf.Alloc(reqmem);
	colorbuf.Alloc(reqmem);
	normalbuf.Alloc(reqmem);
	for(size_t i = 0; i < models.size(); ++i){
		GLsizeiptr memsize = models[i].vertices.size() * sizeof(vec3);
		vertbuf.Append(memsize, &(models[i].vertices[0]));
		colorbuf.Append(memsize, &(models[i].forecolors[0]));
		normalbuf.Append(memsize, &(models[i].normals[0]));
	}
	for(int s = 0; s < SHADER_NUM; ++s){
		shader[s].BindVBO(&vertbuf, "vertposmdl");
		shader[s].BindVBO(&colorbuf,"vertcolor");
		shader[s].BindVBO(&normalbuf, "vertnormdl");
	}
	
	// Create Lights, and bind them to uniform vector handle
	Light lig(vec3(4, -4, 4), vec3(1.0, 1.0, 1.0));
	for(int s = 0; s < SHADER_NUM; ++s){
		shader[s].BindVector(&(lig.pos), "ligposwld");
		shader[s].BindVector(&(lig.color), "ligcolor");
	}

	// Create Transform, and bind them to uniform matrix handle
	
	Transform transform;
	for(int s = 0; s < SHADER_NUM; ++s){
		shader[s].BindMatrix(&(transform.pvm), "PVM");
		shader[s].BindMatrix(&(transform.modelmat), "M");
		shader[s].BindMatrix(&(transform.viewmat), "V");
		shader[s].BindMatrix(&(transform.normat), "normat");
	}
	//the viewing matrix (camera setting) is currently fixed
	//you can also change it in the render loop
	//transform.SetViewMatrix(vec3(0, 0, 6.0), vec3(0.0, 0.0, -40.0), vec3(0.0, 1.0, 0.0));
	// start to render
	vec3 rot(0.0, 0.0, 0.0);
	
	CtrlParam ctrlparam;

    while (!glfwWindowShouldClose(window))
    {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// the projection may changing
#if 1
		// update transformation according to keyboard/mouse control, see control.hpp/cpp
		CatchEvent(window);
		UpdateCtrlParams(ctrlparam);
		transform.SetProjectionMatrix(ctrlparam.dfov);	
		transform.SetViewMatrix(vec3(0, 0, 6.0), vec3(0.0, 0.0, -40.0), vec3(0.0, 1.0, 0.0), ctrlparam.rotview);
		transform.SetModelMatrix(ctrlparam.size, models[0].center, ctrlparam.trans, ctrlparam.rot);
#endif
#if 0
		transform.SetProjectionMatrix(0);
		transform.SetModelMatrix(0.002739, models[0].center, vec3(0.8, -0.7, 2.79), vec3(0, rot.y, 0) );
#endif
		transform.UpdatePVM();
		transform.UpdateNormalMatrix();
		shader[GAURAUD].Draw(GL_TRIANGLES, vertbuf.offset[0] / sizeof(vec3), models[0].vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();

		//rot.y += 5.0;
    }
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}

