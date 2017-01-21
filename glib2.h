// Header File For Glib 2

#ifdef __cplusplus
extern "C" {
#endif

void Ginit(void);
void Gmode(int);
char *GloadCEL(char *);
int GloadCOL(char *);
void Gputsprite(char *,int,int);
void Gputspriteclip(char *,int,int);

void Gpset(unsigned,unsigned,char);
void Gdirectpset(unsigned,unsigned,char);

void Gpal(int,int,int,int);
void GfadeOUT(int);
void GfadeIN(int);
void Goddfade(int,int);

void Gclearbuffer(int);

#ifdef __cplusplus
}
#endif

struct RGB
{
 char r,g,b;
};
//extern int DEBUG;
extern struct RGB Gpalette[256];