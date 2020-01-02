/*

This Example implements a simple SQL syntax parser.
	insert into <table> set column1=value1,column2=value2,...
	select <* | column1,columnm2,...> from <table> [where] [order by] [limit] [offset]
	update <table> set column1=value1,column2=value2 [where] [order by] [limit] [offset]
	delete from <table> [where] [order by] [limit] [offset]
	create [unique] index <name> on <table> (column1,column2,...)
	drop {table | index} <name>
	show {tables | databases}
	describe <table>
	help {insert | select | update | delete | create | drop | show | describe | help | exit | history}


Build

# Windows MSVC
cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example_sql.c /Feexample_sql.exe

# Windows Clang
clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example_sql.c -o example_sql.exe

# Linux Clang
clang -Wall crossline.c example_sql.c -o example_sql

# GCC(Linux, MinGW, Cygwin, MSYS2)
gcc -Wall crossline.c example_sql.c -o example_sql

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossline.h"

void sql_add_completion (crossline_completions_t *pCompletion, const char *prefix, const char **match, const char **help)
{
	int i, len = (int)strlen(prefix);
	for (i = 0;  NULL != match[i]; ++i) {
		if (0 == strncmp(prefix, match[i], len)) {
			crossline_completion_add (pCompletion, match[i], help?help[i]:NULL);
		}
	}
}

int sql_find_key (const char **match, const char *prefix)
{
	int i;
	for (i = 0; NULL != match[i]; ++i) {
		if (0 == strcmp(prefix, match[i])) {
		  return i;
		}
	}
	return -1;
}

enum {
	CMD_INSERT,
	CMD_SELECT,
	CMD_UPDATE,
	CMD_DELETE,
	CMD_CREATE,
	CMD_DROP,
	CMD_SHOW,
	CMD_DESCRIBE,
	CMD_HELP,
	CMD_EXIT,
	CMD_HISTORY,
};

void sql_completion_hook (char const *buf, crossline_completions_t *pCompletion)
{
	int	num, cmd, bUnique = 0;
	char split[8][128], last_ch;
	static const char* sql_cmd[] = {"insert", "select", "update", "delete", "create", "drop", "show", "describe", "help", "exit", "history", NULL};
	static const char* sql_cmd_help[] = {
		"Insert a record to table",
		"Select records from table",
		"Update records in table",
		"Delete records from table",
		"Create index on table",
		"Drop index or table",
		"Show tables or databases",
		"Show table schema",
		"Show help for topic",
		"Exit shell",
		"Show history"};
	static const char* sql_caluse[] = {"where", "order by", "limit", "offset", NULL};
	static const char* sql_index[]  = {"unique", "index", NULL};
	static const char* sql_drop[]   = {"table", "index", NULL};
	static const char* sql_show[]   = {"tables", "databases", NULL};

	memset (split, '\0', sizeof (split));
	num = sscanf (buf, "%s %s %s %s %s %s %s %s", split[0], split[1], split[2], split[3], split[4], split[5], split[6], split[7]);
	cmd = sql_find_key (sql_cmd, split[0]);

	if ((cmd < 0) && (num <= 1)) {
		sql_add_completion (pCompletion, split[0], sql_cmd, sql_cmd_help);
	}
	last_ch = buf[strlen(buf) - 1];
	switch (cmd) {
	case CMD_INSERT: // insert into <table> set column1=value1,column2=value2,...
		if ((1 == num) && (' ' == last_ch)) {
			crossline_completion_add (pCompletion, "into", NULL);
		} else if ((2 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "table name");
		} else if ((3 == num) && (' ' == last_ch)) {
			crossline_completion_add (pCompletion, "set", NULL);
		} else if ((4 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "column1=value1,column2=value2,...");
		}
		break;
	case CMD_SELECT: // select < * | column1,columnm2,...> from <table> [where] [order by] [limit] [offset]
		if ((1 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "* | column1,columnm2,...");
		} else if ((2 == num) && (' ' == last_ch)) {
			crossline_completion_add (pCompletion, "from", NULL);
		} else if ((3 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "table name");
		} else if ((4 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_caluse, NULL);
		} else if ((num > 4) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[num-1], sql_caluse, NULL);
		}
		break;
	case CMD_UPDATE: // update <table> set column1=value1,column2=value2 [where] [order by] [limit] [offset]
		if ((1 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "table name");
		} else if ((2 == num) && (' ' == last_ch)) {
			crossline_completion_add (pCompletion, "set", NULL);
		} else if ((3 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "column1=value1,column2=value2,...");
		} else if ((4 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_caluse, NULL);
		} else if ((num > 4) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[num-1], sql_caluse, NULL);
		}
		break;
	case CMD_DELETE: // delete from <table> [where] [order by] [limit] [offset]
		if ((1 == num) && (' ' == last_ch)) {
			crossline_completion_add (pCompletion, "from", NULL);
		} else if ((2 == num) && (' ' == last_ch)) {
			crossline_hints_set (pCompletion, "table name");
		} else if ((3 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_caluse, NULL);
		} else if ((num > 3) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[num-1], sql_caluse, NULL);
		}
		break;
	case CMD_CREATE: // create [unique] index <name> on <table> (column1,column2,...)
		if ((1 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_index, NULL);
		} else if ((2 == num) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[1], sql_index, NULL);
		} else {
			if ((num >= 2) && !strcmp (split[1], "unique")) {
				bUnique = 1;
			}
			if ((2 == num) && bUnique && (' ' == last_ch)) {
				crossline_completion_add (pCompletion, "index", NULL);
			}
			else if ((2+bUnique == num) && (' ' == last_ch)) {
				crossline_hints_set (pCompletion, "index name");
			} else if ((3+bUnique == num) && (' ' == last_ch)) {
				crossline_completion_add (pCompletion, "on", NULL);
			} else if ((4+bUnique == num) && (' ' == last_ch)) {
				crossline_hints_set (pCompletion, "table name");
			} else if ((5+bUnique == num) && (' ' == last_ch)) {
				crossline_hints_set (pCompletion, "(column1,column2,...)");
			}
		}
		break;
	case CMD_DROP:	// drop table <name>, drop index <name>
		if ((1 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_drop, NULL);
		} else if ((2 == num) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[1], sql_drop, NULL);
		} else if ((num == 2) && (' ' == last_ch)) {
			if (!strcmp (split[1], "table")) {
				crossline_hints_set (pCompletion, "table name");
			} else if (!strcmp (split[1], "index")) {
				crossline_hints_set (pCompletion, "index name");
			}
		}
		break;
	case CMD_SHOW: // show tables, show databases
		if ((1 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_show, NULL);
		} else if ((2 == num) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[1], sql_show, NULL);
		}
		break;
	case CMD_DESCRIBE: // describe <table>
		if (' ' == last_ch) {
			crossline_hints_set (pCompletion, "table name");
		}
		break;
	case CMD_HELP:
		if ((1 == num) && (' ' == last_ch)) {
			sql_add_completion (pCompletion, "", sql_cmd, NULL);
		} else if ((2 == num) && (' ' != last_ch)) {
			sql_add_completion (pCompletion, split[1], sql_cmd, NULL);
		}
		break;
	case CMD_EXIT:
		break;
	}
}

int main ()
{
	char buf[256];

	crossline_completion_register (sql_completion_hook);
	crossline_history_load ("history.txt");

	while (NULL != crossline_readline ("SQL> ", buf, sizeof(buf))) {
		printf ("Read line: \"%s\"\n", buf);
		if (!strcmp (buf, "history")) {
			crossline_history_show ();
		} else if (!strcmp (buf, "exit")) {
			break;
		}
	}

	crossline_history_save ("history.txt");
	return 0;
}
