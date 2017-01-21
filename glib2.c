// Glib 2 - Graphics routines for 320x200 (0x13) mode in Turbo C.






/* Basic use of GLIB 2 (please consult documentation)

First call Ginit() to allocate the double buffer.
Then call Gmode(0x13) to go to graphics mode.

Now you can use the graphics functions to draw a page,
then write it to the screen with Gflushbuffer().

To close, call Gshutdown() and Gmode(0x3).

*/




#include "glib2.h"
#include "dos.h"
#include "stdio.h"
#include "alloc.h"

static void _pal(int,int,int,int);
static char *zzendpad;
static void _Wswap(int *,int *);

#define S_W 320
#define S_H 199

struct CELheader
{
 unsigned version;		// usu bytes 19 91
 unsigned width,height,xpos,ypos;
 int junk1;
 long junk[5];
 char pal[256*3];
};
struct RGB Gpalette[256];

char *Gbuffer,*Gscreen;

//
//
//
//	Ginit, Gshutdown, Gmode
//
//
//

void Ginit()	/* Setup buffers and pointers */
{
 Gbuffer=calloc(6464,10);
 zzendpad=Gbuffer+64320;/* Protection */
 Gclearbuffer(0);
 Gscreen=MK_FP(0xA000,0);
 if(Gbuffer==NULL){Gmode(0x3);exit(100);}
}

void Gshutdown()	/* Opposite of Ginit() */
{
 if(Gbuffer!=NULL)
 {
  free(Gbuffer);
 }
}

void Gmode(int mode)	/* Set video mode by number */
{
 union REGS rg;
 rg.h.ah=0;			// AH=0 (set mode service)
 rg.h.al=mode;			// AL=Mode
 int86(0x10,&rg,&rg);		// Call video interrupt to set mode
}

//
//
//
//	Palette functions
//
//
//

void Gpal(int c,int r,int g,int b)
{
 Gpalette[c].r=r;
 Gpalette[c].g=g;
 Gpalette[c].b=b;
 _pal(c,r,g,b);
}

void GfadeOUT(frames)
{
 int i,fr,tmul;
 if(frames<0)return;
 if(frames==0)frames=1;
 for(fr=frames;fr>0;fr--)
 {
  tmul=(64*fr)/frames;
  while(!inp(0x3da)&8);	// Wait for retrace
  while(inp(0x3da)&8);	// Wait for retrace
  for(i=0;i<256;i++)
  {
   _pal(i,
	(Gpalette[i].r*tmul)/64,
	(Gpalette[i].g*tmul)/64,
	(Gpalette[i].b*tmul)/64);
  }
 }
}

void GfadeIN(frames)
{
 int i,fr,tmul;
 if(frames<0)return;
 if(frames==0)frames=1;
 for(fr=0;fr<=frames;fr++)
 {
  tmul=(64*fr)/frames;
  while(!inp(0x3da)&8);	// Wait for retrace
  while(inp(0x3da)&8);	// Wait for retrace
  for(i=0;i<256;i++)
  {
   _pal(i,
	(Gpalette[i].r*tmul)/64,
	(Gpalette[i].g*tmul)/64,
	(Gpalette[i].b*tmul)/64);
  }
 }
}

static void _pal(int c,int r,int g,int b)
{
 outp(0x3C8,c);
 outp(0x3C9,r);
 outp(0x3C9,g);
 outp(0x3C9,b);
}

void Gpset(unsigned x,unsigned y,char c)
{
 if(x<320 && y<200)
  *(Gbuffer+y*320+x)=c;
}

void Gdirectpset(unsigned x,unsigned y,char c)
{
 if(x<320 && y<200)
  *(Gscreen+y*320+x)=c;
}

void Goddfade(int c,int num)
{
 unsigned int a,b;
 char t;
 for(a=0;a<num;a++)
  for(b=0;b<64000;b++)
  {
   t=*(Gbuffer+b);
   if(t<c)*(Gbuffer+b)=1+t;
   if(t>c)*(Gbuffer+b)=t-1;
  }
}

int GloadCOL(char *filename)
{
 FILE *fp;
 int i;
 int r,g,b;
 fp=fopen(filename,"rb");
 if(fp==NULL)return -1;
 for(i=0;i<256;i++)
 {
  r=getc(fp);
  g=getc(fp);
  b=getc(fp);
  if(r==EOF || g==EOF || b==EOF)return -1;
  Gpal(i,r,g,b);
 }
 fclose(fp);
 return 0;
}

char *GloadCEL(char *filename)	/* Loads a .CEL file into memory which it
				allocates and returns to caller */
{
 unsigned size;
 char *sprite;
 struct CELheader header;
 FILE *fp;
 fp=fopen(filename,"rb");
 if(fp==NULL)return NULL;
 fread(&header,sizeof(header),1,fp);
 size=header.width*header.height;
 sprite=malloc(size+4);
 if(sprite==NULL)return sprite;
 *(int *)sprite=header.width;
 *(int *)(sprite+2)=header.height;
 fread(sprite+4,size,1,fp);
 fclose(fp);
 return sprite;
}

void Gputbob(char *bob,unsigned x,unsigned y)
{	// Place graphic in buffer without transparency
 unsigned int yp,w,h;
 char *p;
 if(bob==NULL)return;
 w=*(int *)bob;
 h=*(int *)(bob+2);
 if(x+w > 320 || y+h >200)return;
 bob+=4;		// Changes LOCAL pointer, NOT actual data
 p=Gbuffer+(y*320+x);
 for(yp=0;yp<h;yp++)
 {
  memcpy(p,bob,w);
  bob+=w;
  p+=320;
 }
}

void Gputsprite(char *sprite,int x,int y)
{	// Place graphic in buffer with colour 0 transparent
 int xp,yp,w,h;
 char *p;
 if(sprite==NULL)return;
 w=*(int *)sprite;
 h=*(int *)(sprite+2);
 if(x+(w>>1) > 320 || y+(h>>1) >200 || x<(w>>1) || y<(h>>1) ){Gputspriteclip(sprite,x,y);return;}
 x-=(w>>1);
 y-=(h>>1);
 sprite+=4;		// Changes LOCAL pointer, NOT actual data
 p=Gbuffer+(y*320+x);
 for(yp=0;yp<h;yp++)
 {
  for(xp=0;xp<w;xp++,sprite++)
   if(*sprite) *(p+xp) = *sprite;
  p+=320;
 }
}

void Gputspriteclip(char *sprite,int x,int y)
{	// Place graphic in buffer with colour 0 transparent
	// and with some of it off the edge of the screen
 int xp,yp,w,h;
 char *p;

 if(sprite==NULL)return;
 w=*(int *)sprite;
 h=*(int *)(sprite+2);

 /* Check that some of it is on screen */
 if(x+(w>>1)<0 || x-(w>>1)>320 || y+(h>>1)<0 || y-(h>>1)>200)return;
 x-=(w>>1);
 y-=(h>>1);
 sprite+=4;		// Changes LOCAL pointer, NOT actual data
 p=Gbuffer+(y*320+x);
 if(y<0)sprite+=(-y)*w;
 for(yp=0;yp<h;yp++)
 {
  if((yp+y)>=0 && (yp+y)<200)
  for(xp=0;xp<w;xp++,sprite++)
   if(*sprite && (xp+x)>=0 && (xp+x)<320) *(p+xp) = *sprite;
  p+=320;
 }
}



void Gclearbuffer(int c)
{
 memset(Gbuffer,c,64000);
}

void Gflushbuffer()
{
 int i;
 memcpy(Gscreen,Gbuffer,64000);
 for(i=0;i<320;i++)if(*(zzendpad+i))exit(10);
}


static void _Wswap(int *a,int *b)
{
 unsigned t;
 t=*a;
 *a=*b;
 *b=t;
}

int onscr(int x,int y)
{
 if(x<0 || y<0)return -1;
 if(x>=S_W || y>=S_H)return 1;
 return 0;
}

#define SHFTVL 16

void Gtrngl(int x1,int y1,int x2,int y2,int x3,int y3,int colr)
{
 int x4,y4,ldx,ldy,rdx,rdy,my;
 long lv,rv,li,ri;
 int fx,y,tx;

 y=onscr(x1,y1) + onscr(x2,y2) + onscr(x3,y3);
 if(y==-3 || y==3)return;

 if(y1>y2){_Wswap(&x1,&x2);_Wswap(&y1,&y2);}
 if(y1>y3){_Wswap(&x1,&x3);_Wswap(&y1,&y3);}
 if(y2>y3){_Wswap(&x3,&x2);_Wswap(&y3,&y2);}

/* if(y1==y2)y2++;*/

 ldx=x3-x1;
 ldy=y3-y1;
 rdx=x2-x1;
 rdy=y2-y1;
 y4=y2;

 if(ldy!=0)
   x4=(ldx*rdy)/ldy +x1;
  else
   return;

 if(rdy)
 {
  lv=(long)x1<<SHFTVL;
  rv=(long)x1<<SHFTVL;
  li=((long)ldx<<SHFTVL)/(long)ldy;
  ri=((long)rdx<<SHFTVL)/(long)rdy;
  my=y1*S_W-S_W;
  for(y=0;y<rdy;y++)
  {
   fx=(lv>>SHFTVL);
   tx=(rv>>SHFTVL);
   lv+=li;
   rv+=ri;
   my+=S_W;
   if( (y+y1)<0 )continue;
   if( (y+y1)>=S_H )break;
   if(fx<0)fx=0;
   if(fx>S_W)fx=S_W;
   if(tx<0)tx=0;
   if(tx>S_W)tx=S_W;

   if(fx<tx)
   memset(fx+my+Gbuffer,colr,tx-fx+1);
   else memset(tx+my+Gbuffer-1,colr,fx-tx+2);
  }
 }

 ldx=x4-x3;
 ldy=y4-y3;
 rdx=x2-x3;
 rdy=y2-y3;

 if(ldy && rdy)
 {
  lv=(long)x3<<SHFTVL;
  rv=(long)x3<<SHFTVL;
  li=((long)ldx<<SHFTVL)/(long)ldy;
  ri=((long)rdx<<SHFTVL)/(long)rdy;
  my=y3*S_W+S_W;
  for(y=0;y>=ldy;y--)
  {
   fx=(lv>>SHFTVL);
   tx=(rv>>SHFTVL);
   lv-=li;
   rv-=ri;
   my-=S_W;
   if( (y+y3)<0 )break;
   if( (y+y3)>=S_H )continue;
   if(fx<0)fx=0;
   if(fx>S_W)fx=S_W;
   if(tx<0)tx=0;
   if(tx>=S_W)tx=S_W-1;

   if(fx<tx)
   memset(fx+my+Gbuffer,colr,tx-fx+1);
   else memset(tx+my+Gbuffer-1,colr,fx-tx+2);
  }
 }
}

