#include <stdio.h>
#include "vector.h"


void 
PrintVector(char *s, Vector v)
{
	printf("%s %f %f %f\n", s, (v)[0], (v)[1], (v)[2]);
}


float 
norm(Vector v)
{
	return (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float 
norm2(Vector v)
{
	return (float) (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void 
CrossProd(Vector v1, Vector v2, Vector v)
{
	v[0] = v1[1] * v2[2] - v2[1] * v1[2];
	v[1] = v1[2] * v2[0] - v2[2] * v1[0];
	v[2] = v1[0] * v2[1] - v2[0] * v1[1];
}

void 
normalize(Vector p)
{
	float           t;

	t = norm(p);
	p[0] /= t;
	p[1] /= t;
	p[2] /= t;
}
