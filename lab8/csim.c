/* ********************************************
 * name: Huang Junsen
 * ID: 5130379024
 ***********************************************/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

//read the address from a string
long get_add(char *c)
{
	long sum = 0;
	for(int i = 0; i < 16; ++i){
		if(isdigit(c[i])){
			sum += ((long)1 << 4 * (15-i)) * (c[i]-'0');
		}
		else{ 
			sum += ((long)1 << 4 * (15-i)) * ((c[i]-'a') + 10);
		}
	}
	return sum;
}

//read the set size from a string
long get_S(char *c,int s, int b)
{
	long sum = get_add(c);
	return sum%((long)1<<(s+b))/(1<<b);
}

//read the tag from a string
long get_tag(char *c, int s, int b)
{
	long sum = get_add(c);
	return sum/((long)1<<(s+b));
}

typedef struct Line
{	
	int valid;
	long tag;
	struct Line *next;
}line;

typedef struct Set
{
	line *line_head;
	struct Set *next;
}set;

typedef struct Cache
{
	long S;
	long B;
	int E;
	set *set_head;

}cache;

//move a line to the head of the set
void moveTohead(set *s, int E,  int index)
{
	line *l;
	if(index == 0)
		return;
	else if(index == E-1){
		int i;
		l = s->line_head;
		for(i = 0; i < E - 1; ++i)
			l = l->next;
		line *tmp;
		tmp = s->line_head;
		s->line_head = l;
		l->next = tmp;
	}
	else{
		int i;
		l = s->line_head;
		for(i = 0; i < index-1; ++i)
			l = l->next;
		line *tmp1 = l->next;
		l->next = l->next->next;
		line *tmp2 = s->line_head;
		s->line_head = tmp1;
		tmp1->next = tmp2;
	}
}

int main(int argc, char *argv[])
{
	int s = 0;
	int E = 0;
	int b = 0;
	char *filename;
	int ch;

	//read the command line
	while((ch = getopt(argc, argv, "s:E:b:t:")) != -1)
	{	
		switch(ch){
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				filename = optarg;
				break;
		}
	}

	/* construct a cache and initial */
	cache *ccache = (cache *)malloc(sizeof(cache));
	ccache->S = (1 << s);
	ccache->E = E;
	ccache->B = (1 << s);

	// the first set of the cache
	set *sset = (set *)malloc(sizeof(set));
	line *lline = (line *)malloc(sizeof(line));
	lline->valid = FALSE;
	ccache->set_head = sset;
	sset->line_head = lline;
	set *stmp;
	line *ltmp;

	// initial lines of the first set
	for (int i = 1; i < E; ++i){
		ltmp = (line *)malloc(sizeof(line));
		ltmp->valid = FALSE;
		lline->next = ltmp;
		lline = lline->next;
	}
	// initial other sets
	for (int i = 1; i < (ccache->S); ++i){
		stmp = (set *)malloc(sizeof(set));
		sset->next = stmp;
		sset = sset->next;

		lline = (line *)malloc(sizeof(line));
		lline->valid = FALSE;
		sset->line_head = lline;
		for (int j = 1; j < E; ++j) {
			ltmp = (line *)malloc(sizeof(line));
			ltmp->valid = FALSE;
			lline->next = ltmp;
			lline = lline->next;
		}
	}
	int hit_count = 0;
    int miss_count = 0;
	int eviction_count = 0;
	FILE* fp = fopen(filename, "r");
	if (fp == NULL) printf("%s: No such file or directory", filename);
	
	char *address = (char *)malloc(16*sizeof(char));
	char input = fgetc(fp);
	// now read the trace file
	while(!feof(fp)) {
		//ignore lines follow with I
		if (input == 'I'){
			while(input != '\n')
				input = fgetc(fp);
			input = fgetc(fp);
		}
		else {
			input = fgetc(fp);     // read instruction
			char ins = input;
			input = fgetc(fp);	   // read a blank;
			input = fgetc(fp);     // read the first char of the address
			int count = 0;
			while(input != ','){
				address[count++] = input;
				input = fgetc(fp);	
			}
			count--;
			while(input != '\n')
				input = fgetc(fp);
			input = fgetc(fp);
			int i;
			for(i = 0; i <= count; ++i){
				address[15-i] = address[count-i];
			}
			for(i = 0; i < 15-count; ++i){
				address[i] = '0';
			}

			long cur_S = get_S(address, s, b);
			set *index_s = ccache->set_head;
			for(i = 0; i < cur_S; ++i){
				index_s = index_s->next;
			}
			line *index_l = index_s->line_head;
			int hit = FALSE;

			// if M, there must be a hit
			if(ins == 'M') hit_count += 1;

			/* every line read must be moved to the head
			 * in this way, the last line is least-recently used
			 */
			for(i = 0; i < ccache->E; ++i){
				if(index_l->valid == TRUE && 
				   index_l->tag == get_tag(address, s, b)){
					hit_count += 1;
					hit = TRUE;
					moveTohead(index_s, E, i);
					break;
				}
				index_l = index_l->next;
			}
			if(hit == FALSE){
				miss_count += 1;
				index_l = index_s->line_head;
				int full = TRUE;
				for(i = 0; i < ccache->E;++i){
					if(index_l->valid == FALSE){
						full = FALSE;
						index_l->valid = TRUE;
						index_l->tag = get_tag(address, s, b);
						moveTohead(index_s, E, i);
						break;
					}
					index_l = index_l->next;
				}
				if(full == TRUE) {
					eviction_count += 1;
					index_l = index_s->line_head;
					for(i = 0; i < ccache->E-1; ++i)
						index_l = index_l->next;
					index_l->tag = get_tag(address, s, b);
					moveTohead(index_s, E, E-1);
				}
			}
		}
	}
	printSummary(hit_count, miss_count, eviction_count);
	sset = ccache->set_head;
	
	/* free */
	for (int i = 0; i < s; ++i){
		lline = sset->line_head;
		for (int j = 0; j < E; ++j){
			ltmp = lline->next;
			free(lline);
			lline = ltmp;
		}
		stmp = sset->next;
		free(sset);
		sset = stmp;
	}
	free(address);
	
    return 0;
}
