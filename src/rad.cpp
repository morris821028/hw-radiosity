/**************************************************************************
 * Filename:  rad.c
 *  Thu May 25 19:56:05 PDT 1995
 *
 *    				by Huang Tz-Shian
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "rad.h"
#include "raycast.h"
#include "shade.h"
#include "vector.h"
#include "config.h"
#include <omp.h>
#include <algorithm>
#include <set>
using namespace std;

int Debug = 0;
float AreaLimit = 5.0;
float SampleArea = -1.0;	       /* model dependent or user-defined */
float ConvergeLimit = 1200.0;
float DeltaFFLimit = 0.000005;
int TriangleLimit = 1000000;

namespace {
	float LightScale = 1.0;
	int WriteIteration = 10;
	int CompressResult = 0;
	bool InteractiveOutput = false;
	int ParallelJobs = 2;
}

Triangle TriStore[MaxTri];
int TriStorePtr;

int trinum;
Vector G0, G1;

/** 
 * Read in the triangles. Also calculate the normal vector, center point, 
 * the equations of three side with normal pointed
 * inward, and the area. The background RGB is the emitting
 * value. The foreground RGB is used as the coeffient of
 * reflectiveness. (See InitRad() and DoRadiosity() for detail
 * information) Determine the GO,G1.
 */

void ReadTriangle(FILE * fp)
{
	float m;
	Vector v0, v1, limit0, limit1;
	char buf[80];
	TrianglePtr tp;

	/* double          garbage[3]; */

	InitVector(limit0, 1000000.0, 1000000.0, 1000000.0);
	InitVector(limit1, -1000000.0, -1000000.0, -1000000.0);
	trinum = 0;
	while (1) {
		/* skip the "Triangle" string */
		if (fscanf(fp, "%s", buf) != 1 || feof(fp))
			break;
		tp = &TriStore[trinum];

		/* read rgb vectors and three vertices */
		ReadIVector(fp, tp->Frgb);
		ReadIVector(fp, tp->Brgb);
		/* garbage is for normal vector. */
		ReadVector(fp, tp->p[0]);
		/* ReadVector(fp, garbage); */
		ReadVector(fp, tp->p[1]);
		/* ReadVector(fp, garbage); */
		ReadVector(fp, tp->p[2]);
		/* ReadVector(fp, garbage); */

		CalCenter(tp);
		CalArea(tp, v0, v1);
		normalize(tp->n);

		/* calculate the plane equations of three sides */
		for (int i = 0; i < 3; i++)
		{
			VectorTo(tp->p[i], tp->p[(i + 1) % 3], v0);
			CrossProd(tp->n, v0, v1);
			tp->se[i][0] = v1[0];
			tp->se[i][1] = v1[1];
			tp->se[i][2] = v1[2];
			tp->se[i][3] = -InnerProd(tp->p[i], v1);
		}

		/* find the boundary */
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				if (limit0[j] > tp->p[i][j])
					limit0[j] = tp->p[i][j];
				if (limit1[j] < tp->p[i][j])
					limit1[j] = tp->p[i][j];
			}

		/**********************************************************
		  Initialize parent variable to itself.
		 **********************************************************/
		tp->parent = trinum++;
		InitVector(tp->neighbor, -1, -1, -1);
	}

	/* determine the encapsulated GRID corner--G0, G1 */
	m = 0;
	for (int i = 0; i < 3; i++)
		if ((limit1[i] - limit0[i]) > m)
			m = (limit1[i] - limit0[i]);
	G0[0] = limit0[0] - PI;
	G0[1] = limit0[1] - PI;
	G0[2] = limit0[2] - PI;
	G1[0] = G0[0] + m + 2 * PI;
	G1[1] = G0[1] + m + 2 * PI;
	G1[2] = G0[2] + m + 2 * PI;
}


/**********************************************************
  Change loop check. Always write output. 0 is final.
 **********************************************************/
void PrintOut(const char *fname, int loop)
{
	TrianglePtr tp;
	FILE *fout;
	char fn[80], tail[10], cmd[80];

	if (loop == 0)
		sprintf(tail, ".fin");
	else
		sprintf(tail, ".%1d", loop);

	strcpy(fn, fname);
	strcat(fn, tail);

	if ((fout = fopen(fn, "w")) == NULL)
	{
		fprintf(stderr, "Error: output file %s open failed\n", fn);
		if (loop == 0)
			fout = stdout;
		else
			return;
	}
	for (int i = 0; i < trinum; i++) {
		tp = &TriStore[i];
		/**********************************************************
		  skip logical triangle.
		 **********************************************************/
		if (tp->parent < 0)
			continue;

		/**********************************************************
		  Triangle key word.
		 **********************************************************/
#ifdef OLD_FORMAT
		fprintf(fout, "Triangle \n");
		fprintf(fout, "%f %f %f ",
				tp->p[0][0], tp->p[0][1], tp->p[0][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[0][0], (int)tp->accB[0][1], (int)tp->accB[0][2]);
		fprintf(fout, "%f %f %f ", tp->p[1][0], tp->p[1][1], tp->p[1][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[1][0], (int)tp->accB[1][1], (int)tp->accB[1][2]);
		fprintf(fout, "%f %f %f ",
				tp->p[2][0], tp->p[2][1], tp->p[2][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[2][0], (int)tp->accB[2][1], (int)tp->accB[2][2]);
#else

		fprintf(fout, "Triangle \n");
		float frontColor[3] = {};
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++)
				frontColor[j] += tp->accB[k][j];
			frontColor[j] /= 3;
			frontColor[j] = max(min(frontColor[j], 255.0f), 0.0f);
			if (isnan(frontColor[j]))
				frontColor[j] = 0;
		}
		fprintf(fout, "%.0f %.0f %.0f %.0f %.0f %0.f\n", frontColor[0], frontColor[1], frontColor[2], 0.0, 0.0, 0.0);
		for (int j = 0; j < 3; j++) {
			fprintf(fout, "%f %f %f %f %f %f\n", tp->p[j][0], tp->p[j][1], tp->p[j][2], tp->n[0], tp->n[1], tp->n[2]);
		}
#endif
	}

	fclose(fout);
	if (CompressResult)
	{
		strcpy(cmd, "gzip ");
		strcat(cmd, fn);
		printf("Compress output file %s .\n", fn);
		if (system(cmd))
			puts("OK");
		else
			puts("Failed");
	}
	if (InteractiveOutput)
	{
		char cmd[128] = "cd ./WebGL_demo && ./run.sh ../";
		strcat(cmd, fn);
		strcat(cmd, " 2>/dev/null");
		int ret = system(cmd);
		fprintf(stderr, "WebGL: %s, return %d\n", cmd, ret);
	}
}


void init(FILE * fp)
{

	ReadTriangle(fp);

	fprintf(stderr, "[" KGRN "INFO" KWHT "] Read #Triangle " KMAG "%d" KWHT " from input file\n", trinum);

	BuildTree();
}



/** 
 * Initiate accB and deltaB to 0. For the light source, set accB and
 * delta equal to its emiting value-- the background color.
 * Note: A patch is a light source iff any one background color is nonzero.
 */
void InitRad(void)
{
	float minarea = 100000000.0, maxarea = 0.0;
	TrianglePtr tp, tn;

	for (int i = 0; i < trinum; i++)
	{
		tp = &TriStore[i];
		/**************************************************
		  SampleArea = smallest area int this environment.
		 **************************************************/
		minarea = min(minarea, tp->area);
		maxarea = max(maxarea, tp->area);

		/**********************************************************
		  light source.
		 **********************************************************/
		if (isLightSource(tp)) {
			InitVector(tp->deltaB, 0.0, 0.0, 0.0);
			float scale = LightScale;
			for (int v = 0; v < 3; v++) {
				tp->accB[v][0] = tp->Brgb[0] * scale;
				tp->accB[v][1] = tp->Brgb[1] * scale;
				tp->accB[v][2] = tp->Brgb[2] * scale;

				tp->deltaB[0] += 100 * tp->accB[v][0] / 3;
				tp->deltaB[1] += 100 * tp->accB[v][1] / 3;
				tp->deltaB[2] += 100 * tp->accB[v][2] / 3;
			}
		}
		else {
			for (int v = 0; v < 3; v++)
				InitVector(tp->accB[v], 0.0, 0.0, 0.0);
			CopyVector(tp->accB[0], tp->deltaB);
		}

		/**********************************************************
		  Initialize neighbor triangle.
		 **********************************************************/
		for (int j = 0; j < trinum; j++)
		{
			if (j == i)
				continue;
			tn = &TriStore[j];


			for (int v = 0; v < 3; v++)
			{
				for (int n = 0; n < 3; n++)
				{
					if ((tp->p[v][0] == tn->p[n][0]) &&
							(tp->p[v][1] == tn->p[n][1]) &&
							(tp->p[v][2] == tn->p[n][2]))
					{
						if ((tp->p[(v + 1) % 3][0] == tn->p[(n + 1) % 3][0]) &&
								(tp->p[(v + 1) % 3][1] == tn->p[(n + 1) % 3][1]) &&
								(tp->p[(v + 1) % 3][2] == tn->p[(n + 1) % 3][2]))
						{
							/*
							 * neighbor is a
							 * "continuous" patch
							 */
							if (CosTheta(tp->n, tn->n) > 0.85)
							{
								/*
								 * same
								 * direction
								 */
								tp->neighbor[v] = j;
								tn->neighbor[n] = i;
							}
						}
						/* oppsite direction */
						else if ((tp->p[(v + 1) % 3][0] == tn->p[(n + 2) % 3][0]) &&
								(tp->p[(v + 1) % 3][1] == tn->p[(n + 2) % 3][1]) &&
								(tp->p[(v + 1) % 3][2] == tn->p[(n + 2) % 3][2]))
						{
							/*
							 * neighbor is a
							 * "continuous" patch
							 */
							if (CosTheta(tp->n, tn->n) > 0.85)
							{
								tp->neighbor[v] = j;
								tn->neighbor[(n + 2) % 3] = i;
							}
						}	  /* end of else if */
					}
				}		  /* end of for (n.. ) */
			}		  /* end of for (v.. ) */
		}			  /* end of for (j.. ) */

	}				  /* end of for (i.. ) */

	if (SampleArea < 0.0)
		SampleArea = maxarea / 100.0;
	printf("[" KGRN "INFO" KWHT "] Max Patch Area = " KMAG "%f\n" KWHT, maxarea);
	printf("[" KGRN "INFO" KWHT "] Min Patch Area = " KMAG "%f\n" KWHT, minarea);
	printf("[" KGRN "INFO" KWHT "] Config Table\n");
	printf("       -debug          %i\n", Debug);
	printf("       -adapt_area     %f\n", AreaLimit);
	printf("       -sample_area    %f\n", SampleArea);
	printf("       -converge       %f\n", ConvergeLimit);
	printf("       -delta_ff       %f\n", DeltaFFLimit);
	printf("       -write_cycle    %d\n", WriteIteration);
	printf("       -triangle       %d\n", TriangleLimit);
	printf("       -light          %f\n", LightScale);
	printf("       -interactive    %d\n", InteractiveOutput);
#ifdef PARALLEL
	printf("       -jobs           %d\n", ParallelJobs);
#endif
	printf("[" KGRN "INFO" KWHT "] Complete initialization\n");
}


int MaxDeltaRad(void)
{
	int maxIdx = -1;
	{
		float maxDelta = 0.f;
		for (int i = 0; i < trinum; i++) {
			TrianglePtr tp = &TriStore[i];
			if (tp->parent < 0)
				continue;
			/**********************************************************
			  logical triangle
			 **********************************************************/

			float delta = tp->deltaB[0] + tp->deltaB[1] + tp->deltaB[2];
			if (delta > maxDelta && delta > ConvergeLimit) {
				maxIdx = i;
				maxDelta = delta;
			}
		}
#ifdef _DEBUG
		if (Debug) {
			printf("------------------------\n");
			printf("MaxDeltaRad--maxDelta = %f\n", maxDelta);
		}
#endif
	}
	return maxIdx;
}
vector<int> listCanditate()
{
	vector< pair<float, int> > C;
	for (int i = 0; i < trinum; i++) {
		TrianglePtr tp = &TriStore[i];
		if (tp->parent < 0)
			continue;
		float delta = tp->deltaB[0] + tp->deltaB[1] + tp->deltaB[2];
		if (delta > ConvergeLimit)
			C.push_back(make_pair(delta, i));
	}
	sort(C.begin(), C.end());
	vector<int> ret;
	size_t n = min(((int) C.size()+4)/5, 10);
	for (size_t i = 0, j = C.size()-1; i < n; i++, j--)
		ret.push_back(C[j].second);
	return ret;
}

static int parallelRadiosity() {
	vector<int> C(listCanditate());
	if (C.size() == 0)
		return 0;
	int prev = 0;
	set<int> R;
	for (auto e : C)
		R.insert(e);
	do {
		int n = trinum;
#pragma omp parallel for
		for (int i = prev; i < n; i++) {
			TrianglePtr desTri, srcTri;
			desTri = &TriStore[i];
			if (desTri->parent < 0)
				continue;
			if (R.count(i))
				continue;
			if (desTri->accB[0][0] < 255.0 || desTri->accB[0][1] < 255.0
					|| desTri->accB[0][2] < 255.0 || desTri->accB[1][0] < 255.0
					|| desTri->accB[1][1] < 255.0 || desTri->accB[1][2] < 255.0
					|| desTri->accB[2][0] < 255.0 || desTri->accB[2][1] < 255.0
					|| desTri->accB[2][2] < 255.0) {
				int phydes = i;
				int logdes = desTri->parent;
				for (size_t j = 0; j < C.size(); j++) {
					srcTri = &TriStore[C[j]];
					int physrc = C[j];
					int logsrc = srcTri->parent;
					if (physrc == phydes)
						continue;
					Shade(srcTri, logsrc, desTri, logdes, phydes);
				}
			}
		}
		prev = n;
		if (n == trinum)
			break;
	} while (true);
	for (size_t j = 0; j < C.size(); j++) {
		TrianglePtr srcTri = &TriStore[C[j]];
		InitVector(srcTri->deltaB, 0.0, 0.0, 0.0);
	}
	return 1;
}
static int serialRadiosity() {
	int physrc, logsrc, phydest, logdest;
	TrianglePtr destri, srctri;
	// find the patch with the largest deltaB
	physrc = MaxDeltaRad();

	if (physrc < 0)
		return 0;
	// calculate the radiosity for other patch
	srctri = &TriStore[physrc];
	logsrc = srctri->parent;

	for (phydest = 0; phydest < trinum; phydest++)
	{
		if (physrc == phydest)
			continue;
		destri = &TriStore[phydest];
		if (destri->parent < 0)
			continue;
		if (destri->accB[0][0] < 255.0 || destri->accB[0][1] < 255.0
				|| destri->accB[0][2] < 255.0 || destri->accB[1][0] < 255.0
				|| destri->accB[1][1] < 255.0 || destri->accB[1][2] < 255.0
				|| destri->accB[2][0] < 255.0 || destri->accB[2][1] < 255.0
				|| destri->accB[2][2] < 255.0)
		{
			logdest = destri->parent;
#ifdef ROLLBACK
			if (Shade(srctri, logsrc, destri, logdest, phydest))
				phydest--;
#else
			Shade(srctri, logsrc, destri, logdest, phydest);
#endif
		}
	}

	// reset the deltaB of patch i
	InitVector(srctri->deltaB, 0.0, 0.0, 0.0);
	return 1;
}
void DoRadiosity(const char fileName[])
{
	InitRad();
#ifdef PRE_PARTITION 
	PrePartitionTriangles();
#endif
	{
		int loop;
		for (loop = 1; ; loop++)
		{
			if (loop % WriteIteration == 0)
				fprintf(stderr, "[" KGRN "INFO" KWHT "] Loop %d BEGIN\n", loop);
#ifdef PARALLEL
			if (parallelRadiosity() == 0)
				break;
#else
			if (serialRadiosity() == 0)
				break;
#endif
			if (loop % WriteIteration == 0) {
				PrintOut(fileName, loop);
				fprintf(stderr, "[" KGRN "INFO" KWHT "] END Loop. There are %i triangles.\n", trinum);
			}
		}
		fprintf(stderr, "[" KGRN "INFO" KWHT "] Complete in #Iterations %d\n", loop);
	}
	PrintOut(fileName, 0);
}



char* ProcessOption(int argc, char *argv[], FILE* &fin)
{
	const char option[][16] = {"-debug", "-adapt_area", "-sample_area", "-converge", "-delta_ff", 
				"-write_cycle", "-zip", "-triangle", "-light", "-o", "-interactive", "-jobs"};
	const int MAX_OPTION = sizeof(option) / sizeof(option[0]);
	if (argc < 3) {
		fprintf(stderr, "Usage: %s [options] input_file -o output_file\n", argv[0]);
		fprintf(stderr, "OPTIONS\n");
		fprintf(stderr, "   Debug Options\n");
		fprintf(stderr, "       -debug <integer>        Output intermediate result according debug level.\n");
		fprintf(stderr, "                               " KMAG "Default -debug %d\n\n" KWHT, Debug);
		fprintf(stderr, "   Radiosity Options\n");
		fprintf(stderr, "       -adapt_area <float>     The threahold of adaptive splitting algorithm.\n");
		fprintf(stderr, "                               " KMAG "Default -area %f\n" KWHT, AreaLimit);
		fprintf(stderr, "       -sample_area <float>    When the difference of form factor for each vertex is\n");
		fprintf(stderr, "                               greater than delta form factor, it should try to split.\n");
		fprintf(stderr, "                               " KMAG "Default by model-dependent\n" KWHT);
		fprintf(stderr, "       -converge <float>       The radiosity B is the energy per unit area unit B is too small.\n");
		fprintf(stderr, "                               " KMAG "Default -converge %f\n" KWHT, ConvergeLimit);
		fprintf(stderr, "       -delta_ff <float>       The difference of delta form factor which is smaller than delta_ff\n");
		fprintf(stderr, "                               will consider as the same.\n");
		fprintf(stderr, "                               " KMAG "Default -delta_ff %f\n" KWHT, DeltaFFLimit);
		fprintf(stderr, "       -write_cycle <integer>  Write the status of radiosity for each write_cycle iterations.\n");
		fprintf(stderr, "                               " KMAG "Default -write_cycle %d\n" KWHT, WriteIteration);
		fprintf(stderr, "       -triangle <integer>     The maximum the number of triangles in the model. If you show image\n");
		fprintf(stderr, "                               on WebGL, set -triangle 30000 is the best resolution\n");
		fprintf(stderr, "                               " KMAG "Defulat -triangle %d\n" KWHT, TriangleLimit);
		fprintf(stderr, "       -light <float>          Adjust the scale of bright light for testing.\n");
		fprintf(stderr, "                               " KMAG "Defulat -light %f\n" KWHT, LightScale);
		fprintf(stderr, "       -jobs <integer>         Multithreading OpenMP omp_set_num_threads\n");
		fprintf(stderr, "                               " KMAG "Defulat -jobs %d\n" KWHT, ParallelJobs);

		fprintf(stderr, "   Output Options\n");
		fprintf(stderr, "       -zip                    Compress output file by zip.\n");
		fprintf(stderr, "                               " KMAG "Defulat false\n" KWHT);
		fprintf(stderr, "       -o </<path>/file>       Assign the prefix filename the output file\n");
		fprintf(stderr, "                               " KMAG "Defulat ./test\n" KWHT);
		fprintf(stderr, "       -interactive            Transfer the result into WebGL each write_cycle\n");
		fprintf(stderr, "                               " KMAG "Defulat false\n" KWHT);

		exit(1);
	}
	static char default_path[16] = "./test";
	char *ifileName = NULL;
	char *ofileName = default_path;
	for (int i = 1; i < argc; i++) {
		int has = 0;
		for (int j = 0; j < MAX_OPTION; j++) {
			if (strcmp(argv[i], option[j]))
				continue;
			has = 1;
			switch (j) {
				case 0:
					assert(sscanf(argv[++i], "%d", &Debug) == 1 && "Debug Error");
					break;
				case 1:
					assert(sscanf(argv[++i], "%f", &AreaLimit) == 1 && "AreaLimit Error");
					break;
				case 2:
					assert(sscanf(argv[++i], "%f", &SampleArea) == 1 && "SampleArea Error");
					break;
				case 3:
					assert(sscanf(argv[++i], "%f", &ConvergeLimit) == 1 && "ConvergeLimit Error");
					break;
				case 4:
					assert(sscanf(argv[++i], "%f", &DeltaFFLimit) == 1 && "DeltaFFLimit Error");
					break;
				case 5:
					assert(sscanf(argv[++i], "%d", &WriteIteration) == 1 && "Debug WriteIteration Error");
					break;
				case 6:
					CompressResult = 1;
					break;
				case 7:
					assert(sscanf(argv[++i], "%d", &TriangleLimit) == 1 && "TriangleLimit Error");
					break;
				case 8:
					assert(sscanf(argv[++i], "%f", &LightScale) == 1 && "LightScale Error");
					break;
				case 9:
					ofileName = argv[++i];
					break;
				case 10:
					InteractiveOutput = true;
					break;
				case 11:
					assert(sscanf(argv[++i], "%d", &ParallelJobs) == 1 && "Parallel Jobs Error");
					omp_set_num_threads(ParallelJobs);
					break;
				default:
					fprintf(stderr, "Invaild Option : %s", argv[i]);
					exit(1);
			}
		}
		if (!has)
			ifileName = argv[i];
	}
	fprintf(stderr, "[" KGRN "INFO" KWHT "] Input Model " KMAG "%s" KWHT " and Output Prefix " KMAG "%s\n" KWHT, ifileName, ofileName);
	fin = fopen(ifileName, "r");
	return ofileName;
}



int main(int argc, char *argv[])
{
	omp_set_num_threads(ParallelJobs);
	FILE *fin;
	const char *foutName = ProcessOption(argc, argv, fin);
	
	if (fin == NULL) {
		fprintf(stderr, "File open error::%s\n", argv[2]);
		exit(1);
	}

	{
		double stTime = omp_get_wtime();
		init(fin);
		double edTime = omp_get_wtime();
		fclose(fin);
		fprintf(stderr, "[" KGRN "INFO" KWHT "] Init took " KMAG "%f" KWHT " sec. time.\n", edTime - stTime);
	}

	{
		double stTime = omp_get_wtime();
		DoRadiosity(foutName);
		double edTime = omp_get_wtime();
		fprintf(stderr, "[" KGRN "INFO" KWHT "] Radiosity Method took " KMAG "%f" KWHT " sec. time.\n", edTime - stTime);
	}
	return 0;
}
