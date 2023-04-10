#include <cstdio>
#include <cstdlib>
#include <cstring>

#define MAX_ENTRIES 10000

typedef struct {
	char *name;
	char *val;
} entry;

char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *len);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);

int main(int argc, char *argv[])

{
	entry entries[MAX_ENTRIES];
	int x,m=0;
	int cl;
//	printf("Content-type: text/html%c%c",10,10);
	if(strcmp(getenv("REQUEST_METHOD"),"POST"))
	{ printf("This script should be referenced with a METHOD of POST.\n");
		printf("If you don't understand this, see this "); printf("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\"> forms overview</A>.%c",10);
		exit(1);
	} if(strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded"))
	{printf("This script can only be used to decode form results. \n");
		exit(1);
	}
	cl = atoi(getenv("CONTENT_LENGTH"));
	for(x=0;cl && (!feof(stdin));x++)
	{m=x;entries[x].val = fmakeword(stdin,'&',&cl); plustospace(entries[x].val);
		unescape_url(entries[x].val);
		entries[x].name = makeword(entries[x].val,'=');
	}
	printf("<H1>Query Results</H1>");
	printf("You submitted the following name/value pairs:<p>%c",10);
	printf("<ul>%c",10);
	for(x=0; x <= m; x++)
		printf("<li> <code>%s = %s</code>%c",entries[x].name, entries[x].val,10);
	printf("</ul>%c",10);
}

char *makeword(char *line, char stop) {
/* Предназначена для выделения части строки, ограниченной "стоп-символами"*/
	int x = 0,y;
	char *word = (char *) malloc(sizeof(char) * (strlen(line) + 1));
	for(x=0;((line[x]) && (line[x] != stop));x++)
		word[x] = line[x];
	word[x] = '\0';
	if(line[x]) ++x;
	y=0;

	while((line[y++] = line[x++]));
	return word;
}

char *fmakeword(FILE *f, char stop, int *cl) {
/* Предназначена для выделения строки, ограниченной "стоп-символом" stop, из потока f длиной cl.
*/
	int wsize;
	char *word;
	int ll;

	wsize = 102400;
	ll=0;
	word = (char *) malloc(sizeof(char) * (wsize + 1));

	while(1) {
		word[ll] = (char)fgetc(f);
		if(ll==wsize) {
			word[ll+1] = '\0';
			wsize+=102400;
			word = (char *)realloc(word,sizeof(char)*(wsize+1));
		}
		--(*cl);
		if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
			if(word[ll] != stop) ll++;
			word[ll] = '\0';
			return word;
		}
		++ll;
	}
}


char x2c(char *what) {
/* Предназначена для преобразования шестнадцатиричного кода символа в код символа
*/
	char digit;

	digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
	digit *= 16;
	digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
	return(digit);
}

void unescape_url(char *url) {

	int x,y;

	for(x=0,y=0;url[y];++x,++y) {
		if((url[x] = url[y]) == '%') {
			url[x] = x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x] = '\0';
}

void plustospace(char *str) {
/*замена символов "+" на символ "пробел"*/
	int x;

	for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}
