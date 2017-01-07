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
#include "report.h"
#include "config.h"
#include <omp.h>
#include <algorithm>
#include <set>
using namespace std;
#define DEBUG3x
#define MAX_OPTION  9

int Debug = 0;
float AreaLimit = 5.0;
float SampleArea = -1.0;	       /* model dependent or user-defined */
float ConvergeLimit = 1200.0;
float DeltaFFLimit = 0.000005;
float LightScale = 1.0;
int TriangleLimit = 1000000;
int WriteIteration = 10;
int CompressResult = 0;
Triangle TriStore[MaxTri];
int TriStorePtr;

int trinum;
Vector G0, G1;

const char option[MAX_OPTION][3] = {"-d", "-a", "-s", "-c", "-f", "-l", "-z", "-t", "-i"};


/* 
 * ** Read in the triangles.  ** **     Also calculate the normal vector,
 * center point, **     the equations of three side with normal pointed
 * inward, **   and the area. ** **     The background RGB is the emitting
 * value.  **   The foreground RGB is used as the coeffient of
 * reflectiveness. **     (See InitRad() and DoRadiosity() for detail
 * information) ** ** Determine the GO,G1.
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
  Change loop check.
  Always write output.
  0 is final.
 **********************************************************/
void PrintOut(char *fname, int loop)
{
	int i;
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
	for (i = 0; i < trinum; i++)
	{
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
	{
		char cmd[128] = "cd ./demo && ./run.sh ../";
		strcat(cmd, fn);
		system(cmd);
	}
}


void init(FILE * fp)
{

	ReadTriangle(fp);

	printf("Triangle: %d\n", trinum);

	BuildTree();
}



/* 
 * * Initiate accB and deltaB to 0 ** For the light source, set accB and
 * delta equal to its emiting  **       value-- the background color. ** **
 * Note:: A patch is a light source iff any one background color  ** is
 * nonzero.
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
		if (minarea > tp->area)
			minarea = tp->area;
		if (maxarea < tp->area)
			maxarea = tp->area;

		/**********************************************************
		  light source.
		 **********************************************************/
		if (isLightSource(tp))
		{
			InitVector(tp->deltaB, 0.0, 0.0, 0.0);
			float scale = LightScale;
			for (int v = 0; v < 3; v++)
			{
				tp->accB[v][0] = tp->Brgb[0] * scale;
				tp->accB[v][1] = tp->Brgb[1] * scale;
				tp->accB[v][2] = tp->Brgb[2] * scale;

				tp->deltaB[0] += 100 * tp->accB[v][0] / 3;
				tp->deltaB[1] += 100 * tp->accB[v][1] / 3;
				tp->deltaB[2] += 100 * tp->accB[v][2] / 3;
			}
		}
		else
		{
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
	printf("       MaxPatchArea    %f\n", maxarea);
	printf("       MinPatchArea    %f\n", minarea);
	printf("       debuglevel      %i\n", Debug);
	printf("       AreaLimit       %f\n", AreaLimit);
	printf("       SampleArea      %f\n", SampleArea);
	printf("       ConvergeLimit   %f\n", ConvergeLimit);
	printf("       DeltaFFLimit    %f\n", DeltaFFLimit);
	printf("       WriteIteration: %d\n", WriteIteration);
	printf("       TriangleLimit:  %d\n", TriangleLimit);
	printf("       LightScale:     %f\n", LightScale);
	printf("Init Rad Over!!\n");
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
			// float dE = delta * tp->area;
			if (delta > maxDelta && delta > ConvergeLimit) {
				maxIdx = i;
				maxDelta = delta;
			}
		}
#ifdef _DEBUG
		if (Debug) {
			printf("------------------------\n");
			printf("MaxDeltaRad--maxDelta = %f\n", maxDelta);
//			printf("MaxDeltaRad--maxArea  = %f\n", maxArea);
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
	size_t n = min(((int) C.size()+4)/5, 1000);
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
void DoRadiosity(char *fname)
{
	InitRad();
#ifdef PRE_PARTITION 
	PrePartitionTriangles();
#endif
	{
		int loop;
		for (loop = 1; ; loop++)
		{
			//if (loop % WriteIteration == 0)
			fprintf(stderr, "Loop %d\n", loop);
			if (parallelRadiosity() == 0)
				break;
			//if (serialRadiosity() == 0)
			//	break;
			if (loop % WriteIteration == 0)
				PrintOut(fname, loop);
			if (Debug)
				printf("In this iteration, there are %i triangles.\n", trinum);
		}
		printf("#Iterations %d\n", loop);
	}
	PrintOut(fname, 0);
}



int ProcessOption(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s [-dascfl] ifile ofile\n", argv[0]);
		fprintf(stderr, "       -d debuglevel    : default %i\n", Debug);
		fprintf(stderr, "       -a AreaLimit     : default %f\n", AreaLimit);
		fprintf(stderr, "       -s SampleArea    : user-defined or model-dependent\n");
		fprintf(stderr, "       -c ConvergeLimit : default %f\n", ConvergeLimit);
		fprintf(stderr, "       -f DeltaFFLimit  : default %f\n", DeltaFFLimit);
		fprintf(stderr, "       -l WriteIteration: default %d\n", WriteIteration);
		fprintf(stderr, "       -t TriangleLimit : default %d\n", TriangleLimit);
		fprintf(stderr, "       -i LightScale    : default %f\n", LightScale);
		fprintf(stderr, "	-z gzip output file. Default = false.\n");

		exit(1);
	}
	for (int i = 1; i < argc - 2; i++) {
		for (int j = 0; j < MAX_OPTION; j++) {
			if (strcmp(argv[i], option[j]))
				continue;
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
				default:
					fprintf(stderr, "Invaild Option : %s", argv[i]);
					exit(1);
			}		  /* end of switch */
		}
	}
	return argc-2;
}



int main(int argc, char *argv[])
{
	const int P = 20;
	omp_set_num_threads(P);
	int i;
	float start[2], end[2];
	FILE *fin;

	i = ProcessOption(argc, argv);

	start[0] = real_time();
	start[1] = process_time();

	if ((fin = fopen(argv[i++], "r")) == NULL)
	{
		fprintf(stderr, "File open error::%s\n", argv[2]);
		exit(1);
	}
	init(fin);

	end[0] = real_time();
	end[1] = process_time();

	printf("Total Initial time = %f\n", end[0] - start[0]);
	printf("User Initial time(CPU) = %f\n", end[1] - start[1]);

	DoRadiosity(argv[i]);

	end[0] = real_time();
	end[1] = process_time();

	printf("Total Process time = %f\n", end[0] - start[0]);
	printf("User Process time(CPU) = %f\n", end[1] - start[1]);

	fclose(fin);
	return 0;
}
