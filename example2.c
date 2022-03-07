/*

Build

# Windows MSVC
cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example2.c /Feexample2.exe

# Windows Clang
clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example2.c -o example2.exe

# Linux Clang
clang -Wall crossline.c example2.c -o example2

# GCC(Linux, MinGW, Cygwin, MSYS2)
gcc -Wall crossline.c example2.c -o example2

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossline.h"

#ifdef _WIN32
	#define strcasecmp				_stricmp
	#define strncasecmp				_strnicmp
#endif

static void completion_hook (char const *buf, crossline_completions_t *pCompletion)
{
	int i;
	crossline_color_e wcolor, hcolor;
	static const char *cmd[] = {"INSERT", "SELECT", "UPDATE", "DELETE", "CREATE", "DROP", "SHOW", "DESCRIBE", "help", "exit", "history", "paging", "color", NULL};
	static const char* cmd_help[] = {
		"Insert a record to table ",
		"Select records from table",
		"Update records in table  ",
		"Delete records from table",
		"Create index on table    ",
		"Drop index or table      ",
		"Show tables or databases ",
		"Show table schema        ",
		"Show help for topic      ",
		"Exit shell               ",
		"Show history             ",
		"Do paing APIs test       ",
		"Do Color APIs test       "};

	for (i = 0; NULL != cmd[i]; ++i) {
		if (0 == strncasecmp(buf, cmd[i], strlen(buf))) {
			if (i < 8) { 
				wcolor = CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_YELLOW; 
			} else { 
				wcolor = CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_CYAN; 
			}
			hcolor = i%2 ? CROSSLINE_FGCOLOR_WHITE : CROSSLINE_FGCOLOR_CYAN;
			crossline_completion_add_color (pCompletion, cmd[i], wcolor, cmd_help[i], hcolor);
		}
	}
}

static void pagint_test ()
{
	int i;
	crossline_paging_set (1);
	for (i = 0; i < 256; ++i) {
		printf ("Paging test: %3d\n", i);
		if (crossline_paging_check (sizeof("paging test: ") + 3)) {
			break;
		}
	}
}

static void color_test ()
{
	printf ("\n*** Color test *** \n");
	printf ("  Default Foregroud and Backgroud\n\n");

	crossline_color_set (CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_WHITE);
	printf ("  Foregroud: Black");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_UNDERLINE | CROSSLINE_FGCOLOR_RED);
	printf ("  Foregroud: Red Underline\n");
	crossline_color_set (CROSSLINE_FGCOLOR_GREEN);
	printf ("  Foregroud: Green\n");
	crossline_color_set (CROSSLINE_FGCOLOR_YELLOW);
	printf ("  Foregroud: Yellow\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BLUE);
	printf ("  Foregroud: Blue\n");
	crossline_color_set (CROSSLINE_FGCOLOR_MAGENTA);
	printf ("  Foregroud: Magenta\n");
	crossline_color_set (CROSSLINE_FGCOLOR_CYAN);
	printf ("  Foregroud: Cyan\n");
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BLACK);
	printf ("  Foregroud: White");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n\n");	

	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_WHITE);
	printf ("  Foregroud: Bright Black");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_RED);
	printf ("  Foregroud: Bright Red\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_GREEN);
	printf ("  Foregroud: Bright Green\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_YELLOW);
	printf ("  Foregroud: Bright Yellow\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLUE);
	printf ("  Foregroud: Bright Blue\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_MAGENTA);
	printf ("  Foregroud: Bright Magenta\n");
	crossline_color_set (CROSSLINE_UNDERLINE | CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_CYAN);
	printf ("  Foregroud: Bright Cyan Underline\n");
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BLACK);
	printf ("  Foregroud: Bright White\n\n");

	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BLACK);
	printf ("  Backgroud: Black   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_RED);
	printf ("  Backgroud: Red     ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_GREEN);
	printf ("  Backgroud: Green   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_YELLOW);
	printf ("  Backgroud: Yellow  ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BLUE);
	printf ("  Backgroud: Blue    ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_MAGENTA);
	printf ("  Backgroud: Magenta ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_CYAN);
	printf ("  Backgroud: Cyan    ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_WHITE);
	printf ("  Backgroud: White   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n\n");	

	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_BLACK);
	printf ("  Backgroud: Bright Black   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_RED);
	printf ("  Backgroud: Bright Red     ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_GREEN);
	printf ("  Backgroud: Bright Green   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_YELLOW);
	printf ("  Backgroud: Bright Yellow  ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_BLUE);
	printf ("  Backgroud: Bright Blue    ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_WHITE | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_MAGENTA);
	printf ("  Backgroud: Bright Magenta ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_CYAN);
	printf ("  Backgroud: Bright Cyan    ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
	crossline_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_BLACK | CROSSLINE_BGCOLOR_BRIGHT | CROSSLINE_BGCOLOR_WHITE);
	printf ("  Backgroud: Bright White   ");
	crossline_color_set (CROSSLINE_COLOR_DEFAULT);
	printf ("\n");	
}

int main ()
{
	char buf[1024]="select ";

	crossline_completion_register (completion_hook);
	crossline_history_load ("history.txt");
	crossline_prompt_color_set (CROSSLINE_FGCOLOR_BRIGHT | CROSSLINE_FGCOLOR_GREEN);

	// Readline with initail text input
	if (NULL != crossline_readline2 ("Crossline> ", buf, sizeof(buf))) {
		printf ("Read line: \"%s\"\n", buf);
	}
	// Readline loop
	while (NULL != crossline_readline ("Crossline> ", buf, sizeof(buf))) {
		printf ("Read line: \"%s\"\n", buf);

		if (!strcmp (buf, "history")) {
			crossline_history_show ();
		}

		if (!strcmp (buf, "paging")) {
			pagint_test ();
		}
		if (!strcmp (buf, "color")) {
			color_test ();
		}
	}

	crossline_history_save ("history.txt");
	return 0;
}
