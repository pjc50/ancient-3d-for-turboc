/* Header file for 3DSCRIPT - by Peter Clay */

#include <3d.h>

//#define TEXTONLY 1

extern struct OBJTYPE
{
 char *name;
 int startpoly,endpoly;
}*objtype;
extern int num_objtypes;

extern struct INSTANCE
{
 s_obj o;		/* object this instance refers to */
 int xv,yv,zv;		/* Velocity */
 int xs,ys,zs;		/* Spin */
 char *name;
}*instance;
extern int num_instances;

extern long frames;

void loadobjecttype(char *,char *);
void redraw(void);
void makeinstance(char *,char *);
void positionobj(char *,long,long,long);

enum DRAWMODE
{normal,picture,fade};