#ifdef __wasm__
#define resonfile
#endif
char res_names[][32]={"#title","#path","#width","#height",""};
uint32_t res_pos[]={0,11,16,21,26};
#ifdef resonfile
#include <stdio.h>
#define res_binsize 26
uint8_t *res_bin;
#else
uint8_t res_bin[26]={0x01,0x53,0x75,0x70,0x65,0x72,0x4D,0x42,0x36,0x34,0x00,0x01,0x53,0x4D,0x42,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00};
#endif
uint8_t*res_get(const char*nm,uint32_t*size)
{
 int i;
#ifdef resonfile
 if(res_bin==NULL)
 {
  FILE*res=fopen("resource.bin","rb");
  if(res){
   res_bin=(uint8_t*)malloc(res_binsize);
   fread(res_bin,1,res_binsize,res);
   fclose(res);}
 }
#endif
 for(i=0;*res_names[i];i++)
  if(strcmp(res_names[i],nm)==0)
  {
   if(size) *size=res_pos[i+1]-res_pos[i];
   return &res_bin[res_pos[i]];
  }
 return NULL;
}
