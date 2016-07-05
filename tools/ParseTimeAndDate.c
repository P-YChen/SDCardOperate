/*************************************************************************
    > File Name: ParseTimeAndDate.c
    > Author: shift
    > Mail: open_shift@163.com 
    > Created Time: 2016年07月05日 星期二 16时21分01秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>

//#define DEBUG
#define FLAG_TIME 0x01
#define FLAG_DATE 0x02
#define FLAG_MS   0x04
#define YEAR_BASE 1980U

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef char i8;
typedef short i16;
typedef int i32;

extern i8 *optarg;
extern i32 optind;
extern i32 opterr;

void
print_version ()
{
	printf ("ParseTimeAndDate - %d.%d.%d\n", 1, 0, 0);
}

#ifdef DEBUG
void Usage (int argc, char **argv)
#else
void Usage (void)
#endif
{
#ifdef DEBUG
	int i;
	printf ("argc=%d\n", argc);
	for (i=0; i<argc; i++)
		printf ("argv[%d]=%s\n", i, argv[i]);
#endif
	printf ("ParseTimeAndDate -t [TIME] -d [DATE] -m [MS]\n");
	printf ("[OPTIONS]:\n");
	printf ("-t, parse time\n");
	printf ("-d, parse date\n");
	printf ("-m, parse ms\n");
	printf ("-h, show this\n");
	printf ("-v, show the version\n");
}

void
print_time (u32 time, u32 ms)
{
	printf ("Time:%u:%u:%u\n", (time&0xf800)>>11,
							   (time&0x07e0)>>5,
							   (time&0x1f)*2 + ms*10/1000);
}

void
print_date (u32 date)
{
	printf ("Date:%u-%u-%u\n", ((date&0xfe00)>>9) + YEAR_BASE,
							   (date&0x01e0)>>5,
							   (date&0x1f));
}

int main (int argc, char **argv)
{
	i32 args;
	u32 time = 0;
	u32 date = 0;
	u32 ms = 0;
	u8 flags = 0;

	if (argc < 2){
#ifdef DEBUG
		Usage (argc, argv);
#else
		Usage ();
#endif
		return 1;
	}

	while ((args=getopt(argc, argv, "t:d:m:hv")) != -1) {
		switch (args) {
			case 't':
				sscanf (optarg, "%x", &time);
				flags |= FLAG_TIME;
				break;
			case 'd':
				sscanf (optarg, "%x", &date);
				flags |= FLAG_DATE;
				break;
			case 'm':
				sscanf (optarg, "%x", &ms);
				flags |= FLAG_MS;
				break;
			case 'v':
				print_version ();
				break;
			case 'h':
#ifdef DEBUG
				Usage (argc, argv);
#else
				Usage ();
#endif
				break;
			default:
				printf ("Unknow args '0x%02x'\n", args);
#ifdef DEBUG
				Usage (argc, argv);
#else
				Usage ();
#endif
		}
	}

	if (flags & FLAG_DATE)
		print_date (date);

	if (flags & (FLAG_TIME|FLAG_MS))
		print_time (time, ms);

	return 1;
}
