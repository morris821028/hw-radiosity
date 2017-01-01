#include <stdio.h>
#include <stdlib.h>
#include "rad.h"
#include "raycast.h"
#include "shade.h"
#define RefRatio	(0.5)
#define FreeTriangle()  (trinum)--
// #define AllocTriangle() (trinum)++

extern int Debug;

/*
   int random() {
   return rand();
   }
 */
static int AllocTriangle(void) { 
	if (trinum == MaxTri) { 
		fprintf(stderr, "\n**** Max Triangle exceed : %i ***\n", trinum);
		exit(-1); 
	}
	return trinum++; 
}


static void SetNeighborToMe(int neighborID, int oldtriID, int newtriID)
{
	if (neighborID >= 0)
	{
		TrianglePtr neitri = &TriStore[neighborID];
		int e = 0;
		for (int v = 0; v < 3; v++)
			if (neitri->neighbor[v] == oldtriID)
				e = v;
		neitri->neighbor[e] = newtriID;
	}
}


/**************************************************
  return ff for the point.
 **************************************************/
static float GetPointInTriangle(TrianglePtr tri, Vector p, float ff[3])
{
	float rand1, rand2, rand3, totalR;

	rand1 = (float)(random() >> 12);
	rand2 = (float)(random() >> 12);
	rand3 = (float)(random() >> 12);
	totalR = rand1 + rand2 + rand3;
	/* Normalize */
	rand1 /= totalR;
	rand2 /= totalR;
	rand3 /= totalR;

	/**************************************************
	  Calculate point.
	 **************************************************/
	for (int v = 0; v < 3; v++)
	{
		p[v] = rand1 * tri->p[0][v] + rand2 * tri->p[1][v] + rand3 * tri->p[2][v];
	}
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
float CalFF(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, Vector p)
{
	float ff = 0;
	float ctheta1, ctheta2;
	Vector dir;

	VectorTo(p, srctri->c, dir);
	if (RayHitted(p, dir, logdest) == logsrc)
	{				  /* if hit logical source */
		ctheta1 = CosTheta(dir, srctri->n);
		ctheta2 = -CosTheta(dir, destri->n);
		ff = ctheta1 * ctheta2 * srctri->area / (norm2(dir) * PI + srctri->area);
	}				  /* if  ray hit. */
	if (ff < 0.0)
		return (0.0);
	return ff;
}



/******************************************************************
  Function Name:
Description:
Use adaptive partition of source triangle to calculate ff.

Remark:

Date:

 *******************************************************************/
float      AdaptCalFF(float ff, TrianglePtr srctri, int logsrc, TrianglePtr destri,
		int logdest, Vector p)
{
	float ff1, ff2;
	Triangle t1, t2;
	float maxlength, length;
	int edge(0);
	Vector l;

	maxlength = 0.0;
	for (int v = 0; v < 3; v++)
	{
		VectorTo(srctri->p[v], srctri->p[(v + 1) % 3], l);
		length = l[0] * l[0] + l[1] * l[1] + l[2] * l[2];
		if (maxlength < length)
		{
			maxlength = length;
			edge = v;
		}
	}
	/**********************************************************
	  Partition source triangle.
	 **********************************************************/
	PartitionSource(srctri, &t1, &t2, edge);
	ff1 = CalFF(&t1, logsrc, destri, logdest, p);
	ff2 = CalFF(&t2, logsrc, destri, logdest, p);
	if (ffabs(ff1 + ff2 - ff) > 0.00001)
	{
		return AdaptCalFF(ff1, &t1, logsrc, destri, logdest, p)
			+ AdaptCalFF(ff2, &t2, logsrc, destri, logdest, p);
	}
	else
	{
		return (ff2 + ff1);
	}
}


static void PartitionDestination(TrianglePtr s, TrianglePtr t1, TrianglePtr t2, int edge)
{

	Vector p;
	register int i0, i1, i2;

	i0 = edge;			  /* edge is the one being splited */
	i1 = (++edge) % 3;
	i2 = (++edge) % 3;

	AverageVector(s->p[i0], s->p[i1], p);
	CopyVector(p, t1->p[0]);
	CopyVector(p, t2->p[0]);

	CopyVector(s->p[i2], t1->p[1]);
	CopyVector(s->p[i1], t2->p[1]);

	CopyVector(s->p[i0], t1->p[2]);
	CopyVector(s->p[i2], t2->p[2]);

	/**********************************************************
	  Triangle center, area, normal.
	 **********************************************************/
	CalCenter(t1);		  /* center point */
	CalCenter(t2);
	t1->area = t2->area = (s->area / 2.0);	/* area */
	CopyVector(s->n, t1->n);	  /* copy normal from original triangle */
	CopyVector(s->n, t2->n);

	/**********************************************************
	  Process other member.  F dB aB
	 **********************************************************/
	CopyVector(s->Frgb, t1->Frgb);
	CopyVector(s->Frgb, t2->Frgb);
	/*
	 * CopyVector(s->Brgb, t1->Brgb); CopyVector(s->Brgb, t2->Brgb);
	 */

	CopyVector(s->deltaB, t1->deltaB);
	CopyVector(s->deltaB, t2->deltaB);

	AverageVector(s->accB[i0], s->accB[i1], p);
	CopyVector(p, t1->accB[0]);
	CopyVector(p, t2->accB[0]);

	CopyVector(s->accB[i2], t1->accB[1]);
	CopyVector(s->accB[i1], t2->accB[1]);

	CopyVector(s->accB[i0], t1->accB[2]);
	CopyVector(s->accB[i2], t2->accB[2]);
	/**********************************************************
	  parent.
	 **********************************************************/
	t1->parent = t2->parent = s->parent;
}


static inline void CalRadiosity(TrianglePtr srctri, TrianglePtr destri, float ff[3])
{
	for (int v = 0; v < 3; v++)
	{
		for (int c = 0; c < 3; c++)
		{			  /* for color = R G B */
			/* calculate the reflectiveness */
			float lo, deltaB;
			lo = destri->Frgb[c] / 255.0 * RefRatio;
			deltaB = lo * srctri->deltaB[c] * ff[v] / 3;
			destri->deltaB[c] += deltaB;
			destri->accB[v][c] += deltaB;
			destri->deltaaccB[v][c] = deltaB;
		}			  /* for RGB */
	}				  /* for each vertex */
}



void Shade(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, int realdest)
{
	int destedge(0), neighboredge, samplenum;
	int t1ID, t2ID, t3ID, t4ID, n1ID, n2ID, neighborID;
	float groudFF, ff[3], ff2[3], ffs, deltaff;
	float maxlength, length;
	TrianglePtr t1, t2, t3, t4, neighbortri;
	Vector l, temp;

	/**********************************************************
	  Calculate FF first.
	 **********************************************************/
	// #pragma omp parallel for
	for (int v = 0; v < 3; v++)
	{
		/*      ff[v] = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, destri->p[v]),
				srctri, logsrc, destri, logdest, destri->p[v]); */
		ff[v] = CalFF(srctri, logsrc, destri, logdest, destri->p[v]);
	}

	/**********************************************************
	  Triangle is too small to split.
	 **********************************************************/
	if (destri->area < AreaLimit)
	{
		CalRadiosity(srctri, destri, ff);
		return;
	}
	groudFF = (ff[0] + ff[1] + ff[2]) / 3.0;
	/* ffs = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, destri->c),
	   srctri, logsrc, destri, logdest, destri->c); */
	ffs = CalFF(srctri, logsrc, destri, logdest, destri->c);
	if ((deltaff = ffabs(groudFF - ffs)) < DeltaFFLimit)
	{
		samplenum = (int)(destri->area / SampleArea);
		for (int v = 0; v < samplenum; v++)
		{
			groudFF = GetPointInTriangle(destri, temp, ff);
			/*      ffs = AdaptCalFF(CalFF(srctri, logsrc, destri, logdest, temp),
					srctri, logsrc, destri, logdest, temp); */
			ffs = CalFF(srctri, logsrc, destri, logdest, temp);
			if ((deltaff = ffabs(groudFF - ffs)) > DeltaFFLimit)
				break;
		}
	}
#ifdef _DEBUG
	if (Debug > 5)
		printf("tri: %i groudFF %f DeltaFF = %f area = %f \n", realdest, groudFF,
				deltaff, destri->area);
#endif

	/**********************************************************
	  If destined triangle is smooth enough, then calculate
	  radioisity, or split it.
	 **********************************************************/
	if (deltaff < DeltaFFLimit)
	{
		CalRadiosity(srctri, destri, ff);
		return;
	}
	/**********************************************************
	  Allocate children triangle space for destri
	 **********************************************************/
	t1ID = AllocTriangle();
	t1 = &TriStore[t1ID];
	t2ID = AllocTriangle();
	t2 = &TriStore[t2ID];
#ifdef _DEBUG
	if (Debug > 5)
		printf("Partition dest tri %i  parent = %i.\n", realdest, logdest);
#endif

	/* find longest edge to split */
	maxlength = 0.0;
	for (int v = 0; v < 3; v++)
	{
		VectorTo(destri->p[v], destri->p[(v + 1) % 3], l);
		length = l[0] * l[0] + l[1] * l[1] + l[2] * l[2];
		if (maxlength < length)
		{
			maxlength = length;
			destedge = v;
		}
	}

	neighborID = destri->neighbor[destedge];
	PartitionDestination(destri, t1, t2, destedge);

	t1->neighbor[1] = n1ID = destri->neighbor[(destedge + 2) % 3];
	t2->neighbor[1] = n2ID = destri->neighbor[(destedge + 1) % 3];
	t1->neighbor[2] = t2->neighbor[0] = -1;

	/* destri is a initial triangle */
	if (destri->parent == realdest)
	{
		destri->parent = -1;	  /* set to logical triangle */
	}
	else
		/* destri is a child triangle */
	{
		*destri = TriStore[t2ID];
		t2 = destri;
		t2ID = realdest;
		FreeTriangle();		  /* release memory */

		/*
		 * t2 replace it's parent, reshade it for delay shading
		 * correct.
		 */
		/* ff2[0] = AdaptCalFF(CalFF(srctri, logsrc, t2, logdest, t2->p[0]), srctri,
		   logsrc, t2, logdest, t2->p[0]); */
		ff2[0] = CalFF(srctri, logsrc, t2, logdest, t2->p[0]);
		ff2[1] = ff[(destedge + 1) % 3];
		ff2[2] = ff[(destedge + 2) % 3];
		CalRadiosity(srctri, t2, ff2);
	}
	SetNeighborToMe(n1ID, realdest, t1ID);
	SetNeighborToMe(n2ID, realdest, t2ID);
	t1->neighbor[0] = t2ID;
	t2->neighbor[2] = t1ID;

	/**********************************************************
	  There is a neighbor adjacent to the splitted edbg
	 **********************************************************/
	if (neighborID >= 0)
	{
		neighbortri = &TriStore[neighborID];
		t3ID = AllocTriangle();
		t3 = &TriStore[t3ID];
		t4ID = AllocTriangle();
		t4 = &TriStore[t4ID];

		/* find which edge adjacent to destri triangle */
		for (int v = 0; v < 3; v++)
		{
			if (neighbortri->neighbor[v] == realdest)
				neighboredge = v;
		}

		if (neighborID < realdest)
		{
			/*
			 * neighbor triangle had been shaded, then "unshade
			 * it"
			 */
			SubtractVector(neighbortri->accB[0], neighbortri->deltaaccB[0]);
			SubtractVector(neighbortri->accB[1], neighbortri->deltaaccB[1]);
			SubtractVector(neighbortri->accB[2], neighbortri->deltaaccB[2]);
		}
		PartitionDestination(neighbortri, t3, t4, neighboredge);

		t3->neighbor[1] = n1ID = neighbortri->neighbor[(neighboredge + 2) % 3];
		t4->neighbor[1] = n2ID = neighbortri->neighbor[(neighboredge + 1) % 3];

		if (neighbortri->parent == neighborID)
		{
			neighbortri->parent = -1;	/* set to logical
										 * triangle */
		}
		else
		{
			*neighbortri = TriStore[t4ID];
			t4 = neighbortri;
			t4ID = neighborID;
			FreeTriangle();	  /* release */
		}

		SetNeighborToMe(n1ID, neighborID, t3ID);
		SetNeighborToMe(n2ID, neighborID, t4ID);
		t3->neighbor[0] = t4ID;
		t4->neighbor[2] = t3ID;

		if (destri->p[destedge] == neighbortri->p[neighboredge])
		{
			t1->neighbor[2] = t3ID;
			t3->neighbor[2] = t1ID;
			t2->neighbor[0] = t4ID;
			t4->neighbor[0] = t2ID;
		}
		else
		{
			t1->neighbor[2] = t4ID;
			t4->neighbor[0] = t1ID;
			t2->neighbor[0] = t3ID;
			t3->neighbor[2] = t2ID;
		}

		if (t4ID < realdest)
		{
			/*
			 * t4 replace it's parent, reshade it for delay
			 * shading correct.
			 */
			for (int v = 0; v < 3; v++)
				/* ff[v] = AdaptCalFF(CalFF(srctri, logsrc, t4, logdest, t4->p[v]), srctri,
				   logsrc, t4, logdest, t4->p[v]); */
				ff[v] = CalFF(srctri, logsrc, t4, logdest, t4->p[v]);
			CalRadiosity(srctri, t4, ff);
		}
	}				  /* end of if (neighborID >=0 */
}				  /* end of Shade() */
