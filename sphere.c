/* Program to generate a sphere as polygons.*/
#include <math.h>
#include "3d.h"
#include <stdio.h>

/* Number of bands (meridians) and parallels around sphere */
/* 3,1 = minimum sphere */
#define NBANDS 32
#define NPARAS 32
#define NPTS (NBANDS*NPARAS+2)
#define RADIUS 1500
#define npolys (NBANDS*NPARAS+2*NBANDS)
#define COLOUR 14

/* Reverse order for inside-out sphere */
#define SEQ1 2
#define SEQ2 1
#define SEQ3 0
#define SEQ4 3

int pnum[NBANDS][NPARAS];
s_pnt point[NPTS];
s_poly polygn[npolys];

FILE *fp;

void makepoints()
{
 int nextpt=2;
 int band,para;
 double theta,chi,k;	/* theta = angle west, chi=angle south */

 point[0].x=0;
 point[0].y=0;
 point[0].z=RADIUS;

 point[1].x=0;
 point[1].y=0;
 point[1].z=-RADIUS;


 for(band=0;band<NBANDS;band++)
 {
  theta=(PI*2*band)/NBANDS;
  for(para=0;para<NPARAS;para++)
  {
   chi=(PI*(para+1))/(NPARAS+1); /* semicircle minus end points */
   k=RADIUS*sin(chi);

   point[nextpt].x=k*sin(theta);
   point[nextpt].y=k*cos(theta);
   point[nextpt].z=RADIUS*cos(chi);

   pnum[band][para]=nextpt;
   nextpt++;
  }
 }
}

void makepolys()
{
 int i,p;
 int nextpoly=0;
 for(i=0;i<NBANDS;i++)
 {
  polygn[nextpoly].v[SEQ1]=0;
  polygn[nextpoly].v[SEQ2]=pnum[i][0];
  polygn[nextpoly].v[SEQ3]=pnum[(i+1)%NBANDS][0];
  nextpoly++;
 }

 for(i=0;i<NBANDS;i++)
 {
  polygn[nextpoly].v[SEQ1]=1;
  polygn[nextpoly].v[SEQ2]=pnum[(i+1)%NBANDS][NPARAS-1];
  polygn[nextpoly].v[SEQ3]=pnum[i][NPARAS-1];
  nextpoly++;
 }

 for(i=0;i<NBANDS;i++)
  for(p=0;p<NPARAS-1;p++)
  {
   polygn[nextpoly].v[SEQ1]=pnum[i][p];
   polygn[nextpoly].v[SEQ2]=pnum[i][p+1];
   polygn[nextpoly].v[SEQ3]=pnum[(i+1)%NBANDS][p+1];
   polygn[nextpoly].v[SEQ4]=pnum[(i+1)%NBANDS][p];
   nextpoly++;
  }
}

void makesphere()
{
 int i;
 makepoints();
 makepolys();
 fprintf(fp,"1\n%d\n%d\n\n",npolys,NPTS);
 fprintf(fp,"0 0 0\n0 0 0\n0 %d\n\n",npolys-1);

 for(i=0;i<2*NBANDS;i++)
 {
  fprintf(fp,"3\n%d %d %d\n%d\n",polygn[i].v[0],
				polygn[i].v[1],
				polygn[i].v[2],
				COLOUR);
 }
 for(i=2*NBANDS;i<npolys;i++)
 {
  fprintf(fp,"4\n%d %d %d %d\n%d\n",polygn[i].v[0],
				polygn[i].v[1],
				polygn[i].v[2],
				polygn[i].v[3],
				COLOUR);
 }
 for(i=0;i<NPTS;i++)
 {
  fprintf(fp,"%d %d %d\n",(int)point[i].x,(int)point[i].y,(int)point[i].z);
 }
}

int main()
{
 printf("Program to create a polygon sphere - (C) Peter Clay.\n");

 fp=fopen("SPHERE.3d","wt");
 if(fp==NULL){printf("Cannot create file.\n");return -1;}

 makesphere();

 fclose(fp);
 printf("Complete. Wrote 1 object at (0,0,0) ; %d points, %d polygons.\n",NPTS,npolys);
 printf("This version compiled on %s\n",__DATE__);
 return 0;
}