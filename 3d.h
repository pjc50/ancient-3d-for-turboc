#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef PI
#define PI 3.141592
#endif

#define MAXPTS 6

#pragma pack(4)

#define S_W 320
#define S_H 200
#define HALFW 160
#define HALFH 100

#define MAXANGLE 512

typedef struct
{
 long    xx,yx,zx,
	 xy,yy,zy,
	 xz,yz,zz;
}matrix;

typedef struct
{
 long x,y,z;
 int rotx,roty,rotz;
}s_trans;

typedef struct
{
 long x,y,z; /* values of position relative to object */
 long rx,ry,rz; /* transformed values - relative to origin */
 int px,py,pz; /* plot values + z into screen*/
 unsigned int recalc;  /* change value when recalculating */
}s_pnt;

typedef struct
{
 int np;  /* number of points */
 int v[MAXPTS]; /* max. 6 at moment */
 int colour;
}s_poly;

typedef struct
{
 s_trans pos; /* position */
 int startp, lastp; /* polygons used */
}s_obj;

typedef struct
{
 int npoints,npolys,nobjs;
 s_trans e_t;     /* Eye transform - remember coords. must be -ve. */
 matrix e_mat;
 s_trans dir;
 unsigned itcode; /* set pnt.recalc=itcode to show it has been processed */
 int hgt,top,hhgt;
}s_world;  /* Keep track of things in world */

typedef struct
{
 int x[MAXPTS],y[MAXPTS];
 long z;
 int np,c;
}s_panel;

/* Function defs. */

void init3d(void); /* standard setup. fill in lookups. */
void loadtextobject(FILE *); /* load all data */
void loadall(void);

int  dopoint(int,matrix *,s_trans *); /* process a point - return: 0-already done. 1-done. 2-offscreen */
int  dopoly(int,matrix *,s_trans *);  /* process & put in panel array */
void doobj(int);   /* process an object */
void doworld(void);

void drawpanel(int);    /* Draw one panel. No visibility check */
void sortpanels(void);  /* Sort panels, furthest first */
int  companel(const void *,const void *); /* Compare Z of panels - for Qsort */

void rot(s_trans *,matrix *);	/* Rotate a point by a matrix */
void getm(matrix *,s_trans *);  /* Make a matrix from a rotation */

void line(int,int,int,int);	/* Line clipped to screen DISABLED */
int  onscr(int,int);		/* check if a given point is on screen */
int  visible(int,int,int); /* See if tri. formed by 3 points is  facing view */
void horizon(int);		/* Draw coloured horizon */

/* Externs */

extern s_world w;
extern s_obj  *obj;
extern s_pnt  *pnt,light;
extern s_poly *polygon;
extern int num_panels;