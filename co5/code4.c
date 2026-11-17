#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>
typedef struct {int id;char group[4];int days_until_eligible;} Donor;
typedef struct {Donor *data;size_t size,capacity;} DonorList;
int add_donor(DonorList *list,int id,const char *group,int days){if(!list||!group||strlen(group)>3)return 0;if(list->size==list->capacity){size_t nc=list->capacity?list->capacity*2:2;if(nc<list->capacity||nc>SIZE_MAX/sizeof(Donor))return 0;Donor *tmp=realloc(list->data,nc*sizeof(Donor));if(!tmp)return 0;list->data=tmp;list->capacity=nc;}Donor *d=&list->data[list->size++];d->id=id;strcpy(d->group,group);d->days_until_eligible=days;return 1;}
int main(int argc,char **argv){size_t n=argc>1?(size_t)strtoull(argv[1],NULL,10):50000;if(n>1000000){fputs("Limit 1000000\n",stderr);return 2;}DonorList list={0};const char *groups[]={"O+","A+","B-","AB+","O-"};clock_t start=clock();FILE *fp=fopen("report_donors.csv","w");if(!fp){perror("report_donors.csv");return 1;}for(size_t i=0;i<n;i++)fprintf(fp,"%d,%s,%d\n",7001+(int)i,groups[i%5],(int)(i%30));fclose(fp);fp=fopen("report_donors.csv","r");if(!fp){perror("report_donors.csv");return 1;}int id,days;char group[4];while(fscanf(fp,"%d,%3[^,],%d\n",&id,group,&days)==3){if(!add_donor(&list,id,group,days)){fputs("Allocation failed\n",stderr);fclose(fp);free(list.data);return 1;}}fclose(fp);clock_t loaded=clock();size_t eligible=0;for(size_t i=0;i<list.size;i++)if(list.data[i].days_until_eligible>=0&&list.data[i].days_until_eligible<=7)eligible++;printf("loaded=%zu eligible_next_7_days=%zu capacity=%zu allocated_bytes=%zu load_seconds=%.6f total_seconds=%.6f\n",list.size,eligible,list.capacity,list.capacity*sizeof(Donor),(double)(loaded-start)/CLOCKS_PER_SEC,(double)(clock()-start)/CLOCKS_PER_SEC);free(list.data);list.data=NULL;list.size=list.capacity=0;puts("memory released");return 0;}
