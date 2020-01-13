/*

Build

# Windows MSVC
cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example.c /Feexample.exe

# Windows Clang
clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example.c -o example.exe

# Linux Clang
clang -Wall crossline.c example.c -o example

# GCC(Linux, MinGW, Cygwin, MSYS2)
gcc -Wall crossline.c example.c -o example

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossline.h"

static void completion_hook (char const *buf, crossline_completions_t *pCompletion)
{
	int i;
	static const char *cmd[] = {"insert", "select", "update", "delete", "create", "drop", "show", "describe", "help", "exit", "history", "paging", NULL};

	for (i = 0; NULL != cmd[i]; ++i) {
		if (0 == strncmp(buf, cmd[i], strlen(buf))) {
			crossline_completion_add (pCompletion, cmd[i], NULL);
		}
	}

}

static void pagint_test ()
{
	int i;
	crossline_paging_reset ();
	for (i = 0; i < 256; ++i) {
		printf ("Paging test: %3d\n", i);
		if (crossline_paging_check (sizeof("paging test: ") + 3)) {
			break;
		}
	}
}

int main ()
{
	char buf[1024]="select ";

	crossline_completion_register (completion_hook);
	crossline_history_load ("history.txt");

	// Readline with initail text input
	if (NULL != crossline_readline2 ("Crossline> ", buf, sizeof(buf))) {
		printf ("Read line: \"%s\"\n", buf);
	}
	// Readline loop
	while (NULL != crossline_readline ("Crossline> ", buf, sizeof(buf))) {
		printf ("Read line: \"%s\"\n", buf);
		if (!strcmp (buf, "paging")) {
			pagint_test ();
		}
	}

	crossline_history_save ("history.txt");
	return 0;
}
