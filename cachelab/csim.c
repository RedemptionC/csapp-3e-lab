#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define instructionlen 20

void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */

int powi(int x,int y)
{
	int t=x;
	while(y-->1)
		x=x*t;
	return x;
}
int is_digit(char c)
{
	return c>='0'&&c<='9';
}
void print_help()
{
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
	exit(1);
}

int main(int argc,char *argv[])
{

	int verbose=0;
	int s=0,E=0,b=0;
	char *filename=NULL;
	int i;
	int hits=0,misses=0,evictions=0;
	// parse cli-args
	for(i=1;i<argc;i++)
	{
		//if it's '-',and next arg exist
		//but if it's -h,
		if(argv[i][0]=='-')
		{
			//first check if it's -h,if so ,print help and exit
			if(argv[i][1]=='h')
			{
				print_help();
			}
			//2nd,check if it's verbose,if so ,set verbose=1,continue
			else if (argv[i][1]=='v')
			{
				verbose=1;
				continue;//next iteration
			}
			
			//3rd,need to caculate the num
			else if(argv[i][1]=='s'||argv[i][1]=='E'||argv[i][1]=='b')
			{
				char opt=argv[i][1];
				if(i+1<argc)
				{
					i++;
				}
				else
				{
					print_help();
				}
				// switch to the num string
				if(is_digit(argv[i][0]))
				{
					int j=0;
					int num=0;
					while(is_digit(argv[i][j])&&argv[i][j]!='\0')
					{
						num=num*10+argv[i][j++]-'0';

					}	
					switch(opt)
					{
						case 's':s=num;break;
						case 'E':E=num;break;
						case 'b':b=num;break;
						default:break;//won't get here
					}
					// caculation done
				}
				else
				{
					print_help();
				}
			}
			else if (argv[i][1]=='t')
			{
				if(i+1>=argc)
					print_help();
				i++;
				filename=argv[i];
			}
			else
			{
				print_help();
			}
			

		}
		else
		{
			continue;
		}
	}
	// argc==1
	if(i==1)
	{
		print_help();
	}
	// printf("s: %d,E: %d,b: %d,verbose: %d\n",s,E,b,verbose);
	// int S=powi(2,s),B=powi(2,b); B unused
	int S=powi(2,s);
	/*
		cli-args processed ,start reading file
	*/
	FILE *fp=fopen(filename,"r");
	if (fp==NULL)
	{
		printf("open file error\n");
		exit(-1);
	}
	/*
		malloc cache matrix,2-d,first 3 cell in a line store 
		valid and tag ,lru cell
	*/
	int ** cache=(int **)malloc(S*E*sizeof(int *));
	for(i=0;i<S*E;i++)
	{
		cache[i]=(int *)malloc((3)*sizeof(int));
		memset(cache[i],0,(3)*sizeof(int));
	}

	char instruction[instructionlen];
	// while (!feof(fp))
	while(fgets(instruction,instructionlen,fp)!=NULL)
	{
		i=0;

		if (instruction[0]!=' ')
		{
			continue;
		}

		if(verbose)
		{
			while(instruction[i]!='\n')
				printf("%c",instruction[i++]);
			printf(" ");
		}
		char op=instruction[1];
		int j=1;
		unsigned int addr=0;
		/*
			notice that the form of the instruction is consistent
			so caculate addr first,then save the size
		*/
		while(instruction[j]!=',')
		{

			if (is_digit(instruction[j]))
			{
				addr=addr*16+instruction[j]-'0';
			}
			if(instruction[j]>='a'&&instruction[j]<='f')
			{
				addr=addr*16+instruction[j]-'a'+10;
			}
			j++;
		}
		// int size=instruction[++j]-'0'; unused

		/*
			extract tag,set,offset(low b bits) from addr
		*/
		unsigned int t=~0;
		t=t>>(sizeof(int)*8-b);// b bits 1
		// int offset=addr&t; unused
		addr=addr>>b;
		/*
			notice:0 is default int,not unsigned,need a u postfix
			cause we need a logical right shift
		*/
		int set=((~0u)>>(sizeof(int)*8-s))&addr;//s bits 1,& addr->set
		int tag=addr>>s;
		int currentline=(set)*E;// set*E->always 1st line in that set
								//only works for E=1 
		// line matching,set*E~(set+1)*E-1
		int match=0,emptyline=set*E,empty=0;//empty=1 means there is empty line
		i=set*E;
		for(;i<(set+1)*E;i++)
		{
			if(cache[i][1]==tag&&cache[i][0]==1)
			{
				currentline=i;
				match=1;
				break;
			}
			if(cache[i][0]==0&&empty==0)
			{
				empty=1;
				emptyline=i;
			}
		}
		if(!match)
		{
			if(empty)
			{
				currentline=emptyline;
			}
			else
			{
				/*
					miss,and no empty line,use lru to replace some line
				*/
				for(i=set*E;i<(set+1)*E;i++)
				{
					if(cache[i][2]>cache[currentline][2])
						{
							currentline=i;
						}
				}
			}
			
		}
		// cache[currentline][2]++;//leads to lfu policy
		for(i=0;i<S*E;i++)
			if(cache[i][0])
				cache[i][2]++;
		cache[currentline][2]=0;
		// printf(" currentline:%d ",currentline);
		if(op=='L')
		{
			// if not valid 
			if(cache[currentline][0]==0)
			{
				misses++;
				if(verbose)
					printf("miss ");
				cache[currentline][0]=1;
				cache[currentline][1]=tag;
			}
			else
			{
				if(cache[currentline][1]==tag)
				{
					if(verbose)
						printf("hit ");
					hits++;
				}
				else
				{
					cache[currentline][1]=tag;
					if(verbose)
						printf("miss eviction ");
					misses++;
					evictions++;
				}
				
			}
			if(verbose)
				printf("\n");
			continue;
		}
		else if (op=='M')
		{
			/*
				modify,first read,then write.write always hit
			*/
			if(cache[currentline][0]==0)
			{
				/*
					read miss,fetch,write hit
				*/
				cache[currentline][0]=1;
				cache[currentline][1]=tag;
				if(verbose)
					printf("miss hit ");
				misses++;
				hits++;
			}
			else
			{
				if(cache[currentline][1]==tag)
				{
					/*
						read hit,don't change anything?(valid,tag)
					*/
					if(verbose)
						printf("hit hit ");
					hits+=2;
				}
				else
				{
					/*
						conflict,
					*/
					cache[currentline][1]=tag;
					if(verbose)
						printf("miss eviction hit ");
					misses++;
					evictions++;
					hits++;
				}
				
			}
			if(verbose)
				printf("\n");
			continue;
			
		}
		else
		{
			/*
				S,save
			*/
			if(cache[currentline][0]==0)
			{
				misses++;
				if(verbose)
					printf("miss ");
				cache[currentline][0]=1;
				cache[currentline][1]=tag;
			}
			else
			{
				if(cache[currentline][1]==tag)
				{
					if(verbose)
						printf("hit ");
					hits++;
				}
				else
				{
					cache[currentline][1]=tag;
					if(verbose)
						printf("miss eviction ");
					misses++;
					evictions++;
				}
				
			}
			if(verbose)	
				printf("\n");
			continue;
		}
		
		
	}
	fclose(fp);

	i=0;
	for(;i<S*E;i++)
		free((void *)cache[i]);
	free((void *)cache);

	printSummary(hits,misses,evictions);
	return 0;
}
