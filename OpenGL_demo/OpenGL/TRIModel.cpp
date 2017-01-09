#include <glm/glm.hpp>
#include "TRIModel.h"

using namespace std;
using namespace glm;

bool TRIModel::loadFromFile(const char* fileName){
	char tmpstr[100] = "";
	double max[3]={-DBL_MAX, -DBL_MAX, -DBL_MAX};
	double min[3]={DBL_MAX, DBL_MAX, DBL_MAX};
	FILE* inFile = fopen(fileName, "r");
	if(!inFile){
		cout << "Can not open object File \"" << fileName << "\" !" << endl;
		return false;
	}

	cout <<"Loading \"" << fileName << "\" !" << endl;
	while(fscanf(inFile,"%s",tmpstr) != EOF){
		vec3 tmpvert;
		vec3 tmpn;
		int tmpint[6];
		vec3 fcolor;
		vec3 bcolor;
		fscanf(inFile,"%d %d %d %d %d %d",&tmpint[0], &tmpint[1], &tmpint[2], &tmpint[3], &tmpint[4], &tmpint[5]);
		for(int i = 0; i < 3; ++i){
			fcolor[i] = tmpint[i] / 255.0;
			bcolor[i] = tmpint[i+3] / 255.0;
		}
		for(int i = 0; i < 3; i++){
			fscanf(inFile,"%f %f %f %f %f %f",&tmpvert.x,&tmpvert.y, &tmpvert.z, &tmpn.x, &tmpn.y, &tmpn.z);
			vertices.push_back(tmpvert);
			normals.push_back(tmpn);
			forecolors.push_back(fcolor);
			backcolors.push_back(bcolor);
			for(int j = 0; j < 3; j++){
				if(tmpvert[j] < min[j]){
					min[j] = tmpvert[j];
				}
				if(tmpvert[j] > max[j]){
					max[j] = tmpvert[j];
				}
			}
		}
	}
	for(int i = 0; i < 3; i++)
		center[i] = (min[i] + max[i]) / 2;
	return true;
}


TRIModel::TRIModel(){
}

TRIModel::~TRIModel(){
}