#include <bits/stdc++.h>
using namespace std;

const float eps = 1e-6;

struct Tri {
	// foreground color, background color: RGB
	float fc[3], bc[3];
	// 3 vertex position: x, y, z
	float vxyz[3][3];
	// the normal vector of vertex: x, y, z
	float nxyz[3][3];
};

struct Pt {
    float x, y, z;
    Pt(float x = 0.f, float y = 0.f, float z = 0.f):
    	x(x), y(y), z(z) {}	
    bool operator==(const Pt &a) const {
    	return fabs(x - a.x) < eps && fabs(y - a.y) < eps && fabs(z - a.z) < eps;
    }
    bool operator!=(const Pt &a) const {
    	return !(a == *this);
    }
    bool operator<(const Pt &a) const {
        if (fabs(x - a.x) > eps)
            return x < a.x;
        if (fabs(y - a.y) > eps)
            return y < a.y;
        if (fabs(z - a.z) > eps)
            return z < a.z;
        return false;
    }
	Pt extendMin(const Pt &b) const {
		return Pt(min(x, b.x), min(y, b.y), min(z, b.z));
	}
	Pt extendMax(const Pt &b) const {
		return Pt(max(x, b.x), max(y, b.y), max(z, b.z));
	}
};

void readLine(string line, float f[]) {
	stringstream sin(line);
	for (int i = 0; i < 9; i++) {
		assert(sin >> f[i]);
	}
}

void writeColorfulJson(string ofileName, vector<Tri> &A) {
	ofstream fout(ofileName);
	if (!fout) {
		fprintf(stderr, "Output file path failed");
		exit(1);
	}
	// output json format
	fout << "{" << endl;

	fout << "\t\"colorful\": true," << endl;

	fout << "\t\"vertexPositions\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			sz++;
			fout << t.vxyz[j][0] << "," << t.vxyz[j][1] << "," << t.vxyz[j][2];
		}
	}
	fout << "]," << endl;

	fout << "\t\"vertexNormals\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			sz++;
			fout << t.nxyz[j][0] << "," << t.nxyz[j][1] << "," << t.nxyz[j][2];
		}
	}
	fout << "]," << endl;

	fout << "\t\"vertexTextureCoords\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			sz++;
			fout << 0.5f << "," << 0.5f << "," << 0.5f;
		}
	}

	fout << "]," << endl;

	fout << "\t\"vertexFrontColors\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			sz++;
			fout << t.fc[0]/255.0 << "," << t.fc[1]/255.0 << "," << t.fc[2]/255.0;
		}
	}
	fout << "]," << endl;

	fout << "\t\"vertexBackColors\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			sz++;
			fout << t.bc[0]/255.0 << "," << t.bc[1]/255.0 << "," << t.bc[2]/255.0;
		}
	}
	fout << "]," << endl;

	fout << "\t\"indices\" : [";
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fout << ",";
			fout << sz;
			sz++;
		}
	}
	fout << "]" << endl;

	fout << "}" << endl;

}
void readColorfulTriangle(string ifileName, string ofileName) {
	ifstream fin(ifileName);

	// read triangle
	string objName;
	vector<Tri> A;
	Pt leftPt, rightPt;
	bool hasLeft = false, hasRight = false;
	while (getline(fin, objName)) {
		assert(objName == "Triangle " && "Make sure all object triangle based");
		string line;
		float f[9];
		Tri t;	
		// read vertex information
		for (int i = 0; i < 3; i++) {
			assert(getline(fin, line));
			readLine(line, f);
			for (int j = 0, k = 0; j < 3; j++, k++)
				t.vxyz[i][k] = f[j];
			for (int j = 3, k = 0; j < 6; j++, k++)
				t.fc[k] = f[j];
			for (int j = 6, k = 0; j < 9; j++, k++)
				t.bc[k] = f[j];
			for (int k = 0; k < 3; k++)
				t.nxyz[i][k] = 0;
			// extend boundary	
			Pt tmpPt(t.vxyz[i][0], t.vxyz[i][1], t.vxyz[i][2]);
			if (hasLeft)
				leftPt = leftPt.extendMin(tmpPt);
			else
				hasLeft = true, leftPt = tmpPt;
			if (hasRight)
				rightPt = rightPt.extendMax(tmpPt);
			else
				hasRight = true, rightPt = tmpPt;
		}
		// store triangle
		A.push_back(t);
	}

	// normalize bounding box 1 x 1 x 1
	const float maxSide = max(max(rightPt.x-leftPt.x, rightPt.y-leftPt.y), rightPt.z-leftPt.z);
	const float scale = 1.f / maxSide;
	const float view_scale = 10.f;
	for (int i = 0; i < A.size(); i++) {
		float left[3] = {leftPt.x, leftPt.y, leftPt.z};
		for (int j = 0; j < 3; j++)	 {
			for (int k = 0; k < 3; k++) {
				A[i].vxyz[j][k] -= left[k];
				A[i].vxyz[j][k] *= scale;
				A[i].vxyz[j][k] -= 0.5f;
				A[i].vxyz[j][k] *= view_scale;
			}
		}
	}
	// write 
	writeColorfulJson(ofileName, A);
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		if (!strcmp("--help", argv[1])) {
			fprintf(stderr, "\t\t--format\t\tModel File Format\n\t\t\t\t"
					"--format SIMPLE / --format COLOR\n");
			exit(0);
		}
		exit(1);
	}
	if (argc < 5) {
		fprintf(stderr, "./tri2json -i <filename.tri> -o <jsonname.json> [--format]\n");
		fprintf(stderr, "./tri2json --help\n");
		exit(1);
	}

	string ifileName = "", ofileName = "", fileFormat = "COLOR";

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i") && i+1 < argc) {
			ifileName = argv[i+1], i++;
		} else if (!strcmp(argv[i], "-o") && i+1 < argc) {
			ofileName = argv[i+1], i++;
		} else if (!strcmp(argv[i], "--format") && i+1 < argc) {
			assert((!strcmp(argv[i+1], "COLOR") || !strcmp(argv[i+1], "SIMPLE")) && "Please check file format");
			fileFormat = argv[i+1], i++;
		}
	}
	assert(ifileName.length() && "Please give input file name");
	assert(ofileName.length() && "Please give output file name");

	if (fileFormat == "COLOR") {
		readColorfulTriangle(ifileName, ofileName);
	} else {
		assert(false && "Unsupport format");
	}
	return 0;
}


