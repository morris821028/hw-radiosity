#ifndef TRIMODEL_H
#define TRIMODEL_H

#include <iostream>
#include <vector>




class TRIModel{
	
public:
	bool loadFromFile(const char* fileName);
	glm::vec3 center;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> forecolors;
	std::vector<glm::vec3> backcolors;
	TRIModel();
	~TRIModel();

};

#endif