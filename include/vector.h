#ifndef _VECTOR_H
#define _VECTOR_H

#include <math.h>
#include <assert.h>

typedef float	Vector[3];
typedef float	*VectorPtr;

typedef float 	Matrix[4][4];


#define InitVector(v,a,b,c) {(v)[0]=a; (v)[1]=b; (v)[2]=c;}
#define CopyVector(v1,v2)   {(v2)[0]=(v1)[0]; (v2)[1]=(v1)[1]; (v2)[2]=(v1)[2];}
#define ScaleVector(a,v1,v2) {\
                (v2)[0] = (a)* (v1)[0];\
                (v2)[1] = (a)* (v1)[1];\
                (v2)[2] = (a)* (v1)[2];}
#define Add1Vector(v1,b,v2,v3) {\
                (v3)[0] = (v1)[0] + (b)* (v2)[0];\
                (v3)[1] = (v1)[1] + (b)* (v2)[1];\
                (v3)[2] = (v1)[2] + (b)* (v2)[2];}
#define AddVector(a,v1,b,v2,v3) {\
                (v3)[0] = (a)* (v1)[0] + (b)* (v2)[0];\
                (v3)[1] = (a)* (v1)[1] + (b)* (v2)[1];\
                (v3)[2] = (a)* (v1)[2] + (b)* (v2)[2];}
#define AverageVector(v1,v2,v3) {\
	        (v3)[0] = ((v1)[0] + (v2)[0])/2.0;\
                (v3)[1] = ((v1)[1] + (v2)[1])/2.0;\
                (v3)[2] = ((v1)[2] + (v2)[2])/2.0;}
	
	
#define InterVector(v1,v2,t,v3) AddVector((1-(t)),(v1),(t),(v2),(v3))
#define MinusVector(v)   {(v)[0] = -(v)[0]; (v)[1] = -(v)[1]; (v)[2] = -(v)[2];}
#define SubtractVector(v1,v2)	{(v1)[0] -= (v2)[0]; (v1)[1] -= (v2)[1]; (v1)[2] -= (v2)[2];}
#define InnerProd(v1,v2) ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])
#define VectorTo(v1,v2,v3) {\
                (v3)[0]=(v2)[0]-(v1)[0];\
                (v3)[1]=(v2)[1]-(v1)[1];\
                (v3)[2]=(v2)[2]-(v1)[2]; }
#define CosTheta(v1,v2) (InnerProd(v1,v2)/norm(v1)/norm(v2))

#define ReadVector(fp,v)    {assert(fscanf(fp,"%f%f%f",&(v[0]),&(v[1]),&(v[2])) == 3);}
#define ReadIVector(fp,v)   {assert(fscanf(fp,"%d%d%d",&(v[0]),&(v[1]),&(v[2])) == 3);}


#define  Round(x) ( (x)>=0? (int)((x)+0.5): (int)((x)-0.5) )
#define  max(x,y) ( (x)>(y)?(x):(y) )
#define  min(x,y) ( (x)<(y)?(x):(y) )
#define PrintV(t) (printf("%f %f %f\n",(t)[0],(t)[1],(t)[2]))
#define PrintVI(t) (printf("%i %i %i\n",(t)[0],(t)[1],(t)[2]))

/* calculate geometry */
#define CalCenter(tp) {\
    	     	(tp)->c[0] = ((tp)->p[0][0]+(tp)->p[1][0]+(tp)->p[2][0])/3.0;\
		(tp)->c[1] = ((tp)->p[0][1]+(tp)->p[1][1]+(tp)->p[2][1])/3.0;\
		(tp)->c[2] = ((tp)->p[0][2]+(tp)->p[1][2]+(tp)->p[2][2])/3.0; }
#define CalArea(tp,v0,v1) {\
		VectorTo((tp)->p[0], (tp)->p[1], v0);\
		VectorTo((tp)->p[0], (tp)->p[2], v1);\
		CrossProd(v0, v1, (tp)->n);\
		(tp)->area = norm((tp)->n)/2; }
	

inline float norm(Vector v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
inline float norm2(Vector v) {
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
inline void CrossProd(Vector v1, Vector v2, Vector v) {
	v[0] = v1[1] * v2[2] - v2[1] * v1[2];
	v[1] = v1[2] * v2[0] - v2[2] * v1[0];
	v[2] = v1[0] * v2[1] - v2[0] * v1[1];
}
inline void normalize(Vector p) {
	float t = norm(p);
	p[0] /= t, p[1] /= t, p[2] /= t;
}
inline void PrintVector(char *s, Vector v) {
	printf("%s %f %f %f\n", s, (v)[0], (v)[1], (v)[2]);
}
#endif
