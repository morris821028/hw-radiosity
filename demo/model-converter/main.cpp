#include <bits/stdc++.h>
using namespace std;

const double eps = 1e-6;

struct Tri {
	// foreground color, background color: RGB
	double fc[3], bc[3];
	// 3 vertex position: x, y, z
	double vxyz[3][3];
	// the normal vector of vertex: x, y, z
	double nxyz[3][3];
	double area() const {
		double v[3], v1[3], v2[3];
		for (int i = 0; i < 3; i++) {
			v1[i] = vxyz[1][i] - vxyz[0][i];
			v2[i] = vxyz[2][i] - vxyz[0][i];
		}
		v[0] = v1[1] * v2[2] - v2[1] * v1[2];
		v[1] = v1[2] * v2[0] - v2[2] * v1[0];
		v[2] = v1[0] * v2[1] - v2[0] * v1[1];
		return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])/2;
	}
};

struct Pt {
    double x, y, z;
    Pt(double x = 0.f, double y = 0.f, double z = 0.f):
    	x(x), y(y), z(z) {}	
    bool operator==(const Pt &a) const {
    	return fabs(x - a.x) < eps && fabs(y - a.y) < eps && fabs(z - a.z) < eps;
    }
	Pt operator-(const Pt &a) const {
		return Pt(x-a.x, y-a.y, z-a.z);
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
	Pt cross(const Pt &vb) const {
		Pt r;
		r.x = y * vb.z - vb.y * z;
		r.y = z * vb.x - vb.z * x;
		r.z = x * vb.y - vb.x * y;
		return r;
	}
	double length() const {
		return sqrt(x*x+y*y+z*z);
	}
	void normalize() {
		double l = length();
		x /= l, y /= l, z /= l;
	}
};

void readLine(string line, double f[]) {
	stringstream sin(line);
	for (int i = 0; i < 6; i++) {
		assert(sin >> f[i]);
	}
}

void writeColorfulJson(string ofileName, vector<Tri> &A) {
	FILE *fout = fopen(ofileName.c_str(), "w");
	if (fout == NULL) {
		fprintf(stderr, "Output file path failed");
		exit(1);
	}
	// output json format
	fprintf(fout, "{\n");

	fprintf(fout, "\t\"colorful\": true,\n");

	fprintf(fout, "\t\"vertexPositions\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			sz++;
			fprintf(fout, "%g,%g,%g", t.vxyz[j][0], t.vxyz[j][1], t.vxyz[j][2]);
		}
	}
	fprintf(fout, "],\n");

	fprintf(fout, "\t\"vertexNormals\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			sz++;
			fprintf(fout, "%g,%g,%g", t.nxyz[j][0], t.nxyz[j][1], t.nxyz[j][2]);
		}
	}
	fprintf(fout, "],\n");

	fprintf(fout, "\t\"vertexTextureCoords\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			sz++;
			fprintf(fout, "1,1,1");
		}
	}

	fprintf(fout, "],\n");

	fprintf(fout, "\t\"vertexFrontColors\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			sz++;
			fprintf(fout, "%g,%g,%g", t.fc[0]/255.0, t.fc[1]/255.0, t.fc[2]/255.0);
		}
	}
	fprintf(fout, "],\n");

	fprintf(fout, "\t\"vertexBackColors\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			sz++;
			fprintf(fout, "%g,%g,%g", t.bc[0]/255.0, t.bc[1]/255.0, t.bc[2]/255.0);
		}
	}
	fprintf(fout, "],\n");

	fprintf(fout, "\t\"indices\" : [");
	for (int i = 0, sz = 0; i < A.size(); i++) {
		Tri t = A[i];
		for (int j = 0; j < 3; j++) {
			if (sz)
				fprintf(fout, ",");
			fprintf(fout, "%d", sz);
			sz++;
		}
	}
	fprintf(fout, "]\n");
	fprintf(fout, "}\n");
	fprintf(stderr, "Model-Converter Success\n");
}
void readColorfulTriangle(string ifileName, string ofileName) {
	ifstream fin(ifileName);
	if (fin.fail())
		assert(false && "Input file not exited");
	// read triangle
	string objName;
	vector<Tri> A;
	Pt leftPt, rightPt;
	bool hasLeft = false, hasRight = false;
	while (getline(fin, objName)) {
		assert(objName == "Triangle " && "Make sure all object triangle based");
		string line;
		double f[9];
		Tri t;	

		assert(getline(fin, line));
		readLine(line, f);

		for (int i = 0; i < 3; i++)
			t.fc[i] = f[i], t.bc[i] = f[i+3];

		// read vertex information
		for (int i = 0; i < 3; i++) {
			assert(getline(fin, line));
			readLine(line, f);
			for (int j = 0; j < 3; j++) {
				t.vxyz[i][j] = f[j];
				t.nxyz[i][j] = f[j+3];
			}
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
		// duplicate back triangle
		{
			for (int i = 0; i < 3; i++)
				swap(t.bc[i], t.fc[i]);
			float gap = 1e-8;
			for (int i = 0; i < 3; i++) {
				t.vxyz[i][0] -= t.nxyz[i][0]*gap;
				t.vxyz[i][1] -= t.nxyz[i][1]*gap;
				t.vxyz[i][2] -= t.nxyz[i][2]*gap;
			}
			for (int i = 0; i < 3; i++) {
				t.nxyz[i][0] = -t.nxyz[i][0];
				t.nxyz[i][1] = -t.nxyz[i][1];
				t.nxyz[i][2] = -t.nxyz[i][2];
			}
		//	A.push_back(t);
		}
	}

	// normalize bounding box 1 x 1 x 1
	if (true)
	{
		const double maxSide = max(max(rightPt.x-leftPt.x, rightPt.y-leftPt.y), rightPt.z-leftPt.z);
		const double view_scale = 10.f;
		const double scale = view_scale / maxSide;
		for (int i = 0; i < A.size(); i++) {
			double left[3] = {leftPt.x, leftPt.y, leftPt.z};
			for (int j = 0; j < 3; j++)	 {
				for (int k = 0; k < 3; k++) {
					A[i].vxyz[j][k] -= left[k];
					A[i].vxyz[j][k] *= scale;
					A[i].vxyz[j][k] -= 0.5f * view_scale;
				}
			}
			if (A[i].area() < eps)
				fprintf(stderr, "[Warning] Triangle area is too small to show\n");
		}
	}
	fprintf(stderr, "#Triangle %d\n", A.size());
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


