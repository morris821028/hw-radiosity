#include "Light.h"
using namespace glm;

Light::Light(){
	
}
Light::Light(vec3 pos, vec3 color){
	this->pos = pos;
	this->color = color;
}

Light::~Light(){

}


