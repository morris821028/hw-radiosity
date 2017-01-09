#include "Transform.h"
#include <iostream>
using namespace glm;
using namespace std;

Transform::Transform(){
}


Transform::~Transform(){
}

void Transform::SetViewMatrix(vec3 eye, vec3 center, vec3 up){
	viewmat = lookAt( eye, center, up);
}
void Transform::SetViewMatrix(vec3 eye, vec3 center, vec3 up, vec3 rotv){
	viewmat = lookAt( eye, center, up);
	mat4 rotmat = rotate(mat4(1.0), rotv.z, vec3(0.0, 0.0, 1.0));
	rotmat = rotate(rotmat, rotv.y, vec3(0.0, 1.0, 0.0));
	rotmat = rotate(rotmat, rotv.x, vec3(1.0, 0.0, 0.0));
	viewmat = rotmat * viewmat;
}

void Transform::SetModelMatrix(float size, vec3 center, vec3 trans, vec3 rot){
	mat4 resizemat = glm::scale(mat4(1.0), vec3(size, size, size));
	mat4 tranmat= translate(mat4(1.0), trans);
	//rotate along world axis: multiply matrix to the left
	//rotate along object axis: multiply matrix to the right
	mat4 rotmat = rotate(mat4(1.0), rot.z, vec3(0.0, 0.0, 1.0));
	rotmat = rotate(rotmat, rot.y, vec3(0.0, 1.0, 0.0));
	rotmat = rotate(rotmat, rot.x, vec3(1.0, 0.0, 0.0));
	modelmat = tranmat * rotmat * resizemat;
	modelmat = translate(modelmat, -center);
}

void Transform::SetProjectionMatrix(GLfloat dfov){
	// Initial Field of View
	float initfov = 45.0f;

	// Projection matrix : 45¢X Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	float fov = initfov + dfov;
	projmat = perspective(fov, 4.0f / 3.0f, 0.1f, 100.0f);
}

void Transform::UpdatePVM(){
	pvm = projmat * viewmat * modelmat;
}

void Transform::UpdateNormalMatrix(){
	normat = transpose( inverse(viewmat * modelmat) );
}
