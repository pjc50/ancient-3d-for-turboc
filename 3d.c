#include <glib2.h>
#include "3d.h"
#include <stdlib.h>
#define SVAL 512
#define VIEWFACTOR 1000

/* Global variables */

s_world w;
s_obj  *obj;
s_pnt  *pnt,light;  /* All points + lightsource */
s_poly *polygon;

s_panel panel[500];
int npanels,num_panels;
int c_panel;

long cosn[MAXANGLE],sinn[MAXANGLE]; /* Look-up tables:all values *SVAL; MAXANGLE unit in a revolution */

/* Function code */

void line(int x1,int y1,int x2,int y2)
{
#ifdef WIREFRAME
 int o1,o2,x,y;
 long t;
 o1=onscr(x1,y1);
 o2=onscr(x2,y2);
 if(o1==0 && o2==0) /* If both points are onscreen , then it's easy. */
 {
  _moveto(x1,y1);
  _lineto(x2,y2);
 }
  else
 {
  if(o1==0 || o2==0) /* If one is onscreen, then which? */
  {
   if(o1==0)          /* point 1 on ? */
   {
    /* leave points alone */
   }
   else  /* if not then point 2 is on */
   {
    /* swap p1 & p2 */
    t=x1;
    x1=x2;
    x2=t;
    t=y1;
    y1=y2;
    y2=t;
   }

   if(x2>S_W || x2<0)    /* x off */
   {
    x=(x2>0)?S_W:0;
    t=y2-y1;
    t=(t*(x-x1))/(x2-x1);
    y=(int)t+y1;
    _moveto(x1,y1);
    _lineto(x,y);
   }
   else        /* y off */
   {
    y=(y2>w.top)?w.hgt:w.top;
    t=x2-x1;
    t=(t*(y-y1))/(y2-y1);
    x=(int)t+x1;
    _moveto(x1,y1);
    _lineto(x,y);
   }
  }
 }
#else
 x1=x2;
 y1=y2;
 x2=x1;
 y2=y1;
#endif
}/***********************************************************************/




/* End of line function !!!!!!!!!!!!!!!!!!!!!!*/




/********************************************************************/


void getm(matrix *mat,s_trans *c)
{
 long x,y,z;
 x=c->rotx;
 y=c->roty;
 z=c->rotz;

 mat->xx=((cosn[z]*cosn[y])+(sinn[y]*-sinn[x]*-sinn[z])/SVAL)/SVAL;
 mat->xy=(cosn[x]*-sinn[z])/SVAL;
 mat->xz=((-sinn[y]*cosn[z])+(cosn[y]*-sinn[x]*-sinn[z])/SVAL)/SVAL;

 mat->yx=((cosn[y]*sinn[z])+(sinn[y]*-sinn[x]*cosn[z])/SVAL)/SVAL;
 mat->yy=(cosn[x]*cosn[z])/SVAL;
 mat->yz=((-sinn[y]*sinn[z])+(cosn[y]*-sinn[x]*cosn[z])/SVAL)/SVAL;

 mat->zx=(sinn[y]*cosn[x])/SVAL;
 mat->zy=sinn[x];
 mat->zz=(cosn[y]*cosn[x])/SVAL;
}


void doobj(int n)
{
 s_obj o;
 int i;
 matrix m,*mat=&m;

 o=obj[n];
 if(n==0)num_panels=0;


 w.itcode=n+1;


 if(o.pos.rotx==0 && o.pos.roty==0 && o.pos.rotz==0)
  mat=NULL;
 else
  getm(mat,&o.pos);

 for(i=o.startp;i<=o.lastp;i++)    /* process each polygon */
 {
  dopoly(i,mat,&o.pos);
 }
}

int dopoly(int n,matrix *vect,s_trans *v)
{
 /* Simply process each point then draw it */
 int i,j;
 s_poly p;

 num_panels++;

 p=polygon[n];


 for(i=0;i<p.np;i++)  /* for each point... */
 {
  j=dopoint(p.v[i],vect,v);
/*  if(j>1)break;*/
 }
 if(i!=p.np)return 1;   /* if jumped out early because a point was invisible,
			don't draw the shape. */

 if(!(p.colour & 0x100))/* one - sided if bit 8 set */
  if( visible(p.v[0],p.v[1],p.v[2])!=1 )return 0;

 if(p.colour & 0x200)
	panel[c_panel].c=p.colour % 256;
	else
	{
		j=shadepanel(&p);
		if(j<0 && (p.colour & 0x100))j=-j;/* If one - sided, choose brightest. */
		panel[c_panel].c=(p.colour+j)%256;
	}

 panel[c_panel].np=p.np;
 panel[c_panel].z=0;
 for(i=0;i<(p.np);i++) /* now fill in the panel structure */
 {
  panel[c_panel].x[i]=pnt[p.v[i]].px;
  panel[c_panel].y[i]=pnt[p.v[i]].py;
  panel[c_panel].z+=pnt[p.v[i]].pz;
 }
 panel[c_panel].z/=p.np;
 c_panel++;
 npanels++;
 j=j+1;
 return 0;
}

int visible(int p1,int p2,int p3)
{
 long ay,az,by,bz,vz;
 ay=pnt[p2].py-pnt[p1].py;
 az=pnt[p2].px-pnt[p1].px;
 by=pnt[p3].py-pnt[p2].py;
 bz=pnt[p3].px-pnt[p2].px;
 vz=(bz*ay)-(by*az);
 if(vz<1)return 1;
 return 0;
}

int shadepanel(s_poly *p)
{

 /* Compute colour of panel for polygon p.*/
 /* evaluate based on angle between p0->p1 and p0->light */
 long rval;
 long s1,s2;
 s_pnt p0,p1,p2,nrml;

 s1=1;
 s2=1;

 p0=pnt[p->v[0]];
 p1=pnt[p->v[1]];
 p2=pnt[p->v[2]];

 nrml.z=(p0.rx-p1.rx) *(p0.ry-p2.ry) - (p0.ry-p1.ry)*(p0.rx-p2.rx);
 nrml.y=(p0.rz-p1.rz) *(p0.rx-p2.rx) - (p0.rx-p1.rx)*(p0.rz-p2.rz);
 nrml.x=(p0.ry-p1.ry) *(p0.rz-p2.rz) - (p0.rz-p1.rz)*(p0.ry-p2.ry);

 /* cos theta = (a . b) / (|a| * |b|) */

 s1=nrml.x*nrml.x + nrml.y*nrml.y +nrml.z*nrml.z;
 s2=(p0.rx-light.rx)*(p0.rx-light.rx) + (p0.ry-light.ry)*(p0.ry-light.ry) + (p0.rz-light.rz)*(p0.rz-light.rz);

/* if(s1>0)s1=sqrt(s1);
 if(s2>0)s2=sqrt(s2);*/


 rval= nrml.x * (p0.rx-light.rx) + nrml.y * (p0.ry-light.ry) + nrml.z * (p0.rz-light.rz) ;

 if(s1<1)return 0;   /* Easy get-out conditions */
 if(s2<1)return 0;

 if(rval==0)return 0;

 rval=16.0*(double)(rval)/(sqrt((double)s1*s2));

 return rval;
}

/*****************************************
*
* Complex function for plotting point.
* 1. Rotate with m.
* 2. apply vect to position
* 3. apply eyetrans to "
* 4. scale to perspective.
*
*
*/

int dopoint(int n,matrix *T_m,s_trans *T_vect)
{
 s_pnt p;
 s_trans t; /* tempoary vector storage */
 s_trans vect;


 p=pnt[n];
 if(p.recalc!=w.itcode)       /* If not previously calculated */
 {
 vect=*T_vect;
 t.x=p.x;
 t.y=p.y;
 t.z=p.z;

 /* phase 1 - rotate then add VECT */
 if(T_m!=NULL)rot(&t,T_m);

 t.x+=vect.x;
 t.y+=vect.y;
 t.z+=vect.z;

 p.rx=t.x;
 p.ry=t.y;
 p.rz=t.z;

 /* phase 2 - ditto eyetrans */
 vect=w.e_t;


 t.x-=vect.x;
 t.y-=vect.y;
 t.z-=vect.z;

// rot(&t,&w.e_mat);


 if(t.z <0  )return(2); /* If behind the viewer then ignore */

 /* phase 3 - apply perspec. */
 p.px=((int)((t.x*VIEWFACTOR)/(t.z+VIEWFACTOR)))+HALFW;
 p.py=(int)((t.y*VIEWFACTOR)/(t.z+VIEWFACTOR))+w.hhgt;
 p.pz=t.z;

 }/* end of IF NOT PREVIOUSLY CALCULATED */

 p.recalc=w.itcode;
 pnt[n]=p;


 if(p.px>(S_W+100) || p.py>(w.hgt+100) || p.px< -100 || p.py<(w.top-100))return 3;
	/* if offscreen then tell draw routine to check it */

 return 1;
}

int companel(const void *p1,const void *p2)
{
 return
 (
 ((s_panel *)p2) ->z
 )
 -
 (
 ((s_panel *)p1) ->z
 );
}

void sortpanels()
{
 int n;
 qsort(panel,npanels,sizeof(s_panel),companel);
 for(n=0;n<npanels;n++)drawpanel(n);
 c_panel=0;
 npanels=0;
}



void drawpanel(int n)
{
 s_panel p;
 int i;
 p=panel[n];

 for(i=0;i<(p.np-2);i++) /* join point 0 up to other pairs of points.*/
 {
  Gtrngl(p.x[0  ],p.y[0  ],
	 p.x[i+1],p.y[i+1],
	 p.x[i+2],p.y[i+2],
	 p.c);
 }
}

void rot(s_trans *t,matrix *m_par)
{
 long x,y,z,tx,ty,tz;
 matrix m;
 m=*m_par;
 tx=t->x;
 ty=t->y;
 tz=t->z;
 x=m.xx*tx + m.xy*ty + m.xz*tz;
 y=m.yx*tx + m.yy*ty + m.yz*tz;
 z=m.zx*tx + m.zy*ty + m.zz*tz;
 t->x=x/SVAL;
 t->y=y/SVAL;
 t->z=z/SVAL;
}

void horizon(int colour)
{
 long x1,y1,x2,y2;
 int roll,elev,t;
 elev++;
 roll=w.e_t.rotz;
 x1=cosn[roll]<<2;
 y1=sinn[roll]<<2;
 x2=-cosn[roll]<<2;
 y2=-sinn[roll]<<2;
 if( labs(x1-x2) <= (labs(y1-y2)*S_W)/(w.hgt-w.top) )  /* y off */
 {
  if(y1<0){y1=-w.hhgt;t=w.hhgt;}else{y1=w.hhgt;t=-w.hhgt;};
  x1=(x1*t)/y2;
  x2=-x1;
  y2=-y1;
  if(y1>y2)
  {
   Gtrngl((int)x1+HALFW,(int)y1+w.hhgt,(int)x2+HALFW,(int)y2+w.hhgt,0,w.top,colour);
   Gtrngl(0,w.hgt,(int)x1+HALFW,(int)y1+w.hhgt,0,w.top,colour);
  }
  else
  {
   Gtrngl((int)x1+HALFW,(int)y1+w.hhgt,(int)x2+HALFW,(int)y2+w.hhgt,S_W,w.hgt,colour);
   Gtrngl(S_W,w.top,(int)x1+HALFW,(int)y1+w.hhgt,S_W,w.hgt,colour);
  }
 }
 else                                                /* x off */
 {
  if(x1<0){x1=-HALFW;t=HALFW;}else{x1=HALFW;t=-HALFW;};
  y1=(y1*t)/x2 -1;
  y2=-y1;
  x2=t;
  if(x1>x2)
  {
   Gtrngl((int)x1+HALFW,(int)y1+w.hhgt,(int)x2+HALFW,(int)y2+w.hhgt,0,w.hgt,colour);
   Gtrngl(S_W,w.hgt,(int)x1+HALFW,(int)y1+w.hhgt,0,w.hgt,colour);
  }
  else
  {
   Gtrngl((int)x1+HALFW,(int)y1+w.hhgt,(int)x2+HALFW,(int)y2+w.hhgt,0,w.top,colour);
   Gtrngl(S_W,w.top,(int)x2+HALFW,(int)y2+w.hhgt,0,w.top,colour);
  }
 }
}










/*************************
* Setup world & graphics
* 1. set mode
* 2. INIT w & lookups
*/

void init3d()
{
 int th;
 float a;

 npanels=0;
 c_panel=0;


 for(th=0;th<MAXANGLE;th++)
 {
  a=((float)th*PI*2.0)/(float)MAXANGLE;
  sinn[th]=(long)(sin(a)*(float)SVAL);
  cosn[th]=(long)(cos(a)*(float)SVAL);
/*  line(HALFW,HALFH,(int)sinn[th]/4+HALFW,(int)cosn[th]/4+HALFH);*/
 }

 w.e_t.x=0;
 w.e_t.y=0;
 w.e_t.z=-2000;
 w.e_t.rotx=0;
 w.e_t.roty=0;
 w.e_t.rotz=0;

 w.hgt  =200;
 w.hhgt =100;
 w.top  =0;

 light.rx=0;
 light.ry=0;
 light.rz=0;

 c_panel=0;
}

void close3d()
{
}

void loadtextobject(FILE *fl)
{
 long int i,j;
 int nobjs,npolys,npoints,k;

 fscanf(fl,"%ld",&i);
 obj=realloc(obj,(size_t)(i+w.nobjs)*sizeof(s_obj));
 nobjs=i;

 fscanf(fl,"%ld",&i);
 polygon=realloc(polygon,(size_t)(i+w.npolys)*sizeof(s_poly));
 npolys=i;

 fscanf(fl,"%ld",&i);
 pnt=realloc(pnt,(i+w.npoints)*sizeof(s_pnt));
 npoints=i;


 if(fl==NULL)exit(1);

 for(j=w.nobjs;j<(w.nobjs+nobjs);j++)
 {
  fscanf(fl,"%ld",&i);
  (obj+j)->pos.x=i;
  fscanf(fl,"%ld",&i);
  (obj+j)->pos.y=i;
  fscanf(fl,"%ld",&i);
  (obj+j)->pos.z=i;
  fscanf(fl,"%d",&i);
  (obj+j)->pos.rotx=i;
  fscanf(fl,"%d",&i);
  (obj+j)->pos.roty=i;
  fscanf(fl,"%d",&i);
  (obj+j)->pos.rotz=i;

  fscanf(fl,"%d",&i);
  (obj+j)->startp=i+w.npolys; /* add on num. of already loaded polygons to maintain relevance */
  fscanf(fl,"%d",&i);
  (obj+j)->lastp=i+w.npolys;
 }

 w.nobjs+=nobjs;

 for(j=w.npolys;j<(w.npolys+npolys);j++)
 {
  fscanf(fl,"%d",&i);
  (polygon+j)->np=i;
  for(k=0;k<(polygon+j)->np;k++)
  {
   fscanf(fl,"%d",&i);
   (polygon+j)->v[k]=i+w.npoints;
  }
  fscanf(fl,"%d",&i);
  (polygon+j)->colour=i;
 }

 w.npolys+=npolys;

 for(j=w.npoints;j<(w.npoints+npoints);j++)
 {
  fscanf(fl,"%ld",&i);
  (pnt+j)->x=i;
  fscanf(fl,"%ld",&i);
  (pnt+j)->y=i;
  fscanf(fl,"%ld",&i);
  (pnt+j)->z=i;
 }
 w.npoints+=npoints;
}

void loadall()
{
 FILE *fl,*b;
 int i,j;
 char fname[20];

 obj=malloc(1); /* Declare small blocks which can be realloced. */
 polygon=malloc(1);
 pnt=malloc(1);
 w.nobjs=0;
 w.npolys=0;
 w.npoints=0;

 b=fopen("world.dat","rt");
 if(b==0)exit(1);
 fscanf(b,"%d",&i);
 for(j=0;j<i;j++)
 {
  fscanf(b,"%s",fname);
  fl=fopen(fname,"rt");
  loadtextobject(fl);
  fclose(fl);
 }
}
