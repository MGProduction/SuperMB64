#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../src/stb/stb_image.h"

#define MINILIB_IMPLEMENTATION
#include "../src/mg/minilib.h"
#include "../src/mg/asefile.h"

#define QOI_IMPLEMENTATION
#include "../src/libs/qoi.h"

typedef struct{
 FILE    *f;
 uint8_t *buf;
 uint32_t bufsize,rbufsize;
 uint32_t idx[256];
 uint8_t  cnt;
}resbld;

void resbld_new(resbld*res,const char*resourceh)
{
 if(resourceh&&*resourceh)
  res->f=fopen(resourceh,"wb");
 else
  res->f=NULL;
 res->rbufsize=256*1024;
 res->buf=(uint8_t*)malloc(res->rbufsize);
 res->bufsize=0;res->cnt=0;res->idx[0]=0;

 if(res->f)
  {
   fprintf(res->f,"#ifdef __wasm__\r\n");
   fprintf(res->f,"#define resonfile\r\n");
   fprintf(res->f,"#endif\r\n");

   fprintf(res->f,"char res_names[][32]={");
  }
}

void resbld_delete(resbld*res,const char*resourcebin)
{
 uint32_t i;
 if(res->f)
  {
   fprintf(res->f,",\"\"};\r\n");
     
   fprintf(res->f,"dword res_pos[]={0");
   for(i=1;i<res->cnt+1;i++)
    fprintf(res->f,",%d",res->idx[i]);
   fprintf(res->f,"};\r\n");

   fprintf(res->f,"#ifdef resonfile\r\n");
   fprintf(res->f,"#include <stdio.h>\r\n");
   fprintf(res->f,"#define res_binsize %d\r\n",res->bufsize);
   fprintf(res->f,"byte *res_bin;\r\n");   
   fprintf(res->f,"#else\r\n");  
   fprintf(res->f,"byte res_bin[%d]={",res->bufsize);
   for(i=0;i<res->bufsize;i++)
    {
     if((i%32)==31) fprintf(res->f,"\r\n");
     if(i) fprintf(res->f,",");
     fprintf(res->f,"0x%02X",res->buf[i]);
    }
  }

 if(resourcebin&&*resourcebin)
  {
   FILE*fbin=fopen(resourcebin,"wb");
   if(fbin)
    {
     fwrite(res->buf,1,res->bufsize,fbin);
     fclose(fbin);
    }
  }

 free(res->buf);
 if(res->f)
  {
   fprintf(res->f,"};\r\n");
   fprintf(res->f,"#endif\r\n");
   
   fprintf(res->f,"byte*res_get(const char*nm,dword*size)\r\n");
   fprintf(res->f,"{\r\n");
   fprintf(res->f," int i;\r\n");
   fprintf(res->f,"#ifdef resonfile\r\n");
   fprintf(res->f," if(res_bin==NULL)\r\n");
   fprintf(res->f," {\r\n");
   fprintf(res->f,"  FILE*res=fopen(\"resource.bin\",\"rb\");\r\n");
   fprintf(res->f,"  if(res){\r\n");
   fprintf(res->f,"   res_bin=(byte*)malloc(res_binsize);\r\n");
   fprintf(res->f,"   fread(res_bin,1,res_binsize,res);\r\n");
   fprintf(res->f,"   fclose(res);}\r\n");
   fprintf(res->f," }\r\n");
   fprintf(res->f,"#endif\r\n");
   fprintf(res->f," for(i=0;*res_names[i];i++)\r\n");
   fprintf(res->f,"  if(strcmp(res_names[i],nm)==0)\r\n");
   fprintf(res->f,"  {\r\n");
   fprintf(res->f,"   if(size) *size=res_pos[i+1]-res_pos[i];\r\n");
   fprintf(res->f,"   return &res_bin[res_pos[i]];\r\n");
   fprintf(res->f,"  }\r\n");
   fprintf(res->f," return NULL;\r\n");
   fprintf(res->f,"}\r\n");
   fprintf(res->f,"int res_getvalue(const char*nm,int defvalue)\r\n");
   fprintf(res->f,"{\r\n");
   fprintf(res->f," byte*data=res_get(nm,NULL);\r\n");
   fprintf(res->f," if(data&&(data[0]==0))\r\n");
   fprintf(res->f,"  return *(int*)&data[1];\r\n");
   fprintf(res->f," else\r\n");
   fprintf(res->f,"  return defvalue;\r\n");
   fprintf(res->f,"}\r\n");
   fprintf(res->f,"const char*res_getstring(const char*nm,const char*defvalue)\r\n");
   fprintf(res->f,"{\r\n");
   fprintf(res->f," byte*data=res_get(nm,NULL);\r\n");
   fprintf(res->f," if(data&&(data[0]==1))\r\n");
   fprintf(res->f,"  return (const char*)&data[1];\r\n");
   fprintf(res->f," else\r\n");
   fprintf(res->f,"  return defvalue;\r\n");
   fprintf(res->f,"}\r\n");
   fclose(res->f);
  }
}

int resbld_addinfo(resbld*res,const char*id,const char*value)
{
 uint32_t st_size=0;
 uint8_t  mem[256];
 if(isbetween(*value,'0','9'))
  {mem[st_size++]=0;*(int*)&mem[st_size]=atoi(value);st_size+=sizeof(int);}
 else
  {mem[st_size++]=1;strcpy((char*)&mem[st_size],value);st_size+=strlen(value)+1;}

 if(res->bufsize+st_size>res->rbufsize)
  {
   res->rbufsize=res->bufsize+st_size+64*1024;
   res->buf=(uint8_t*)realloc(res->buf,res->rbufsize);
  }
 memcpy(res->buf+res->bufsize,mem,st_size);
  
 if(res->cnt)
  fprintf(res->f,",");
 fprintf(res->f,"\"%s\"",id);

 res->bufsize+=st_size;
 res->idx[++res->cnt]=res->bufsize;

 return 1;
}

int tilemap2a(_tilemaps*anim,uint16_t*a)
{
 int      i=0,k;
 uint16_t pos=1+anim->ntilemap*(2+2+2+2+1+1);
 a[i++]=anim->ntilemap;
 for(k=0;k<anim->ntilemap;k++)
  {
   a[i++]=anim->tilemap[k].name&0xFFFF;
   a[i++]=anim->tilemap[k].name>>16;
   a[i++]=anim->tilemap[k].mapw*anim->tilemap[k].tilew;
   a[i++]=anim->tilemap[k].maph*anim->tilemap[k].tileh;
   a[i++]=anim->tilemap[k].mapw;
   a[i++]=anim->tilemap[k].maph;
   a[i++]=anim->tilemap[k].tilew;
   a[i++]=anim->tilemap[k].tileh;
   a[i++]=anim->tilemap[k].tilecount;
   a[i++]=pos;
   pos+=anim->tilemap[k].mapw*anim->tilemap[k].maph;
  }
 for(k=0;k<anim->ntilemap;k++)
  {
   int n;
   for(n=0;n<anim->tilemap[k].mapw*anim->tilemap[k].maph;n++)
    a[i++]=anim->tilemap[k].map[n];
  }
 return i;
}

int anim2a(_animation*anim,uint16_t*a)
{
 int i=0,j,k,sum=0;
 a[i++]=anim->naframes;
 a[i++]=0;
 a[i++]=0;
 for(sum=j=0;j<anim->naframes;j++)
 {
  a[i++]=anim->aframes[j].name&0xFFFF;
  a[i++]=anim->aframes[j].name>>16;  
  a[i++]=sum;
  a[i++]=sum+anim->aframes[j].nids-1;  
  a[i++]=anim->aframes[j].flags;
  sum+=anim->aframes[j].nids;
 }
 a[1]=i; // inizio sequenza ids
 // seqquenza di ids
 for(j=0;j<anim->naframes;j++)
  for(k=0;k<anim->aframes[j].nids;k++)
   a[i++]=anim->aframes[j].ids[k];
 a[2]=i; // inizio sequenza di nframes
 for(j=0;j<anim->nframes;j++)
  {
   a[i++]=anim->frames[j].ms;
   a[i++]=anim->frames[j].uv.x;
   a[i++]=anim->frames[j].uv.y;
   a[i++]=anim->frames[j].uv.w;
   a[i++]=anim->frames[j].uv.h;
  }
 return i;
}

void resbld_emit(resbld*res,uint8_t*mem,uint32_t st_size,const char*id,const char*ext)
{
 if(st_size)
  {
   if(res->bufsize+st_size>res->rbufsize)
   {
    res->rbufsize=res->bufsize+st_size+64*1024;
    res->buf=(uint8_t*)realloc(res->buf,res->rbufsize);
   }
   memcpy(res->buf+res->bufsize,mem,st_size);
  }
 if(mem)
  free(mem);
 
 if(st_size)
  {
   if(res->cnt)
    fprintf(res->f,",");
   if(ext&&*ext)
    fprintf(res->f,"\"%s.%s\"",id,ext);
   else
    fprintf(res->f,"\"%s\"",id);

   res->bufsize+=st_size;
   res->idx[++res->cnt]=res->bufsize;
  }
}

int resbld_addfile(resbld*res,const char*id,const char*path,const char*lname)
{
 struct stat filestat;
 char   name[256];
 /*if((strcmp(ext,"ogg")==0)||(strcmp(ext,"mp3")==0))
  sprintf(name,"res/audio/%s.%s",png,ext);
 else
  sprintf(name,"res/%s.%s",png,ext);*/
 sprintf(name,"%s/%s",path,lname);
 if(stat(name, &filestat) == 0)
  {  
   FILE*f=fopen(name,"rb");
   if(f)
    {
     uint32_t st_size=filestat.st_size;
     uint8_t *mem=(uint8_t*)malloc(st_size);
     fread(mem,1,st_size,f);  
     fclose(f);  

     if(string_hasextension(name,"ase"))
      {
       if(memcmp(lname,"sprite_",6)==0)
        {
         qoi_desc  desc;
         uint8_t   *nmem;
         uint8_t   *nmemq;
         int        w,h,nst_size,asize;
         uint16_t*  a=(uint16_t*)malloc(4096);
         _animation*anim;
         nmem=mg_readASE(name,mem,st_size,&w,&h,&anim,NULL,NULL,atlas_freesize);
         memset(&desc,0,sizeof(desc));
         desc.width=w;desc.height=h;desc.channels=4;

         asize=anim2a(anim,a);

         nmemq=(uint8_t*)qoi_encode(nmem,&desc,&nst_size);
         free(nmem);
         free(mem);
         mem=nmemq;st_size=nst_size;
         resbld_emit(res,mem,st_size,id,NULL);     
         
         resbld_emit(res,(uint8_t*)&a[0],asize*sizeof(a),id,"anim");     
        }
       else
       if(memcmp(lname,"tilemap_",7)==0)
        {
         qoi_desc  desc;
         
         uint8_t   *nmemq;
         int        i,w,h,nst_size,asize;
         _tilemaps  tilemap;
         uint16_t*  a=(uint16_t*)malloc(64*1024);
         mg_readASE(name,mem,st_size,&w,&h,NULL,NULL,&tilemap,atlas_freesize);

         asize=tilemap2a(&tilemap,a);
         resbld_emit(res,(uint8_t*)&a[0],asize*sizeof(a),id,NULL);
         
         for(i=0;i<tilemap.ntilemap;i++)
          {
           char r[32];
           memset(&desc,0,sizeof(desc));
           w=tilemap.tilemap[i].tilew;
           h=tilemap.tilemap[i].tileh*tilemap.tilemap[i].tilecount;
           desc.width=w;desc.height=h;desc.channels=4;
           nmemq=(uint8_t*)qoi_encode(tilemap.tilemap[i].tiles,&desc,&nst_size);
           mem=nmemq;st_size=nst_size;
           r[0]='a'+i;r[1]=0;
           resbld_emit(res,mem,st_size,id,r);     
          }

         /*asize=anim2a(anim,a);

         nmemq=(uint8_t*)qoi_encode(nmem,&desc,&nst_size);
         free(nmem);
         free(mem);
         mem=nmemq;st_size=nst_size;
         resbld_emit(res,mem,st_size,id,NULL);     
         
         resbld_emit(res,(uint8_t*)&a[0],asize*sizeof(a),id,"anim");*/
        }
       else
        {
        }
      }
     else
     if(string_hasextension(name,"png"))
      {
       uint8_t *nmem;
       uint8_t *imem;
       int      w,h,c,nst_size;
       qoi_desc desc;
       memset(&desc,0,sizeof(desc));
       imem=stbi_load_from_memory(mem,st_size,&w,&h,&c,NULL);
       desc.width=w;desc.height=h;desc.channels=c;
       nmem=(uint8_t*)qoi_encode(imem,&desc,&nst_size);
       free(imem);
       free(mem);
       mem=nmem;st_size=nst_size;
       resbld_emit(res,mem,st_size,id,NULL);
      }
     else
      resbld_emit(res,mem,st_size,id,NULL);     
        
     return 1;
    }
  }
 else
  printf("#ERR: can't read %s\n",name);
 return 0;
}

int main(int argc,const char*argv[])
{
 if(argc>1)
  {
   FILE*f=fopen(argv[1],"rb");
   if(f)
    {
     resbld res;
     char   path[256]={0};
     resbld_new(&res,argv[3]);
     while(!feof(f))
     {
      char       line[256],key[64];
      const char*value;
      fgets(line,sizeof(line),f);
      string_trim(line,line,1|2);   
      if((*line==0)||(memcmp(line,"//",2)==0))
       ;
      else
       {
        value=string_gettoken(line,key,'=');
        if(*key=='#')
         {
          if(strcmp(key,"#path")==0)
           strcpy(path,value);
          else
           resbld_addinfo(&res,key,value);
         }
        else
         resbld_addfile(&res,key,path,value);         
       }
     }
     resbld_delete(&res,argv[2]);
     fclose(f);
    }
  }
 
 return 0;
}