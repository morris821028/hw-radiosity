#include "rad.h"
#include "raycast.h"
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unordered_set>
#include <unordered_map>
using namespace std;

int             TriListStore[MaxTriList];
int             TriListStorePtr;

TreeNode        TreeNodeStore[MaxTreeNode];
int             TreeNodeStorePtr;

float           MinGridLen;
int             GridNum;


/**
 * two-dimension clip (at dimension x & y) clip line (p0 -- p1) with grid
 * defined by corner (go,g1). We only detect if the intersection of line
 * with the grid (interior included) is empty.
 */
static int Clip(Vector p0, Vector p1, Vector g0, Vector g1, int x, int y)
{
	char mask1 = (p0[x] < g0[x])<<0 |
		(p0[x] > g1[x])<<1 |
		(p0[y] < g0[y])<<2 |
		(p0[y] > g1[y])<<3 ;
	char mask2 = (p1[x] < g0[x])<<0 |
		(p1[x] > g1[x])<<1 |
		(p1[y] < g0[y])<<2 |
		(p1[y] > g1[y])<<3 ;
	if (mask1&mask2)	
		return false;
	if (!(mask1|mask2))
		return true;

	float a, b, s, t;
	a = p1[y] - p0[y];
	b = p1[x] - p0[x];

	t = a * (g0[x] - p0[x]) - b * (g0[y] - p0[y]);
	if (t == 0)
		return true;

	s = a * (g0[x] - p0[x]) - b * (g1[y] - p0[y]);
	if ((s < 0) != (t < 0))
		return true;

	s = a * (g1[x] - p0[x]) - b * (g0[y] - p0[y]);
	if ((s < 0) != (t < 0))
		return true;

	s = a * (g1[x] - p0[x]) - b * (g1[y] - p0[y]);
	if ((s < 0) != (t < 0))
		return true;

	return false;
}


/**
 * sort three vector with respect to dimension d
 */
static inline void sort(VectorPtr * maxv, VectorPtr * midv, VectorPtr * minv, int d)
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
static void InterpVector(Vector maxv, Vector minv, float g, int d, Vector p)
{
	float dd = maxv[d] - minv[d];
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
static int CrossOver(TrianglePtr tri, Vector g0, Vector g1)
{
	VectorPtr       maxv, midv, minv;
	Vector          p0, p1;

	maxv = tri->p[0];
	midv = tri->p[1];
	minv = tri->p[2];
	for (int x = 0; x < 3; x++) {
		sort(&maxv, &midv, &minv, x);	/* sorted by index x  */

		if (g1[x] < minv[x])
			return false;
		InterpVector(maxv, minv, g1[x], x, p0);
		if (g1[x] < midv[x]) {
			InterpVector(midv, minv, g1[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return true;
			else
				return false;
		} else if (g1[x] <= maxv[x]) {
			InterpVector(maxv, midv, g1[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return true;
			else
				return false;
		}

		if (g0[x] > maxv[x])
			return false;
		InterpVector(maxv, minv, g0[x], x, p0);
		if (g0[x] > midv[x]) {
			InterpVector(maxv, midv, g0[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return true;
			else
				return false;
		} else if (g0[x] >= minv[x]) {
			InterpVector(midv, minv, g0[x], x, p1);
			if (Clip(p0, p1, g0, g1, (x + 1) % 3, (x + 2) % 3))
				return true;
			else
				return false;
		}
	}
	return true;
}

static inline vector<int> CountCrossTriangle(vector<int> &triangleIdxs, Vector g0, Vector g1)
{
	vector<int> ret;
	ret.reserve(triangleIdxs.size());
	for (auto idx: triangleIdxs) {
		if (CrossOver(&TriStore[idx], g0, g1))
			ret.push_back(idx);
	}
	return ret;
}

static int buildTree(vector<int> triangleIdxs, int level, Vector g0, Vector g1)
{
	int n = triangleIdxs.size();
	int index = TreeNodeStorePtr;
	TreeNode *tn = &TreeNodeStore[TreeNodeStorePtr++];

	/* for debug use */
	if (TreeNodeStorePtr >= MaxTreeNode) {
		printf("treenodenum exceeded\n");
		exit(1);
	}
	if (g1[0] - g0[0] < MinGridLen)
		MinGridLen = (g1[0] - g0[0]);

	CopyVector(g0, tn->g0);
	CopyVector(g1, tn->g1);

	if (n <= MaxTriPerNode || level >= MaxLevel) {
		for (int i = 0; i < 8; i++)
			tn->sub[i] = -1;
		tn->list = TriListStorePtr;
		for (auto idx: triangleIdxs)
			TriListStore[TriListStorePtr++] = idx;
		TriListStore[TriListStorePtr++] = -1;
		return index;
	}

	Vector gg;
	AddVector(0.5, g0, 0.5, g1, gg);	/* midv point of g0, g1 */

	for (int i = 0; i < 8; i++) {
		Vector q0, q1;
		InitVector(q0, g0[0], g0[1], g0[2]);
		InitVector(q1, g1[0], g1[1], g1[2]);
		for (int j = 0; j < 3; j++) {
			if ((i>>j)&1) {
				q0[j] = gg[j];
			} else {
				q1[j] = gg[j];
			}
		}
		vector<int> filterIdxs(CountCrossTriangle(triangleIdxs, q0, q1));
		tn->sub[i] = buildTree(filterIdxs, level+1, q0, q1);
	}
	return index;
}


void BuildTree(void)
{
	vector<int> triangleIdxs(trinum);
	for (int i = 0; i < trinum; i++)
		triangleIdxs[i] = i;
	TriListStorePtr = 0;

	TreeNodeStorePtr = 0;

	MinGridLen = G1[0] - G0[0];

	buildTree(triangleIdxs, 0, G0, G1);

	GridNum = (int) ((G1[0] - G0[0]) / MinGridLen);

	printf("BuildTree over, TreeNode # is %d, Trilist # is %d\n",
			TreeNodeStorePtr, TriListStorePtr);
}




static inline float NextPoint(Vector p, Vector v, Vector g0, Vector g1, Vector q)
{
	float t_min = 1000000.0;
	for (int i = 0; i < 3; i++) {
		if (fabs(v[i]) > 1e-6) {
			float s;
			if (v[i] > 0)
				s = (g1[i] - p[i]) / v[i];
			else
				s = (g0[i] - p[i]) / v[i];
			t_min = min(t_min, s);
		}
	}
	t_min += 0.1;
	Add1Vector(p, t_min, v, q);
	return t_min;
}


static inline int TreeNodeNum(Vector p)
{
	int             ctn, gsize;
	int             gnum[3], index;

	if ((p[0] < G0[0]) || (p[0] > G1[0]) ||
			(p[1] < G0[1]) || (p[1] > G1[1]) ||
			(p[2] < G0[2]) || (p[2] > G1[2]))
		return -1;

	for (int i = 0; i < 3; i++)
		gnum[i] = (int) (((double) p[i] - (double) G0[i]) / MinGridLen);

	ctn = 0;
	gsize = GridNum;
	while (TreeNodeStore[ctn].sub[0] >= 0) {
		gsize >>= 1;
		index = 0;
		for (int i = 0; i < 3; i++) {
			if (gnum[i] >= gsize) {
				index |= (1 << i);
				gnum[i] -= gsize;
			}
		}
		ctn = TreeNodeStore[ctn].sub[index];
	}

	return ctn;
}

//int TriHitted(Vector p, Vector v, TrianglePtr tp, float *t)
static int TriHitted(Vector p, Vector v, TrianglePtr tp, float *t)
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
		return false;
	Add1Vector(p, *t, v, q);

	if ((q[0] * (tp->se[0][0]) + q[1] * (tp->se[0][1]) +
				q[2] * (tp->se[0][2]) + tp->se[0][3]) < 0.0)
		return false;
	if ((q[0] * (tp->se[1][0]) + q[1] * (tp->se[1][1]) +
				q[2] * (tp->se[1][2]) + tp->se[1][3]) < 0.0)
		return false;
	if ((q[0] * (tp->se[2][0]) + q[1] * (tp->se[2][1]) +
				q[2] * (tp->se[2][2]) + tp->se[2][3]) < 0.0)
		return false;
	return true;
}


static inline int TriListHitted(Vector p, Vector v, int tlist, int src_tri)
{
	int tri_idx, tri_tmp;
	float t_min, t_tmp;

	if (tlist < 0)
		return -1;
	tri_idx = -1;
	t_min = 1000000.0;
	while ((tri_tmp = TriListStore[tlist++]) >= 0) {
		if (tri_tmp == src_tri)
			continue;
		if (TriHitted(p, v, &TriStore[tri_tmp], &t_tmp) && (t_tmp < t_min)) {
			tri_idx = tri_tmp;
			t_min = t_tmp;
		}
	}
	return tri_idx;
}
int RayHitted(Vector p, Vector v, int otri)
{
	int             tn, otn, tri;
	float           t(0);
	Vector          q;
	CopyVector(p, q);
	otn = -1;
	while ((tn = TreeNodeNum(q)) >= 0) {	// find the ID of the cude which the start point of the ray in
		if (otn == tn) {
			fprintf(stderr, "*** Precision Exceeded !!! \n");
			fprintf(stderr, "*** tri is %d\n", otri);
			t += 1.0;
			Add1Vector(p, t, v, q);
			continue;
		}
		otn = tn;

		if ((tri = TriListHitted(p, v, TreeNodeStore[tn].list, otri)) >= 0)	{ // test the triangle in this cube
			return tri;
		}
		t = NextPoint(p, v, TreeNodeStore[tn].g0, TreeNodeStore[tn].g1, q);
	}
	return -1;
}
