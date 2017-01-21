#include <stdio.h>
#include "3d.h"

/* Quick program to load a DXF file into 3d file format. */

s_pnt *point;
s_poly *poli;
int npts=0,npolys=0;
FILE *dxf;
FILE *out;

void loadDXF(void);

#define MAXPOINTS 2000
#define MAXPOLYS 1000

int COLOUR=50;

int getpointnum(int x,int y,int z)
{
 int i;
 i=0;
 while( (
 (point+i)->x != x ||
 (point+i)->y != y ||
 (point+i)->z != z )&& i<npts)i++;
 if(i!=npts)return i;

 if(i>MAXPOINTS){printf("Too many points.\n");exit(1);}

 (point+i)->x = x;
 (point+i)->y = y;
 (point+i)->z = z;

 npts++;
 return npts-1;
}

void loadDXF()
{
 char tstr[100]="blanks";
 int i,j,n,match,prevpoint;
 float x=0,y=0,z=0;

 y=sin(x);/* Just to link FPU code */

 while(stricmp(tstr,"EOF")!=0)
 {
  do
  {
   n=fscanf(dxf,"%s",tstr);
   printf("%s ",tstr);
   match=(stricmp(tstr,"3DFACE")==0);
   if(stricmp(tstr,"EOF")==0)break;
  }while(n && !match);
  if(!match)break;
  /* Now, next input fields list polygon vertices */
  prevpoint=-1;
  for(i=0;i<6;i++)
  {
   fscanf(dxf,"%d",&n);
   printf("%d\t%f %f %f\n",n,x,y,z);
   if(n==0)break;/* no more points */
   fscanf(dxf,"%f",&x);
   fscanf(dxf,"%d %f %d %f",&j,&y,&j,&z);
   j=getpointnum((int)x,(int)y,(int)z);
   if(j==prevpoint)i--;
   (poli+npolys)->v[i]=j;
   prevpoint=j;
  }
  (poli+npolys)->np=i;
  npolys++;
 }
}

void save3d()
{
 int i,j;
 s_poly p;
 s_pnt pt;

 fprintf(out,"1 %d %d\n\n",npolys,npts);

 fprintf(out,"0 0 0\n0 0 0\n0 %d\n\n",npolys-1);

 for(i=0;i<npolys;i++)
 {
  p=*(poli+i);

  fprintf(out,"%d\t",p.np);
  for(j=0;j<p.np;j++)fprintf(out,"%d ",p.v[j]);
  fprintf(out,"\t%d\n",COLOUR);

/*  fprintf(out,"%d\t",p.np);
  for(j=p.np-1;j>=0;j--)fprintf(out,"%d ",p.v[j]);
  fprintf(out,"\t%d\n",COLOUR);*/
 }

 for(i=0;i<npts;i++)
 {
  pt=*(point+i);
  fprintf(out,"%ld %ld %ld\n",pt.x,pt.y,pt.z);
 }
}

int main(int argc,char *argv[])
{
 poli=malloc(MAXPOLYS*sizeof(s_poly));
 point=malloc(MAXPOINTS*sizeof(s_pnt));
 if(poli==NULL)puts("Memory - 2");
 if(point==NULL)puts("Memory - 1");


 if(argc!=4){printf("Are you dense?\n\a\n DXF-3D <in> <out> <colour>\n");exit(1);}

 dxf=fopen(argv[1],"rt");
 if(dxf==NULL){printf("File not found!\n");exit(1);}

 out=fopen(argv[2],"wt");
 if(out==NULL)exit(2);

 COLOUR=atoi(argv[3]);

 loadDXF();

 save3d();

 free(poli);
 free(point);
 return 0;
}