#include <bios.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
//#include <expire.h>
#include <alloc.h>
#include "3dscript.h"
#include <string.h>
#include <glib2.h>

#define DAYSTOEXPIRE 20

char *overlaypic;
char *script,*scriptpos;
long frames;
time_t stoptime,starttime;

struct OBJTYPE *objtype;
int num_objtypes;

struct INSTANCE *instance;
int num_instances;

void init(void);
void loadscript(char *);
void getnextcmd(char *);

int nextcommand()
{
 char cmd[100],tmp[100];/* Buffer for current command */
 int i;

 /* Get command from script */
 getnextcmd(cmd);

#ifdef TEXTONLY
 printf("\nProcessing:\"%s\"",cmd);
#endif

 /****** Command : END ********/
 if(stricmp(cmd,"end")==0)return 0;
 /****** Command : LOAD ********/
 if(stricmp(cmd,"load")==0)
 {
  getnextcmd(cmd);/* filename */
#ifdef TEXTONLY
  printf(" Loading object:\"%s\"",cmd);
#endif
  getnextcmd(tmp);/* typename */
  loadobjecttype(cmd,tmp);
  return 1;
 }
 /****** Command : INSTANCE ********/
 if(stricmp(cmd,"instance")==0)
 {
  getnextcmd(cmd);
  getnextcmd(tmp);
  makeinstance(cmd,tmp);
  return 1;
 }
 /****** Command : DISPLAY ********/
 if(stricmp(cmd,"display")==0)
 {
  getnextcmd(tmp);
  sscanf(tmp,"%d",&i);
  while(i-- && !kbhit())
  {
   if(overlaypic!=NULL)Gputbob(overlaypic,0,0);
	else Gclearbuffer(0);
   redraw();
  }
  return 1;
 }

 /****** Command : POSITION ********/
 if(stricmp(cmd,"position")==0)
 {
  long a,b,c;
  getnextcmd(cmd);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&a);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&b);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&c);
  positionobj(cmd,a,b,c);
  return 1;
 }
 /****** Command : ROTATE ********/
 if(stricmp(cmd,"rotate")==0)
 {
  long a,b,c;
  getnextcmd(cmd);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&a);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&b);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&c);
  rotateobj(cmd,a,b,c);
  return 1;
 }
 /****** Command : VECTOR ********/
 if(stricmp(cmd,"vector")==0)
 {
  long a,b,c;
  getnextcmd(cmd);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&a);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&b);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&c);
  vectorobj(cmd,a,b,c);
  return 1;
 }
 /****** Command : SPIN ********/
 if(stricmp(cmd,"spin")==0)
 {
  long a,b,c;
  getnextcmd(cmd);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&a);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&b);
  getnextcmd(tmp);
  sscanf(tmp,"%ld",&c);
  spinobj(cmd,a,b,c);
  return 1;
 }
 /****** Command : LABEL ********/
 if(stricmp(cmd,"label")==0)
 {
  getnextcmd(cmd);/* simply ignore next word */
  return 1;
 }
 /****** Command : GOTO ********/
 if(stricmp(cmd,"goto")==0)
 {
  getnextcmd(cmd);
  scriptpos=strstr(script,cmd);/* find label */
  if(scriptpos==NULL)
   scriptpos=script;
   else
   getnextcmd(tmp);/* and move to after label */
  return 1;
 }

 printf("Invalid Command! --- Press enter\n");
 getchar();
 return 0;
}

int main(int argc,char *argv[])
{
 printf("3DSCRIPT v0.1 by Peter Clay\n");

/* if(DayExpire(DAYSTOEXPIRE))
 {
  printf("This executable has now time - expired. Please consult the author\n"
	"for the latest version of this program.\n"
	"This version was compiled on %s and expires after %d days.\n"
	__DATE__,DAYSTOEXPIRE);
  exit(1);
 }*/

 if(argc!=2){printf("Syntax: %s <script file>\n");exit(2);}

 loadscript(argv[1]);

 printf("Free memory:%ld bytes\n",farcoreleft());
 if(farcoreleft()<400000){printf("Insufficient memory.\n");exit(3);}

 overlaypic=GloadCEL("BLAKMARB.CEL");

#ifndef TEXTONLY
 init();
#endif

 frames=0;
 time(&starttime);

 light.rz=-2000;

 do
 {
#ifdef TEXTONLY
 printf("%d types, %d instances\n",num_objtypes,num_instances);
#endif
 if(kbhit())break;
 }
 while(nextcommand());

 time(&stoptime);
#ifndef TEXTONLY
 Gmode(0x03);
 Gshutdown();
 close3d();
#endif
 free(script);
 free(overlaypic);

 while(kbhit())getch();
 printf("This demo managed %ld frames in %ld seconds on %d polygons.\n",frames,(long)stoptime-starttime,num_panels);

 printf("This version was compiled on %s and expires after %d days.\n",
	__DATE__,DAYSTOEXPIRE);
 return 0;
}

void init()
{
 init3d();
 Gmode(0x13);
 Ginit();
 GloadCOL("3DPAL.COL");
 w.e_t.z=-1000;
}

void loadscript(char *name)
{
 FILE *fp;
 long l;
 int incomment;
 fp=fopen(name,"rb");

 if(fp==NULL){printf("File %s not found.\n",name);exit(4);}

 /* Allocate memory and load file */
 fseek(fp,0,SEEK_END);
 script=malloc(l=ftell(fp));
 if(script==NULL){printf("Stupid script file.\n");exit(5);}
 fseek(fp,0,SEEK_SET);
 fread(script,1,l-1,fp);/* load it as one block for simplicity */
 scriptpos=script;
 *(script+l-1)='\0';
 fclose(fp);

 /* Wipe out comments in script */
 incomment=0;
 do
 {
  if(incomment)
   if(*scriptpos=='\n')incomment=0;
   else *scriptpos=' ';
  else
   if(*scriptpos=='#'){incomment=1;*scriptpos=' ';}
  scriptpos++;
 }
 while(*scriptpos);
 scriptpos=script;

#ifdef TEXTONLY
 puts(script);
#endif
}

void getnextcmd(char *buf)
{
 int i;
 while(isspace(*scriptpos))scriptpos++;
 i=sscanf(scriptpos,"%s",buf);
 if(i==0){printf("Missing end!");exit(6);}
 while(!isspace(*scriptpos))scriptpos++;
}

