#include "rad.h"
#include "raycast.h"
#include <stdio.h>
#include <stdlib.h>

int             TriListStore[MaxTriList];
int             TriListStorePtr;

TreeNode        TreeNodeStore[MaxTreeNode];
int             TreeNodeStorePtr;

float           MinGridLen;
int             GridNum;


/*
 * * two-dimension clip (at dimension x & y ) ** clip line (p0--p1) with grid
 * defined by corner (go,g1) ** We only detect if the intersection of line
 * with the grid  **  (interior included) is empty.
 */
int 
Clip(Vector p0, Vector p1, Vector g0, Vector g1, int x, int y)
{
	float           a, b, s, t;

	if ((p0[x] < g0[x]) && (p1[x] < g0[x]) ||
	    (p0[x] > g1[x]) && (p1[x] > g1[x]) ||
	    (p0[y] < g0[y]) && (p1[y] < g0[y]) ||
	    (p0[y] > g1[y]) && (p1[y] > g1[y]))
		return FALSE;

	a = p1[y] - p0[y];
	b = p1[x] - p0[x];

	t = a * (g0[x] - p0[x]) - b * (g0[y] - p0[y]);
	if (t == 0)
		return TRUE;

	s = a * (g0[x] - p0[x]) - b * (g1[y] - p0[y]);
	if (s * t <= 0)
		return TRUE;

	s = a * (g1[x] - p0[x]) - b * (g0[y] - p0[y]);
	if (s * t <= 0)
		return TRUE;

	s = a * (g1[x] - p0[x]) - b * (g1[y] - p0[y]);
	if (s * t <= 0)
		return TRUE;

	return FALSE;
}


/*
 * * sort three vector with respect to dimension d
 */
void 
sort(VectorPtr * maxv, VectorPtr * midv, VectorPtr * minv, int d)
{
	VectorPtr       t;

	if ((*maxv)[d] < (*midv)[d]) {
		t = *maxv;
		*maxv = *midv;
		*midv = t;
	}
	if ((*maxv)[d] < (*minv)[d]) {
		t = *maxv;
		*maxv = *minv;
		*minv = t;
	}
	if ((*midv)[d] < (*minv)[d]) {
		t = *midv;
		*midv = *minv;
		*minv = t;
	}
}


/*
 * * Interpolation vector p from maxv, minv such that  **  the value of p[d]
 * 
 * is g.
 */
void 
InterpVector(Vector maxv, Vector minv, float g, int d, Vector p)
{
	float           dd;

	dd = maxv[d] - minv[d];
	if (dd == 0.0) {
		CopyVector(maxv, p);
	} else {
		AddVector((g - minv[d]) / dd, maxv, (maxv[d] - g) / dd, minv, p);
	}

}


/*
 * * test if tri is crossed with cube defined by two corner **  g0, g1. ** *
 * For each side of the cube, **     calculate the intersection of tri and
 * 
 * the side (should be a line) **       test if the line is clipped with the
 * grid which is **             the projection of cube to the side.
 */
int 
CrossOver(TrianglePtr tri, Vector g0, Vector g1)
{
	int             side, x;
	VectorPtr       maxv, midv, minv;
	Vector          p0, p1;

	side = 0;
	maxv = tri->p[0];
	midv = tri->p[1];
	minv = tri->p[2];
	for (x = 0; x < 3; x++) {
		sort(&maxv, &midv, &minv, x);	/* sorted by index x  */

		if (g1[x] < minv[x])
			return FALSE;
		InterpVector(maxv, minv, g1[x], x, p0);
		if (g1[x] < midv[x]) {
			InterpVector(midv, minv, g1[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return TRUE;
		} else if (g1[x] <= maxv[x]) {
			InterpVector(maxv, midv, g1[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return TRUE;
		} else
			side++;

		if (g0[x] > maxv[x])
			return FALSE;
		InterpVector(maxv, minv, g0[x], x, p0);
		if (g0[x] > midv[x]) {
			InterpVector(maxv, midv, g0[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return TRUE;
		} else if (g0[x] >= minv[x]) {
			InterpVector(midv, minv, g0[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return TRUE;
		} else
			side++;
	}
	return (side == 6);
}


int 
CountCrossTri(int olist, int *tlist, Vector g0, Vector g1)
{
	int             tp, n;

	*tlist = TriListStorePtr;
	n = 0;
	while ((tp = TriListStore[olist++]) >= 0)
		if (CrossOver(&TriStore[tp], g0, g1)) {
			TriListStore[TriListStorePtr++] = tp;
			n++;
		}
	TriListStore[TriListStorePtr++] = -1;

	/* for debug use */
	if (TriListStorePtr >= MaxTriList) {
		printf("TriListStorePtr  exceeded\n");
		exit(1);
	}
	return n;
}


int 
partition(int list, int n, int level, Vector g0, Vector g1)
{
	TreeNode       *tn;
	Vector          q0, q1, gg;
	int             i, list1, index;

	index = TreeNodeStorePtr;
	tn = &TreeNodeStore[TreeNodeStorePtr++];

	/* for debug use */
	if (TreeNodeStorePtr >= MaxTreeNode) {
		printf("treenodenum exceeded\n");
		exit(1);
	}
	if ((g1[0] - g0[0]) < MinGridLen)
		MinGridLen = (g1[0] - g0[0]);

	CopyVector(g0, tn->g0);
	CopyVector(g1, tn->g1);
	tn->list = list;

	if ((n <= MaxTriPerNode) || (level >= MaxLevel)) {
		for (i = 0; i < 8; i++)
			tn->sub[i] = -1;
		return index;
	}
	AddVector(0.5, g0, 0.5, g1, gg);	/* midv point of g0, g1 */

	InitVector(q0, g0[0], g0[1], g0[2]);
	InitVector(q1, gg[0], gg[1], gg[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[0] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, gg[0], g0[1], g0[2]);
	InitVector(q1, g1[0], gg[1], gg[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[1] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, g0[0], gg[1], g0[2]);
	InitVector(q1, gg[0], g1[1], gg[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[2] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, gg[0], gg[1], g0[2]);
	InitVector(q1, g1[0], g1[1], gg[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[3] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, g0[0], g0[1], gg[2]);
	InitVector(q1, gg[0], gg[1], g1[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[4] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, gg[0], g0[1], gg[2]);
	InitVector(q1, g1[0], gg[1], g1[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[5] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, g0[0], gg[1], gg[2]);
	InitVector(q1, gg[0], g1[1], g1[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[6] = partition(list1, n, level + 1, q0, q1);

	InitVector(q0, gg[0], gg[1], gg[2]);
	InitVector(q1, g1[0], g1[1], g1[2]);
	n = CountCrossTri(list, &list1, q0, q1);
	tn->sub[7] = partition(list1, n, level + 1, q0, q1);

	return index;
}


void 
BuildTree(void)
{
	int             i;

	for (i = 0; i < trinum; i++)
		TriListStore[i] = i;
	TriListStorePtr = trinum;
	TriListStore[TriListStorePtr++] = -1;

	TreeNodeStorePtr = 0;

	MinGridLen = G1[0] - G0[0];

	partition(0, trinum, 0, G0, G1);

	GridNum = (int) ((G1[0] - G0[0]) / MinGridLen);

	printf("BuildTree over, TreeNode # is %d, Trilist # is %d\n",
	       TreeNodeStorePtr, TriListStorePtr);
}




float 
NextPoint(Vector p, Vector v, Vector g0, Vector g1, Vector q)
{
	float           s, t;
	int             i;

	s = t = 1000000.0;
	for (i = 0; i < 3; i++) {
		if (v[i] > 0.0)
			s = (g1[i] - p[i]) / v[i];
		else if (v[i] < 0.0)
			s = (g0[i] - p[i]) / v[i];
		if (s < t)
			t = s;
	}
	t += 0.1;
	Add1Vector(p, t, v, q);
	return t;
}


int 
TreeNodeNum(Vector p)
{
	int             i, ctn, gsize;
	int             gnum[3], index;

	if ((p[0] < G0[0]) || (p[0] > G1[0]) ||
	    (p[1] < G0[1]) || (p[1] > G1[1]) ||
	    (p[2] < G0[2]) || (p[2] > G1[2]))
		return -1;

	for (i = 0; i < 3; i++)
		gnum[i] = (int) (((double) p[i] - (double) G0[i]) / MinGridLen);

	ctn = 0;
	gsize = GridNum;
	while (TreeNodeStore[ctn].sub[0] >= 0) {
		gsize >>= 1;
		index = 0;
		for (i = 0; i < 3; i++)
			if (gnum[i] >= gsize) {
				index += (1 << i);
				gnum[i] -= gsize;
			}
		ctn = TreeNodeStore[ctn].sub[index];
	}

	return ctn;
}


int 
TreeNodeNum1(Vector p)
{
	int             i, ctn, index;
	Vector          gg;

	if ((p[0] < G0[0]) || (p[0] > G1[0]) ||
	    (p[1] < G0[1]) || (p[1] > G1[1]) ||
	    (p[2] < G0[2]) || (p[2] > G1[2]))
		return -1;

	ctn = 0;
	while (TreeNodeStore[ctn].sub[0] >= 0) {
		AddVector(0.5, TreeNodeStore[ctn].g0,
			  0.5, TreeNodeStore[ctn].g1, gg);
		index = 0;
		for (i = 0; i < 3; i++)
			if (p[i] >= gg[i])
				index += (1 << i);
		ctn = TreeNodeStore[ctn].sub[index];
	}

	return ctn;
}



int 
TriHitted(Vector p, Vector v, TrianglePtr tp, float *t)
{
	float           a1, a2;
	Vector          vv, q;

	a1 = InnerProd(v, tp->n);
	VectorTo(p, tp->c, vv);
	a2 = InnerProd(vv, tp->n);

	/**********************************************************
		t < 0 means target triangle is in the oppsite side.
	**********************************************************/
	if ((*t = a2 / a1) < 0.0001)
		return FALSE;
	Add1Vector(p, *t, v, q);

	if ((q[0] * (tp->se[0][0]) + q[1] * (tp->se[0][1]) +
	     q[2] * (tp->se[0][2]) + tp->se[0][3]) < 0.0)
		return FALSE;
	if ((q[0] * (tp->se[1][0]) + q[1] * (tp->se[1][1]) +
	     q[2] * (tp->se[1][2]) + tp->se[1][3]) < 0.0)
		return FALSE;
	if ((q[0] * (tp->se[2][0]) + q[1] * (tp->se[2][1]) +
	     q[2] * (tp->se[2][2]) + tp->se[2][3]) < 0.0)
		return FALSE;
	return TRUE;
}


int 
TriListHitted(Vector p, Vector v, int tlist, int otri)
{
	int             tri, trii;
	float           trit, t;

	if (tlist < 0)
		return -1;
	trii = -1;
	trit = 1000000.0;
	while ((tri = TriListStore[tlist++]) >= 0) {
		if (tri == otri)
			continue;
		if (TriHitted(p, v, &TriStore[tri], &t) && (t < trit)) {
			trii = tri;
			trit = t;
		}
	}
	return trii;
}


int 
RayHitted(Vector p, Vector v, int otri)
{
	int             tn, otn, tri;
	float           t;
	Vector          q;

	CopyVector(p, q);
	otn = -1;
	while ((tn = TreeNodeNum(q)) >= 0) {
		if (otn == tn) {
			fprintf(stderr, "*** Precision Exceeded !!! \n");
			fprintf(stderr, "*** tri is %d\n", otri);
			t += 1.0;
			Add1Vector(p, t, v, q);
			continue;
		}
		otn = tn;

		if ((tri = TriListHitted(p, v, TreeNodeStore[tn].list, otri)) >= 0)
			return tri;
		t = NextPoint(p, v, TreeNodeStore[tn].g0, TreeNodeStore[tn].g1, q);
	}
	return -1;
}
