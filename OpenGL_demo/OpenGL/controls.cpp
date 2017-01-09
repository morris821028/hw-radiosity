// Include GLFW
#include <GL/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

float spd = 3.0f; // 3 units / second
float msespd = 0.005f;
float dfov = 0.0f;

float size = 0.01;
vec3 trans = vec3(0.0, 0.0, 0.0);
vec3 rot = vec3(0.0, 0.0, 0.0);
vec3 rotview = vec3(0.0, 0.0, 0.0);
int count = 0;
int trig = 5;
bool nokeypress = true;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	dfov -= yoffset;
	
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
		// Scale up
	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		size *= 1.1;
	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
		size *= 0.9;

	// Move object right
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		trans.x += 0.1;

	// Move object left
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		trans.x -= 0.1;

	// Move object up
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		trans.y += 0.1;

	// Move object down
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		trans.y -= 0.1;

	// Move object forward
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		trans.z += 0.1;

	// Move object backward
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		trans.z -= 0.1;

	// Rotate object in X axis
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		rot.x += 30;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		rot.x -= 30;

	//Rotate object in Y axis
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		rot.y += 30;
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		rot.y -= 30;

	//Rotate object in Z axis
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		rot.z += 30;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		rot.z -= 30;

	// Rotate view in X axis
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		rotview.x += 30;
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		rotview.x -= 30;

	//Rotate object in Y axis
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
		rotview.y += 30;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		rotview.y -= 30;

	//Rotate object in Z axis
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		rotview.z += 30;
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		rotview.z -= 30;
}
bool KeyPress(GLFWwindow* window, int key){
	if( glfwGetKey(window, key) == GLFW_PRESS ){
		nokeypress = false;
		count++;
	}
	if( count == trig ) {
		count = 0;
		return true;
	}
	return false;
}

bool KeyRelease(GLFWwindow* window, int key){
	return (glfwGetKey(window, key) == GLFW_RELEASE);
}
void Multiply(GLFWwindow* window, int forkey, int backey, float& param, float forval, float backval){
	if( KeyPress(window, forkey) )
		param *= forval;
	if( KeyPress(window, backey) )
		param *= backval;
}
void Add(GLFWwindow* window, int forkey, int backey, float& param, float forval, float backval){
	if( KeyPress(window, forkey) )
		param += forval;
	if( KeyPress(window, backey) )
		param += backval;
}
void KeyHandling(GLFWwindow* window){
	nokeypress = true;
	if( KeyPress(window, GLFW_KEY_ESCAPE) )
		glfwSetWindowShouldClose(window, GL_TRUE);

	//swing the parameters
	Multiply(window, GLFW_KEY_EQUAL, GLFW_KEY_MINUS, size, 1.1, 0.9);
	Add(window, GLFW_KEY_A, GLFW_KEY_B, trans.x, 0.1, -0.1);
	Add(window, GLFW_KEY_C, GLFW_KEY_D, trans.y, 0.1, -0.1);
	Add(window, GLFW_KEY_E, GLFW_KEY_F, trans.z, 0.1, -0.1);
	Add(window, GLFW_KEY_G, GLFW_KEY_H, rot.x, 30, -30);
	Add(window, GLFW_KEY_I, GLFW_KEY_J, rot.y, 30, -30);
	Add(window, GLFW_KEY_K, GLFW_KEY_L, rot.z, 30, -30);
	Add(window, GLFW_KEY_M, GLFW_KEY_N, rotview.x, 15, -15);
	Add(window, GLFW_KEY_O, GLFW_KEY_P, rotview.y, 15, -15);
	Add(window, GLFW_KEY_Q, GLFW_KEY_R, rotview.z, 15, -15);
	
	if(nokeypress)
		count = 0;
	/*printf("size:%f\n", size);
	printf("translation:%f %f %f\n", trans.x, trans.y, trans.z);
	printf("rotation:%f %f %f\n", rot.x, rot.y, rot.z);
	*/
}
void CatchEvent(GLFWwindow* window){
	KeyHandling(window);
	//glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

void UpdateCtrlParams(CtrlParam& ctrlparam){
	ctrlparam.dfov = dfov;
	ctrlparam.rot = rot;
	ctrlparam.rotview = rotview;
	ctrlparam.size = size;
	ctrlparam.trans = trans;
}

