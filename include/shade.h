#ifndef _SHADE_H
#define _SHADE_H

int Shade(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, int realdest);
float CalFF(TrianglePtr srctri, int logsrc, TrianglePtr destri, int logdest, Vector p);
#endif

