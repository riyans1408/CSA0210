#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int valid_group(const char *s){const char *g[]={"O+","O-","A+","A-","B+","B-","AB+","AB-"};for(int i=0;i<8;i++)if(!strcmp(s,g[i]))return 1;return 0;}
int main(int argc,char **argv){if(argc!=3||strcmp(argv[1],"find")||!valid_group(argv[2])){fprintf(stderr,"Usage: %s find {O+,O-,A+,A-,B+,B-,AB+,AB-}\n",argv[0]);return 2;}char name[16],line[256];printf("Enter donor name: ");if(!fgets(line,sizeof line,stdin)){fputs("Input error\n",stderr);return 1;}if(!strchr(line,'\n')&&!feof(stdin)){fputs("Input line too long\n",stderr);return 1;}line[strcspn(line,"\n")]=0;if(strlen(line)>=sizeof name){fputs("Name too long (max 15)\n",stderr);return 1;}strcpy(name,line);FILE *fp=fopen("donors_missing.txt","r");if(!fp)perror("donors_missing.txt");else fclose(fp);printf("Accepted: %s; group: %s\n",name,argv[2]);return 0;}
