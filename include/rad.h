#include <stdio.h>
#include "vector.h"

#define PI		(3.1415926535897932)

#define MaxLevel	6

#define MaxTri		1000000
#define MaxTriList	200000
#define MaxTreeNode	20000
#define MaxTriPerNode   10

#define ffabs(x) (float)fabs((double)(x))


typedef struct {
	Vector	p[3];		/* the vertices */
	int     Frgb[3];	/* the foreground R G B color vector */
	int     Brgb[3];	/* the background R G B color vector */
	Vector 	accB[3];	/* accumulated Radisoity */
	Vector	deltaaccB[3];	/* delta accumulated Radisoty in this iteration */
	Vector 	deltaB; 	/* delta radiosity for exposing */
	float   se[3][4];	/* plane equations of the sides */
	Vector	c,n;		/* center and normal */
	float	area;		/* area */
	int	parent;		/* Initially set to itself */
				/* -1 : logical triangle */
				/* otherwise point to logical parent tri */
	int	neighbor[3];	/* neighbor adjacent to p[0]-[1], p[1]-[2], p[2]-[0] */
} Triangle, *TrianglePtr;

typedef struct {
	Vector	g0,g1;
	int	sub[8];
	int	list;
} TreeNode;


extern Triangle	TriStore[MaxTri];
extern int	TriStorePtr;

extern int	TriListStore[MaxTriList];
extern int	TriListStorePtr;

extern TreeNode	TreeNodeStore[MaxTreeNode];
extern int	TreeNodeStorePtr;

extern int	trinum;
extern Vector	G0, G1;

extern float	MinGridLen;
extern int	GridNum;

extern int	Debug;
extern float	AreaLimit;
extern float	SampleArea;
extern float	ConvergeLimit;
extern float	DeltaFFLimit;
