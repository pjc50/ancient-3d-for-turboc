#include <glib2.h>
#include "3dscript.h"

void positionobj(char *name,long nx,long ny,long nz)
{
 int i;

 for(i=0;i<num_instances;i++)
  if(stricmp(name,(instance+i)->name)==0)break;
 if(i==num_instances)/*if not fount in list */
 {
  if(stricmp(name,"light")==0)
  {
   light.rx=nx;
   light.ry=ny;
   light.rz=nz;
  }
  return;
 }

 (instance+i)->o.pos.x=nx;
 (instance+i)->o.pos.y=ny;
 (instance+i)->o.pos.z=nz;
}

void rotateobj(char *name,long nx,long ny,long nz)
{
 int i;

 for(i=0;i<num_instances;i++)
  if(stricmp(name,(instance+i)->name)==0)break;
 if(i==num_instances)return;/*if not fount in list */

 (instance+i)->o.pos.rotx=nx;
 (instance+i)->o.pos.roty=ny;
 (instance+i)->o.pos.rotz=nz;
}

void vectorobj(char *name,long nx,long ny,long nz)
{
 int i;

 for(i=0;i<num_instances;i++)
  if(stricmp(name,(instance+i)->name)==0)break;
 if(i==num_instances)return;/*if not fount in list */

 (instance+i)->xv=nx;
 (instance+i)->yv=ny;
 (instance+i)->zv=nz;
}

void spinobj(char *name,long nx,long ny,long nz)
{
 int i;

 for(i=0;i<num_instances;i++)
  if(stricmp(name,(instance+i)->name)==0)break;
 if(i==num_instances)return;/*if not fount in list */

 (instance+i)->xs=nx;
 (instance+i)->ys=ny;
 (instance+i)->zs=nz;
}

void makeinstance(char *typename,char *iname)
{
 int tp;
 int i;

 /* Find the type */
 for(i=0;i<num_objtypes;i++)
  if(stricmp(typename,(objtype+i)->name)==0)break;
 if(i==num_objtypes)return;/*if not fount in list */
 tp=i;
 i=num_instances;

 if(i==0)instance=malloc(sizeof(struct INSTANCE));
  else instance=realloc(instance,(i+1)*sizeof(struct INSTANCE));

 (instance+i)->name=malloc(strlen(iname));
 strcpy( (instance+i)->name,iname);

 (instance+i)->o.startp=(objtype+tp)->startpoly;
 (instance+i)->o.lastp=(objtype+tp)->endpoly;
 (instance+i)->o.pos.x=0;
 (instance+i)->o.pos.y=0;
 (instance+i)->o.pos.z=0;
 (instance+i)->o.pos.rotx=0;
 (instance+i)->o.pos.roty=0;
 (instance+i)->o.pos.rotz=0;

 (instance+i)->xv=0;
 (instance+i)->yv=0;
 (instance+i)->zv=0;
 (instance+i)->xs=0;
 (instance+i)->ys=0;
 (instance+i)->zs=0;

 num_instances++;
}

void redraw()
{
 int i;

#ifndef TEXTONLY
#endif
 for(i=0;i<w.npoints;i++)pnt[i].recalc=-1;

 free(obj);
 obj=malloc(sizeof(s_obj)*num_instances);

 getm(&w.e_mat,&w.e_t);

 for(i=0;i<num_instances;i++)
 {
  *(obj+i)=(instance+i)->o;
  (instance+i)->o.pos.x+=(instance+i)->xv;
  (instance+i)->o.pos.y+=(instance+i)->yv;
  (instance+i)->o.pos.z+=(instance+i)->zv;
  (instance+i)->o.pos.rotx=((instance+i)->o.pos.rotx+(instance+i)->xs)%MAXANGLE;
  (instance+i)->o.pos.roty=((instance+i)->o.pos.roty+(instance+i)->ys)%MAXANGLE;
  (instance+i)->o.pos.rotz=((instance+i)->o.pos.rotz+(instance+i)->zs)%MAXANGLE;

#ifdef TEXTONLY
  printf("(%ld %ld %ld) %d %d   \n",obj->pos.x,obj->pos.y,obj->pos.z,obj->startp,obj->lastp);
#endif
#ifndef TEXTONLY
  doobj(i);
#endif
 }
#ifndef TEXTONLY
 sortpanels();
#endif

#ifndef TEXTONLY
 Gflushbuffer();
#endif
 frames++;
}

void loadobjecttype(char *name,char *typename)
{
 FILE *fp;

 w.nobjs=0;
 free(obj);
 obj=malloc(1);

 if(num_objtypes==0)objtype=malloc(sizeof(struct OBJTYPE));
  else objtype=realloc(objtype,(num_objtypes+1)*sizeof(struct OBJTYPE));

 (objtype+num_objtypes)->name=malloc(strlen(typename));
 strcpy( (objtype+num_objtypes)->name,typename);


 fp=fopen(name,"rt");
 if(fp==NULL){printf("Object not found!");exit(7);}
 loadtextobject(fp);
 fclose(fp);

 (objtype+num_objtypes)->startpoly=obj->startp;
 (objtype+num_objtypes)->endpoly=obj->lastp;

 num_objtypes++;
}
