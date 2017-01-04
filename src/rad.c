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
#define DEBUG3x
#define MAX_OPTION  7

int Debug = 0;
float AreaLimit = 5.0;
float SampleArea = -1.0;	       /* model dependent or user-defined */
float ConvergeLimit = 1200.0;
float DeltaFFLimit = 0.001;
int WriteIteration = 20;
int CompressResult = 0;
Triangle TriStore[MaxTri];
int TriStorePtr;

int trinum;
Vector G0, G1;

const char option[MAX_OPTION][3] = {"-d", "-a", "-s", "-c", "-f", "-l", "-z"};


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
		fprintf(fout, "Triangle \n");
		fprintf(fout, "%f %f %f ",
				tp->p[0][0], tp->p[0][1], tp->p[0][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[0][0], (int)tp->accB[0][1], (int)tp->accB[0][2]);
		fprintf(fout, "%f %f %f ", tp->p[1][0], tp->p[1][1], tp->p[1][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[1][0], (int)tp->accB[1][1], (int)tp->accB[1][2]);
		fprintf(fout, "%f %f %f ",
				tp->p[2][0], tp->p[2][1], tp->p[2][2]);
		fprintf(fout, "%i %i %i 0 0 0 \n", (int)tp->accB[2][0], (int)tp->accB[2][1], (int)tp->accB[2][2]);
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
	int i, j, v, n;
	float minarea = 100000000.0, maxarea = 0.0;
	TrianglePtr tp, tn;

	for (i = 0; i < trinum; i++)
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
		if ((tp->Brgb[0] == 255) || (tp->Brgb[1] == 255) ||
				(tp->Brgb[2] == 255))
		{
			InitVector(tp->deltaB, 0.0, 0.0, 0.0);
			for (v = 0; v < 3; v++)
			{
				tp->accB[v][0] = tp->Brgb[0];
				tp->accB[v][1] = tp->Brgb[1];
				tp->accB[v][2] = tp->Brgb[2];

				tp->deltaB[0] += 100 * tp->accB[v][0] / 3;
				tp->deltaB[1] += 100 * tp->accB[v][1] / 3;
				tp->deltaB[2] += 100 * tp->accB[v][2] / 3;
			}
		}
		else
		{
			for (v = 0; v < 3; v++)
				InitVector(tp->accB[v], 0.0, 0.0, 0.0);
			CopyVector(tp->accB[0], tp->deltaB);
		}

		/**********************************************************
		  Initialize neighbor triangle.
		 **********************************************************/
		for (j = 0; j < trinum; j++)
		{
			if (j == i)
				continue;
			tn = &TriStore[j];


			for (v = 0; v < 3; v++)
			{
				for (n = 0; n < 3; n++)
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
	printf("Init Rad Over!!\n");
}


int MaxDeltaRad(void)
{
	int maxIdx = -1;
	{
		float maxDelta = 0.f, maxArea = 0.f;
		for (int i = 0; i < trinum; i++) {
			TrianglePtr tp = &TriStore[i];
			if (tp->parent < 0)
				continue;
			/**********************************************************
			  logical triangle
			 **********************************************************/

			float delta = tp->deltaB[0] + tp->deltaB[1] + tp->deltaB[2];
			float dE = delta * tp->area;
			if (delta > maxDelta && delta > ConvergeLimit) {
				maxIdx = i;
				maxDelta = delta, maxArea = dE;
			}
		}
#ifdef _DEBUG
		if (Debug) {
			printf("------------------------\n");
			printf("MaxDeltaRad--maxDelta = %f\n", maxDelta);
			printf("MaxDeltaRad--maxArea  = %f\n", maxArea);
		}
#endif
	}
	return maxIdx;
}


void DoRadiosity(char *fname)
{
	int physrc, logsrc, phydest, logdest;
	int loop;
	TrianglePtr destri, srctri;

	InitRad();
	for (loop = 1;; loop++)
	{
		/* find the patch with the largest deltaB */
		physrc = MaxDeltaRad();

		if (Debug)
		{
			printf("Doing iteraton %i for patch %d\n", loop, physrc);
		}
		if (physrc < 0)
			/* solution is good enough */
			break;

		/* calculate the radiosity for other patch */
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
		}			  /* for each tri */

		/* reset the deltaB of patch i */
		InitVector(srctri->deltaB, 0.0, 0.0, 0.0);

		if ((loop % WriteIteration) == 0)
			PrintOut(fname, loop);

		if (Debug)
			printf("In this iteration, there are %i triangles.\n", trinum);
	}
	printf("total iterations are %d\n", loop);
	PrintOut(fname, 0);
}



int ProcessOption(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s [-dascfl] ifile ofile\n", argv[0]);
		fprintf(stderr, "       -d debuglevel    : default %i\n", Debug);
		fprintf(stderr, "       -a AreaLimit     : default %f\n", AreaLimit);
		fprintf(stderr, "       -s SampleArea    : user-defined or model-dependent\n");
		fprintf(stderr, "       -c ConvergeLimit : default %f\n", ConvergeLimit);
		fprintf(stderr, "       -f DeltaFFLimit  : default %f\n", DeltaFFLimit);
		fprintf(stderr, "       -l WriteIteration: default %d\n", WriteIteration);
		fprintf(stderr, "	-z gzip output file. Default = false.\n");

		exit(1);
	}
	int i;
	for (i = 1; i < argc - 2; i++) {
		for (int j = 0; j < MAX_OPTION; j++) {
			if (*argv[i] == '-')
			{
				if (!strcmp(argv[i], option[j]))
				{
					switch (j)
					{
						case 0:
							if ((sscanf(argv[++i], "%d", &Debug)) != 1)
							{
								fprintf(stderr, "Debug Read Error !\n");
								exit(1);
							}
							break;
						case 1:
							if ((sscanf(argv[++i], "%f", &AreaLimit)) != 1)
							{
								fprintf(stderr, "AreaLimit Read Error !\n");
								exit(1);
							}
							break;
						case 2:
							if ((sscanf(argv[++i], "%f", &SampleArea)) != 1)
							{
								fprintf(stderr, "SampleArea Read Error !\n");
								exit(1);
							}
							break;
						case 3:
							if ((sscanf(argv[++i], "%f", &ConvergeLimit)) != 1)
							{
								fprintf(stderr, "ConvergeLimit Read Error !\n");
								exit(1);
							}
							break;
						case 4:
							if ((sscanf(argv[++i], "%f", &DeltaFFLimit)) != 1)
							{
								fprintf(stderr, "DeltaFFLimit Read Error !\n");
								exit(1);
							}
							break;
						case 5:
							if ((sscanf(argv[++i], "%d", &WriteIteration)) != 1)
							{
								fprintf(stderr, "Debug WriteIteration Error !\n");
								exit(1);
							}
							break;
						case 6:
							CompressResult = 1;
							break;
						default:
							fprintf(stderr, "Invaild Option : %s", argv[i]);
							exit(1);
					}		  /* end of switch */
				}		  /* end of if (!strcmp... */
			}			  /* end of if (argv[i]... */
		}
	}
	return i;
}



int main(int argc, char *argv[])
{
	const int P = 5;
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
