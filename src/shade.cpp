#include <stdio.h>
#include <stdlib.h>
#include "rad.h"
#include "raycast.h"
#include "shade.h"
#include "config.h"
#define RefRatio	(0.5)

/* Helper */
#define FreeTriangle()  (trinum)--
// #define AllocTriangle() (trinum)++

extern int Debug;

const float eps = 1e-6;

static int AllocTriangle(void) { 
	static int firstFlag = 1;
	if (trinum == MaxTri) { 
		if (firstFlag) {
			fprintf(stderr, "\n**** Max Triangle exceed : %i ***\n", trinum);
			firstFlag = 0;
		}
		return -1;
		fprintf(stderr, "\n**** Max Triangle exceed : %i ***\n", trinum);
		exit(-1); 
	}
	return trinum++; 
}


static void SetNeighborToMe(int neighborID, int oldtriID, int newtriID)
{
	if (neighborID < 0)
		return ;
	TrianglePtr neitri = &TriStore[neighborID];
	for (int v = 0; v < 3; v++) {
		if (neitri->neighbor[v] == oldtriID) {
			neitri->neighbor[v] = newtriID;
		}
	}
}


/**************************************************
  return ff for the point.
 **************************************************/
static float GetPointInTriangle(TrianglePtr tri, Vector p, float ff[3])
{
	float rand1, rand2, rand3, totalR;

	rand1 = (float) rand();
	rand2 = (float) rand();
	rand3 = (float) rand();
	totalR = rand1 + rand2 + rand3;
	/* Normalize */
	rand1 /= totalR;
	rand2 /= totalR;
	rand3 /= totalR;

	/**************************************************
	  Calculate point.
	 **************************************************/
	for (int v = 0; v < 3; v++)
		p[v] = rand1 * tri->p[0][v] + rand2 * tri->p[1][v] + rand3 * tri->p[2][v];
	return (rand1 * ff[0] + rand2 * ff[1] + rand3 * ff[2]);
}


/******************************************************************
  Function Name:
Description:
Partition a Source triangle to two triangle.
 *******************************************************************/
static void PartitionSource(TrianglePtr s, TrianglePtr t1, TrianglePtr t2, int edge)
{
	Vector p;
	int i0, i1, i2;

	i0 = edge;			  /* edge is the one being splited */
	i1 = (++edge) % 3;
	i2 = (++edge) % 3;
	AverageVector(s->p[i0], s->p[i1], p);
	CopyVector(p, t1->p[0]);
	CopyVector(p, t2->p[0]);

	CopyVector(s->p[i2], t1->p[1]);
	CopyVector(s->p[i0], t1->p[2]);
	CalCenter(t1);
	CopyVector(s->n, t1->n);

	CopyVector(s->p[i1], t2->p[1]);
	CopyVector(s->p[i2], t2->p[2]);
	CalCenter(t2);
	CopyVector(s->n, t2->n);

	t1->area = t2->area = (s->area / 2.0);
}


/******************************************************************
  Function Name:
Description:
Calculate ff for a vertex to a triangle.
 *******************************************************************/
static float CalFF(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, Vector p)
{
	Vector dir;
	VectorTo(p, srctri->c, dir);
	float ctheta1 = CosTheta(dir, srctri->n);
	float ctheta2 = -CosTheta(dir, destri->n);
	float ff = ctheta1 * ctheta2;
	if (ff <= 0.0)
		return 0.0;
	ff *= srctri->area / (norm2(dir) * PI + srctri->area);
	// ff *= srctri->area / (norm2(dir) * PI);
	if (ff <= 0.0)
		return 0.0;
	if (RayHitted(p, dir, logdest) == logsrc)
		return ff;
	return 0.0;
}



/******************************************************************
  Function Name:
Description:
Use adaptive partition of source triangle to calculate ff.

Remark:

Date:

 *******************************************************************/
float AdaptCalFF(float ff, TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, Vector p)
{
	int edge = 0;
	{
		float maxlength = 0.0;
		for (int v = 0; v < 3; v++) {
			Vector l;
			VectorTo(srctri->p[v], srctri->p[(v + 1) % 3], l);
			float length = l[0] * l[0] + l[1] * l[1] + l[2] * l[2];
			if (maxlength < length) {
				maxlength = length, edge = v;
			}
		}
		if (maxlength < 1)
			return ff;
	}
	/**********************************************************
	  Partition source triangle.
	 **********************************************************/
	Triangle t1, t2;
	PartitionSource(srctri, &t1, &t2, edge);
	float ff1 = CalFF(&t1, logsrc, destri, logdest, p);
	float ff2 = CalFF(&t2, logsrc, destri, logdest, p);
	if (fabs(ff1 + ff2 - ff) <= 15.0 * eps)
		return ff1 + ff2 + (ff1 + ff2 - ff) / 15.0;
	return AdaptCalFF(ff1, &t1, logsrc, destri, logdest, p)
			+ AdaptCalFF(ff2, &t2, logsrc, destri, logdest, p);
}


static void PartitionDestination(TrianglePtr s, TrianglePtr t1, TrianglePtr t2, int edge)
{
	int i0, i1, i2;

	i0 = edge;			  /* edge is the one being splited */
	i1 = (++edge) % 3;
	i2 = (++edge) % 3;
	{
		Vector p;
		AverageVector(s->p[i0], s->p[i1], p);
		CopyVector(p, t1->p[0]);
		CopyVector(s->p[i2], t1->p[1]);
		CopyVector(s->p[i0], t1->p[2]);
		CopyVector(s->n, t1->n);
		CopyVector(s->Frgb, t1->Frgb);
		CopyVector(s->deltaB, t1->deltaB);
		CalCenter(t1);

		CopyVector(p, t2->p[0]);
		CopyVector(s->p[i1], t2->p[1]);
		CopyVector(s->p[i2], t2->p[2]);
		CopyVector(s->n, t2->n);
		CopyVector(s->Frgb, t2->Frgb);
		CopyVector(s->deltaB, t2->deltaB);
		CalCenter(t2);
		// CopyVector(s->Brgb, t1->Brgb); CopyVector(s->Brgb, t2->Brgb);
	}
	{
		Vector p;
		AverageVector(s->accB[i0], s->accB[i1], p);
		CopyVector(p, t1->accB[0]);
		CopyVector(s->accB[i2], t1->accB[1]);
		CopyVector(s->accB[i0], t1->accB[2]);

		CopyVector(p, t2->accB[0]);
		CopyVector(s->accB[i1], t2->accB[1]);
		CopyVector(s->accB[i2], t2->accB[2]);
	}
	t1->area = t2->area = (s->area / 2.0);
	t1->parent = t2->parent = s->parent;
}


static inline void CalRadiosity(TrianglePtr srctri, TrianglePtr destri, float ff[3])
{
	const float k = RefRatio / 255.0;
	float lo[3] = {destri->Frgb[0]*k*(srctri->deltaB[0]), destri->Frgb[1]*k*(srctri->deltaB[1]), destri->Frgb[2]*k*(srctri->deltaB[2])};
	for (int v = 0; v < 3; v++)	{ // for each vertex
		for (int c = 0; c < 3; c++) { // for each color: RGB
			/* calculate the reflectiveness */
			float deltaB = lo[c] * ff[v] / 3;
			destri->deltaB[c] += deltaB;
			destri->accB[v][c] += deltaB;
			destri->deltaaccB[v][c] = deltaB;
		}
	}
}

/**
 * @return -1: failed, 0: success, 1: success and replace its parent 
 */
inline int PartitionTriangleAndStore(TrianglePtr srcTri, int realSrc, int &destedge, 
		TrianglePtr &t1, TrianglePtr &t2, TrianglePtr &t3, TrianglePtr &t4, bool &reshadeNeighbor) {
	reshadeNeighbor = false;
	/*****************************************************
	  find longest edge to split -> create acute triangle
	 ******************************************************/
	int t1ID = AllocTriangle();
	int t2ID = AllocTriangle();
	if (t1ID < 0 || t2ID < 0)
		return -1;
	t1 = &TriStore[t1ID];
	t2 = &TriStore[t2ID];
	destedge = 0;
	{
		float maxlength = 0.0;
		for (int v = 0; v < 3; v++) {
			Vector l;
			VectorTo(srcTri->p[v], srcTri->p[(v+1) % 3], l);
			float length = norm2(l);
			if (maxlength < length) {
				maxlength = length, destedge = v;
			}
		}
		if (maxlength < 1)
			return -1;
	}
	int replaceFlag = 0;	
	int neighborID = srcTri->neighbor[destedge];
	int n1ID, n2ID;

	PartitionDestination(srcTri, t1, t2, destedge);

	t1->neighbor[1] = n1ID = srcTri->neighbor[(destedge+2) % 3];
	t2->neighbor[1] = n2ID = srcTri->neighbor[(destedge+1) % 3];
	t1->neighbor[2] = t2->neighbor[0] = -1;

	/*****************************************************
	  set the relationship of srcTri, t1 and t2 triangle
	 ******************************************************/
	if (srcTri->parent == realSrc)	{
		// this triangle is a initial triangle, and set to logical triangle
		srcTri->parent = -1;
	} else {
		// replace srcTri with t2
		*srcTri = TriStore[t2ID];
		t2 = srcTri;
		t2ID = realSrc;
		FreeTriangle();		  /* release memory */
		replaceFlag = 1;
	}
	SetNeighborToMe(n1ID, realSrc, t1ID);
	SetNeighborToMe(n2ID, realSrc, t2ID);
	t1->neighbor[0] = t2ID;
	t2->neighbor[2] = t1ID;
	if (neighborID < 0)
		return replaceFlag;

	int t3ID = AllocTriangle();
	int t4ID = AllocTriangle();
	if (t3ID < 0 || t4ID < 0)
		return replaceFlag;
	t3 = &TriStore[t3ID];
	t4 = &TriStore[t4ID];

	/*****************************************************
	  There is a neighbor adjacent to the splitted edge
	 ******************************************************/
	TrianglePtr neighborTri = &TriStore[neighborID];
	int neighboredge = 0;
	// find which edge adjacent to srcTri triangle
	for (int v = 0; v < 3; v++) {
		if (neighborTri->neighbor[v] == realSrc)
			neighboredge = v;
	}
	if (neighborID < realSrc) {
		// neighbor triangle had been shaded, then "unshadeit"
		SubtractVector(neighborTri->accB[0], neighborTri->deltaaccB[0]);
		SubtractVector(neighborTri->accB[1], neighborTri->deltaaccB[1]);
		SubtractVector(neighborTri->accB[2], neighborTri->deltaaccB[2]);
	}
	PartitionDestination(neighborTri, t3, t4, neighboredge);
	t3->neighbor[1] = n1ID = neighborTri->neighbor[(neighboredge + 2) % 3];
	t4->neighbor[1] = n2ID = neighborTri->neighbor[(neighboredge + 1) % 3];

	if (neighborTri->parent == neighborID) {
		// this triangle is a initial triangle, and set to logical triangle
		neighborTri->parent = -1;
	} else {
		*neighborTri = TriStore[t4ID];
		t4 = neighborTri;
		t4ID = neighborID;
		FreeTriangle();	  /* release */
	}

	SetNeighborToMe(n1ID, neighborID, t3ID);
	SetNeighborToMe(n2ID, neighborID, t4ID);
	t3->neighbor[0] = t4ID;
	t4->neighbor[2] = t3ID;

	if (srcTri->p[destedge] == neighborTri->p[neighboredge]) {
		t1->neighbor[2] = t3ID;
		t3->neighbor[2] = t1ID;
		t2->neighbor[0] = t4ID;
		t4->neighbor[0] = t2ID;
	} else {
		t1->neighbor[2] = t4ID;
		t4->neighbor[0] = t1ID;
		t2->neighbor[0] = t3ID;
		t3->neighbor[2] = t2ID;
	}
	// t4 replace it's parent, reshade it for delay shading correct
	if (t4ID < realSrc)
		reshadeNeighbor = true;
	return replaceFlag;
}

int Shade(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, int realdest)
{
	float ff[3], ffs, deltaff;

	/**********************************************************
	  Calculate FF first.
	 **********************************************************/
	// #pragma omp parallel for
	for (int v = 0; v < 3; v++)
	{
#ifdef ADAPT
		ff[v] = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, destri->p[v]),
				srctri, logsrc, destri, logdest, destri->p[v]);
#else
		ff[v] = CalFF(srctri, logsrc, destri, logdest, destri->p[v]);
#endif
	}

	/**********************************************************
	  Triangle is too small to split.
	 **********************************************************/
	if (destri->area < AreaLimit) {
		CalRadiosity(srctri, destri, ff);
		return 0;
	}
#ifdef DONT_PARTITION
	CalRadiosity(srctri, destri, ff);
	return 0;
#endif

	float groudFF = (ff[0] + ff[1] + ff[2]) / 3.0;
#ifdef ADAPT
	ffs = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, destri->c),
			srctri, logsrc, destri, logdest, destri->c);
#else
	ffs = CalFF(srctri, logsrc, destri, logdest, destri->c);
#endif
	if ((deltaff = ffabs(groudFF - ffs)) < DeltaFFLimit) {
		int samplenum = (int)(destri->area / SampleArea);
		for (int v = 0; v < samplenum; v++)
		{
			Vector temp;
			groudFF = GetPointInTriangle(destri, temp, ff);
#ifdef ADAPT
			ffs = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, temp),
					srctri, logsrc, destri, logdest, temp);
#else
			ffs = CalFF(srctri, logsrc, destri, logdest, temp);
#endif
			if ((deltaff = ffabs(groudFF - ffs)) > DeltaFFLimit)
				break;
		}
		/**********************************************************
		  If destined triangle is smooth enough, then calculate
		  radioisity, or split it.
		 **********************************************************/
		if (deltaff < DeltaFFLimit) {
			CalRadiosity(srctri, destri, ff);
			return 0;
		}
	}
#ifdef _DEBUG
	if (Debug > 5)
		printf("tri: %i groudFF %f DeltaFF = %f area = %f \n", realdest, groudFF,
				deltaff, destri->area);
#endif
	int replaceFlag = 0, destedge;
	bool reshadeNeighbor;
	TrianglePtr t1, t2, t3, t4;
	replaceFlag = PartitionTriangleAndStore(destri, realdest, destedge, t1, t2, t3, t4, reshadeNeighbor);
	if (replaceFlag < 0) {
		CalRadiosity(srctri, destri, ff);
		return 0;
	}
	if (replaceFlag == 1) {
#ifndef ROLLBACK
		float ff2[3];
#ifdef ADAPT
		ff2[0] = AdaptCalFF(CalFF(srctri, logsrc, t2, logdest, t2->p[0]), srctri,
				logsrc, t2, logdest, t2->p[0]);
#else
		ff2[0] = CalFF(srctri, logsrc, t2, logdest, t2->p[0]);
#endif
		ff2[1] = ff[(destedge + 1) % 3];
		ff2[2] = ff[(destedge + 2) % 3];
		CalRadiosity(srctri, t2, ff2);
#endif
	}
	if (reshadeNeighbor) {
		for (int v = 0; v < 3; v++) {
#ifdef ADAPT
			ff[v] = AdaptCalFF(CalFF(srctri, logsrc, t4, logdest, t4->p[v]), srctri,
					logsrc, t4, logdest, t4->p[v]); 
#else
			ff[v] = CalFF(srctri, logsrc, t4, logdest, t4->p[v]);
#endif
		}
		CalRadiosity(srctri, t4, ff);
	}
	return replaceFlag;
}

void PrePartitionTriangles()
{
    for (int it = 0; it < 10; it++) {
        int has = 0;
        for (int i = 0; i < trinum; i++) {
            TrianglePtr tp = &TriStore[i];
            if (tp->area < 1000)
                continue;
            has = 1;
            int destedge;
            bool reshadeNeighbor;
            TrianglePtr t1, t2, t3, t4;
            PartitionTriangleAndStore(tp, i, destedge, t1, t2, t3, t4, reshadeNeighbor);
        }
        if (!has)
            break;
    }
    fprintf(stderr, "MoreTriangle %d\n", trinum);
}

