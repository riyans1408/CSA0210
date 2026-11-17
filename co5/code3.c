#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
typedef struct {int group_code;int units;} Stock;
typedef struct {int id;char name[32];char group[4];int age;char phone[16];char date[11];} Donor;
int update_units(FILE *fp,int index,int units){Stock s;if(index<0||fseek(fp,(long)index*sizeof s,SEEK_SET)!=0)return 0;if(fread(&s,sizeof s,1,fp)!=1)return 0;s.units=units;if(fseek(fp,-(long)sizeof s,SEEK_CUR)!=0)return 0;return fwrite(&s,sizeof s,1,fp)==1&&fflush(fp)==0;}
static void donor(Donor *d,int i){memset(d,0,sizeof *d);d->id=100000+i;snprintf(d->name,sizeof d->name,"Donor%06d",i);strcpy(d->group,i%2?"O+":"AB-");d->age=18+i%43;strcpy(d->phone,"9000000000");strcpy(d->date,"2026-01-01");}
static long size(const char *f){struct stat s;return stat(f,&s)==0?(long)s.st_size:-1;}
static int benchmark(int n){FILE *t=fopen("donors_text.csv","w"),*b=fopen("donors_binary.dat","wb");if(!t||!b){perror("open write");return 0;}Donor d;clock_t a=clock();for(int i=0;i<n;i++){donor(&d,i);fprintf(t,"%d,%s,%s,%d,%s,%s\n",d.id,d.name,d.group,d.age,d.phone,d.date);}fclose(t);double tw=(double)(clock()-a)/CLOCKS_PER_SEC;a=clock();for(int i=0;i<n;i++){donor(&d,i);fwrite(&d,sizeof d,1,b);}fclose(b);double bw=(double)(clock()-a)/CLOCKS_PER_SEC;int count=0;char line[256];t=fopen("donors_text.csv","r");a=clock();while(fgets(line,sizeof line,t))count++;fclose(t);double tr=(double)(clock()-a)/CLOCKS_PER_SEC;b=fopen("donors_binary.dat","rb");a=clock();int bc=0;while(fread(&d,sizeof d,1,b)==1)bc++;fclose(b);double br=(double)(clock()-a)/CLOCKS_PER_SEC;printf("n=%d text=%ldB binary=%ldB write=%.6f/%.6f read=%.6f/%.6f counts=%d/%d\n",n,size("donors_text.csv"),size("donors_binary.dat"),tw,bw,tr,br,count,bc);return count==n&&bc==n;}
int main(void){FILE *fp=tmpfile();if(!fp){perror("tmpfile");return 1;}Stock s[5]={{1,40},{2,25},{3,12},{4,8},{5,30}};if(fwrite(s,sizeof *s,5,fp)!=5)return 1;printf("update record 3: %s\n",update_units(fp,3,20)?"done":"failed");printf("update record 9: %s\n",update_units(fp,9,20)?"done":"failed");rewind(fp);Stock v;while(fread(&v,sizeof v,1,fp)==1)printf("%d:%d ",v.group_code,v.units);puts("");fclose(fp);for(int i=0;i<4;i++){int n=(int[]){10,1000,10000,100000}[i];if(!benchmark(n))return 1;}return 0;}
