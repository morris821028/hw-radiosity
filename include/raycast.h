#ifndef _RAYCAST_H
#define _RAYCAST_H

int Clip(Vector p0,Vector p1,Vector g0,Vector g1,int x,int y);
void BuildTree(void);
int RayHitted(Vector p, Vector v, int otri);

#endif


