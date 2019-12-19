/* crossline.c -- Version 1.0
 *
 * Crossline is a small, self-contained, zero-config, MIT licensed, 
 *   cross-platform, readline and libedit replacement.
 *
 * Press <F1> to get full shortcuts list.
 *
 * You can find the latest source code and description at:
 *
 *   https://github.com/JunchuanWang80/crossline
 *
 * ------------------------------------------------------------------------
 *
 * MIT License
 * 
 * Copyright (c) 2019, JC Wang (wang_junchuan@163.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
	#include <io.h>
	#include <conio.h>
	#include <windows.h>
  #ifndef STDIN_FILENO
	#define STDIN_FILENO 			_fileno(stdin)
  #endif
	#define isatty					_isatty
	#define strcasecmp				_stricmp
#else
	#include <unistd.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <signal.h>
	#include <sys/ioctl.h>
#endif

#include "crossline.h"

/*****************************************************************************/

// Default word delimiters for move and cut
#define CROSS_DFT_DELIMITER			" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

#define CROSS_HISTORY_MAX_LINE		256		// Maximum history line number
#define CROSS_HISTORY_BUF_LEN		1024	// History line length
#define CROSS_HIS_MATCH_PAT_NUM		16		// History search pattern number

#define CROSS_COMPLET_MAX_LINE		256		// Maximum completion word number
#define CROSS_COMPLET_WORD_LEN		64		// Completion word length
#define CROSS_COMPLET_HELP_LEN		128		// Completion word's help length
#define CROSS_COMPLET_HINT_LEN		128		// Completion syntax hints length

// Make control-characters readable
#define CTRL_KEY(key)				(key - 0x40)
// Build special key code for escape sequences
#define ALT_KEY(key)				(key + ((KEY_ESC+1)<<8))
#define ESC_KEY3(ch)				((KEY_ESC<<8) + ch)
#define ESC_KEY4(ch1,ch2)			((KEY_ESC<<8) + ((ch1)<<16) + ch2)
#define ESC_KEY6(ch1,ch2,ch3)		((KEY_ESC<<8) + ((ch1)<<16) + ((ch2)<<24) + ch3)
#define ESC_OKEY(ch)				((KEY_ESC<<8) + ('O'<<16) + ch)

/*****************************************************************************/

enum {
	KEY_TAB			= 9,	// Autocomplete.
	KEY_BACKSPACE	= 8,	// Delete character before cursor.
	KEY_ENTER		= 13,	// Accept line. (Linux)
	KEY_ENTER2		= 10,	// Accept line. (Windows)
	KEY_ESC			= 27,	// Escapce
	KEY_DEL2		= 127,  // It's treaded as Backspace is Linux
	KEY_DEBUG		= 30,	// Ctrl-^ Enter keyboard debug mode

#ifdef _WIN32 // Windows

	KEY_INSERT		= (KEY_ESC<<8) + 'R', // Paste last cut text.
	KEY_DEL			= (KEY_ESC<<8) + 'S', // Delete character under cursor.
	KEY_HOME		= (KEY_ESC<<8) + 'G', // Move cursor to start of line.
	KEY_END			= (KEY_ESC<<8) + 'O', // Move cursor to end of line.
	KEY_PGUP		= (KEY_ESC<<8) + 'I', // Move to first line in history.
	KEY_PGDN		= (KEY_ESC<<8) + 'Q', // Move to end of input history.
	KEY_UP			= (KEY_ESC<<8) + 'H', // Fetch previous line in history.
	KEY_DOWN		= (KEY_ESC<<8) + 'P', // Fetch next line in history.
	KEY_LEFT		= (KEY_ESC<<8) + 'K', // Move back a character.
	KEY_RIGHT		= (KEY_ESC<<8) + 'M', // Move forward a character.

	KEY_CTRL_DEL	= (KEY_ESC<<8) + 147, // Cut word following cursor.
	KEY_CTRL_HOME	= (KEY_ESC<<8) + 'w', // Cut from start of line to cursor.
	KEY_CTRL_END	= (KEY_ESC<<8) + 'u', // Cut from cursor to end of line.
	KEY_CTRL_UP		= (KEY_ESC<<8) + 141, // Uppercase current or following word.
	KEY_CTRL_DOWN	= (KEY_ESC<<8) + 145, // Lowercase current or following word.
	KEY_CTRL_LEFT	= (KEY_ESC<<8) + 's', // Move back a word.
	KEY_CTRL_RIGHT	= (KEY_ESC<<8) + 't', // Move forward a word.
	KEY_CTRL_BACKSPACE	= (KEY_ESC<<8) + 127, // Cut from start of line to cursor.

	KEY_ALT_DEL		= ALT_KEY(163),		// Cut word following cursor.
	KEY_ALT_HOME	= ALT_KEY(151), 	// Cut from start of line to cursor.
	KEY_ALT_END		= ALT_KEY(159), 	// Cut from cursor to end of line.
	KEY_ALT_UP		= ALT_KEY(152),		// Uppercase current or following word.
	KEY_ALT_DOWN	= ALT_KEY(160),		// Lowercase current or following word.
	KEY_ALT_LEFT	= ALT_KEY(155),		// Move back a word.
	KEY_ALT_RIGHT	= ALT_KEY(157),		// Move forward a word.
	KEY_ALT_BACKSPACE	= ALT_KEY(KEY_BACKSPACE), // Cut from start of line to cursor.

	KEY_F1			= (KEY_ESC<<8) + ';',	// Show help.
	KEY_F2			= (KEY_ESC<<8) + '<',	// Show history.
	KEY_F3			= (KEY_ESC<<8) + '=',	// Clear history (need confirm).
	KEY_F4			= (KEY_ESC<<8) + '>',	// Search history with current input.

#else // Linux

	KEY_INSERT		= ESC_KEY4('2','~'),	// vt100 Esc[2~: Paste last cut text.
	KEY_DEL			= ESC_KEY4('3','~'),	// vt100 Esc[3~: Delete character under cursor.
	KEY_HOME		= ESC_KEY4('1','~'),	// vt100 Esc[1~: Move cursor to start of line.
	KEY_END			= ESC_KEY4('4','~'),	// vt100 Esc[4~: Move cursor to end of line.
	KEY_PGUP		= ESC_KEY4('5','~'),	// vt100 Esc[5~: Move to first line in history.
	KEY_PGDN		= ESC_KEY4('6','~'),	// vt100 Esc[6~: Move to end of input history.
	KEY_UP			= ESC_KEY3('A'), 		//       Esc[A: Fetch previous line in history.
	KEY_DOWN		= ESC_KEY3('B'),		//       Esc[B: Fetch next line in history.
	KEY_LEFT		= ESC_KEY3('D'), 		//       Esc[D: Move back a character.
	KEY_RIGHT		= ESC_KEY3('C'), 		//       Esc[C: Move forward a character.
	KEY_HOME2		= ESC_KEY3('H'),		// xterm Esc[H: Move cursor to start of line.
	KEY_END2		= ESC_KEY3('F'),		// xterm Esc[F: Move cursor to end of line.

	KEY_CTRL_DEL	= ESC_KEY6('3','5','~'), // xterm Esc[3;5~: Cut word following cursor.
	KEY_CTRL_HOME	= ESC_KEY6('1','5','H'), // xterm Esc[1;5H: Cut from start of line to cursor.
	KEY_CTRL_END	= ESC_KEY6('1','5','F'), // xterm Esc[1;5F: Cut from cursor to end of line.
	KEY_CTRL_UP		= ESC_KEY6('1','5','A'), // xterm Esc[1;5A: Uppercase current or following word.
	KEY_CTRL_DOWN	= ESC_KEY6('1','5','B'), // xterm Esc[1;5B: Lowercase current or following word.
	KEY_CTRL_LEFT	= ESC_KEY6('1','5','D'), // xterm Esc[1;5D: Move back a word.
	KEY_CTRL_RIGHT	= ESC_KEY6('1','5','C'), // xterm Esc[1;5C: Move forward a word.
	KEY_CTRL_BACKSPACE	= 31, 				 // xterm Cut from start of line to cursor.
	KEY_CTRL_UP2	= ESC_OKEY('A'),		 // vt100 EscOA: Uppercase current or following word.
	KEY_CTRL_DOWN2	= ESC_OKEY('B'), 		 // vt100 EscOB: Lowercase current or following word.
	KEY_CTRL_LEFT2	= ESC_OKEY('D'),		 // vt100 EscOD: Move back a word.
	KEY_CTRL_RIGHT2	= ESC_OKEY('C'),		 // vt100 EscOC: Move forward a word.

	KEY_ALT_DEL		= ESC_KEY6('3','3','~'), // xterm Esc[3;3~: Cut word following cursor.
	KEY_ALT_HOME	= ESC_KEY6('1','3','H'), // xterm Esc[1;3H: Cut from start of line to cursor.
	KEY_ALT_END		= ESC_KEY6('1','3','F'), // xterm Esc[1;3F: Cut from cursor to end of line.
	KEY_ALT_UP		= ESC_KEY6('1','3','A'), // xterm Esc[1;3A: Uppercase current or following word.
	KEY_ALT_DOWN	= ESC_KEY6('1','3','B'), // xterm Esc[1;3B: Lowercase current or following word.
	KEY_ALT_LEFT	= ESC_KEY6('1','3','D'), // xterm Esc[1;3D: Move back a word.
	KEY_ALT_RIGHT	= ESC_KEY6('1','3','C'), // xterm Esc[1;3C: Move forward a word.
	KEY_ALT_BACKSPACE	= ALT_KEY(KEY_DEL2), // Cut from start of line to cursor.

	KEY_F1			= ESC_OKEY('P'),		 // 	  EscOP: Show help.
	KEY_F2			= ESC_OKEY('Q'),		 // 	  EscOQ: Show history.
	KEY_F3			= ESC_OKEY('R'),		 //       EscOP: Clear history (need confirm).
	KEY_F4			= ESC_OKEY('S'),		 //       EscOP: Search history with current input.

	KEY_F1_2		= ESC_KEY4('[', 'A'),	 // linux Esc[[A: Show help.
	KEY_F2_2		= ESC_KEY4('[', 'B'),	 // linux Esc[[B: Show history.
	KEY_F3_2		= ESC_KEY4('[', 'C'),	 // linux Esc[[C: Clear history (need confirm).
	KEY_F4_2		= ESC_KEY4('[', 'D'),	 // linux Esc[[D: Search history with current input.

#endif
};

/*****************************************************************************/

typedef struct crossline_completions_t {
	int		num;
	char	word[CROSS_COMPLET_MAX_LINE][CROSS_COMPLET_WORD_LEN];
	char	help[CROSS_COMPLET_MAX_LINE][CROSS_COMPLET_HELP_LEN];
	char	hints[CROSS_COMPLET_HINT_LEN];
} crossline_completions_t;

static char		s_word_delimiter[64] = CROSS_DFT_DELIMITER;
static char 	s_history_buf[CROSS_HISTORY_MAX_LINE][CROSS_HISTORY_BUF_LEN];
static uint32_t s_history_id = 0; // Increase always, wrap until UINT_MAX
static char 	s_clip_buf[CROSS_HISTORY_BUF_LEN]; // Buf to store cut text
static crossline_completion_callback s_completion_callback = NULL;

static char* 	crossline_readline_input (char *buf, int size, const char *prompt, int has_input, int in_his);
static int		crossline_history_dump (FILE *file, int print_id, char *patterns, int sel_id, int paging);
static int 		crossline_getch (void); // Read a character from keyboard

#define isdelim(ch)		(NULL != strchr(s_word_delimiter, ch))	// Check ch is word delimiter

/*****************************************************************************/

static char* s_crossline_help[] = {
" Misc Commands",
" +-------------------------+--------------------------------------------------+",
" | F1                      |  Show edit shortcuts help.                       |",
" | Ctrl-^                  |  Enter keyboard debugging mode.                  |",
" +-------------------------+--------------------------------------------------+",
" Move Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-B, Left            |  Move back a character.                          |",
" | Ctrl-F, Right           |  Move forward a character.                       |",
" | Alt-B, ESC+Left,        |  Move back a word.                               |",
" |    Ctrl-Left, Alt-Left  |  (Ctrl-Left, Alt-Left only support Windows/Xterm)|",
" | Alt-F, ESC+Right,       |  Move forward a word.                            |",
" |   Ctrl-Right, Alt-Right | (Ctrl-Right,Alt-Right only support Windows/Xterm)|",
" | Ctrl-A, Home            |  Move cursor to start of line.                   |",
" | Ctrl-E, End             |  Move cursor to end of line.                     |",
" | Ctrl-L                  |  Clear screen and redisplay line.                |",
" +-------------------------+--------------------------------------------------+",
" Edit Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-H, Backspace       |  Delete character before cursor.                 |",
" | Ctrl-D, DEL             |  Delete character under cursor.                  |",
" | Alt-U,  ESC+Up,         |  Uppercase current or following word.            |",
" |   Ctrl-Up,  Alt-Up      |  (Ctrl-Up, Alt-Up only supports Windows/Xterm)   |",
" | Alt-L,  ESC+Down,       |  Lowercase current or following word.            |",
" |   Ctrl-Down, Alt-Down   |  (Ctrl-Down, Alt-Down only support Windows/Xterm)|",
" | Alt-C                   |  Capitalize current or following word.           |",
" | Alt-\\                   |  Delete whitespace around cursor.                |",
" | Ctrl-T                  |  Transpose character.                            |",
" +-------------------------+--------------------------------------------------+",
" Cut&Paste Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-K, ESC+End,        |  Cut from cursor to end of line.                 |",
" |   Ctrl-End, Alt-End     |  (Ctrl-End, Alt-End only support Windows/Xterm)  |",
" | Ctrl-U, ESC+Home,       |  Cut from start of line to cursor.               |",
" |   Ctrl-Home, Alt-Home   |  (Ctrl-Home, Alt-Home only support Windows/Xterm)|",
" | Ctrl-X                  |  Cut whole line.                                 |",
" | Alt-Backspace,          |  Cut word to left of cursor.                     |",
" |    Esc+Backspace,       |                                                  |",
" |    Clt-Backspace        |  (Clt-Backspace only supports Windows/Xterm)     |",
" | Alt-D, ESC+Del,         |  Cut word following cursor.                      |",
" |    Alt-Del, Ctrl-Del    |  (Alt-Del,Ctrl-Del only support Windows/Xterm)   |",
" | Ctrl-W                  |  Cut to left till whitespace (not word).         |",
" | Ctrl-Y, Ctrl-V, Insert  |  Paste last cut text.                            |",
" +-------------------------+--------------------------------------------------+",
" Complete Commands",
" +-------------------------+--------------------------------------------------+",
" | TAB, Ctrl-I             |  Autocomplete.                                   |",
" | Alt-=, Alt-?            |  List possible completions.                      |",
" +-------------------------+--------------------------------------------------+",
" History Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-P, Up              |  Fetch previous line in history.                 |",
" | Ctrl-N, Down            |  Fetch next line in history.                     |",
" | Alt-<,  PgUp            |  Move to first line in history.                  |",
" | Alt->,  PgDn            |  Move to end of input history.                   |",
" | Ctrl-R, Ctrl-S          |  Search history.                                 |",
" | F4                      |  Search history with current input.              |",
" | F1                      |  Show search help when in search mode.           |",
" | F2                      |  Show history.                                   |",
" | F3                      |  Clear history (need confirm).                   |",
" +-------------------------+--------------------------------------------------+",
" Control Commands",
" +-------------------------+--------------------------------------------------+",
" | Enter,  Ctrl-J, Ctrl-M  |  EOL and accept line.                            |",
" | Ctrl-C, Ctrl-G          |  EOF and abort line.                             |",
" | Ctrl-D                  |  EOF if line is empty.                           |",
" | Alt-R					|  Revert line.                                    |",
" | Ctrl-Z                  |  Suspend Job. (Linux Only, fg will resume edit)  |",
" +-------------------------+--------------------------------------------------+",
" Note: If Alt-key doesn't work, an alternate way is to press ESC first then press key, see above ESC+Key.",
NULL};

static char* s_search_help[] = {
"Patterns are separated by ' ', patter match is case insensitive:",
"    select:   choose line including 'select'",
"    -select:  choose line excluding 'select'",
"    \"select from\":  choose line including \"select from\"",
"    -\"select from\": choose line excluding \"select from\"",
"Example:",
"    \"select from\" where -\"order by\" -limit:  ",
"         choose line including \"select from\" and 'where'",
"         and excluding \"order by\" or 'limit'",
NULL};

/*****************************************************************************/

// Main API to read a line, return buf if get line, return NULL if EOF.
char* crossline_readline (char *buf, int size, const char *prompt)
{
	int not_support = 0, len;

	if ((NULL == buf) || (size <= 1))
		{ return NULL; }
	buf[0] = '\0';
	if (!isatty(STDIN_FILENO)) {  // input is not from a terminal
		not_support = 1;
	} else {
		char *term = getenv("TERM");
		if (NULL != term) {
			if (!strcasecmp(term, "dumb") || !strcasecmp(term, "cons25") ||  !strcasecmp(term, "emacs"))
				{ not_support = 1; }
		}
	}
	if (not_support) {
        if (NULL == fgets(buf, size, stdin))
			{ return NULL; }
        for (len = (int)strlen(buf); (len > 0) && (('\n'==buf[len-1]) || ('\r'==buf[len-1])); --len)
			{ buf[len-1] = '\0'; }
        return buf;
	}

	return crossline_readline_input (buf, size, prompt, 0, 0);
}

// Set move/cut word delimiter, defaut is all not digital and alphabetic characters.
void  crossline_delimiter_set (const char *delim)
{
	if (NULL != delim) {
		strncpy (s_word_delimiter, delim, sizeof(s_word_delimiter) - 1);
		s_word_delimiter[sizeof(s_word_delimiter) - 1] = '\0';
	}
}

void crossline_history_show (void)
{
	crossline_history_dump (stdout, 1, NULL, 0, isatty(STDIN_FILENO));
}

void  crossline_history_clear (void)
{
	memset (s_history_buf, 0, sizeof (s_history_buf));
	s_history_id = 0;
}

int crossline_history_save (const char *filename)
{
	if (NULL == filename) {
		return -1;
	} else {
	#ifdef _WIN32
		FILE *file = fopen(filename, "wt");
	#else
		int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
		FILE *file = fdopen(fd, "wt");
	#endif
		if (file == NULL) {	return -1;	}
		crossline_history_dump (file, 0, NULL, 0, 0);
		fclose(file);
	}
	return 0;
}

int crossline_history_load (const char* filename)
{
	int		len;
	char	buf[CROSS_HISTORY_BUF_LEN];
	FILE	*file;

	if (NULL == filename)	{	return -1; }
	file = fopen(filename, "rt");
	if (NULL == file)	{ return -1; }
	while (NULL != fgets(buf, CROSS_HISTORY_BUF_LEN, file)) {
        for (len = (int)strlen(buf); (len > 0) && (('\n'==buf[len-1]) || ('\r'==buf[len-1])); --len)
			{ buf[len-1] = '\0'; }
		if (len > 0) {
			buf[CROSS_HISTORY_BUF_LEN-1] = '\0';
			strcpy (s_history_buf[(s_history_id++) % CROSS_HISTORY_MAX_LINE], buf);
		}
	}
	fclose(file);
	return 0;
}

// Register completion callback.
void crossline_completion_register (crossline_completion_callback pCbFunc)
{
	s_completion_callback = pCbFunc;
}

// Add completion in callback. Word is must, help for word is optional.
void crossline_completion_add (crossline_completions_t *pCompletions, const char *word, const char *help)
{
	if ((NULL != pCompletions) && (NULL != word) && (pCompletions->num < CROSS_COMPLET_MAX_LINE)) {
		strncpy (pCompletions->word[pCompletions->num], word, CROSS_COMPLET_WORD_LEN - 1);
		pCompletions->word[pCompletions->num][CROSS_COMPLET_WORD_LEN - 1] = '\0';
		pCompletions->help[pCompletions->num][0] = '\0';
		if (NULL != help) {
			strncpy (pCompletions->help[pCompletions->num], help, CROSS_COMPLET_HELP_LEN - 1);
			pCompletions->help[pCompletions->num][CROSS_COMPLET_HELP_LEN - 1] = '\0';
		}
		pCompletions->num++;
	}
}

// Set syntax hints in callback.
void crossline_hints_set (crossline_completions_t *pCompletions, const char *hints)
{
	if ((NULL != pCompletions) && (NULL != hints)) {
		strncpy (pCompletions->hints, hints, CROSS_COMPLET_HINT_LEN - 1);
		pCompletions->hints[CROSS_COMPLET_HINT_LEN - 1] = '\0';
	}
}

/*****************************************************************************/

static void crossline_screen_get (int *rows, int *cols)
{
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO inf;
	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	*cols = inf.srWindow.Right - inf.srWindow.Left + 1;
	*rows = inf.srWindow.Bottom - inf.srWindow.Top + 1;
#else
	struct winsize ws = {};
	ioctl (1, TIOCGWINSZ, &ws);
	*cols = ws.ws_col;
	*rows = ws.ws_row;
#endif
	*cols = *cols > 1 ? *cols : 80;
	*rows = *rows > 1 ? *rows : 24;
}

// Paging control for print.
static int crossline_print_paging (int *print_line, int line_len)
{
	char *paging_hints = "*** Press <Space> or <Enter> to continue . . .";
	int	i, ch, rows, cols, len = (int)strlen(paging_hints);

	crossline_screen_get (&rows, &cols);
	*print_line = *print_line + (line_len + cols - 1) / cols;
	if (*print_line >= (rows - 1)) {
		printf ("%s", paging_hints);
		ch = crossline_getch();
		// clear paging hints
		for (i = 0; i < len; ++i) { printf ("\b"); }
		for (i = 0; i < len; ++i) { printf (" ");  }
		for (i = 0; i < len; ++i) { printf ("\b"); }
		if ((' ' != ch) && (KEY_ENTER != ch) && (KEY_ENTER2 != ch))
			{ return 1; }
		*print_line = 0;
	}
	return 0;
}

static void crossline_show_help (int show_search)
{
	int	print_line = 0, i;
	char **help = show_search ? s_search_help : s_crossline_help;
 	printf (" \b\n");
	for (i = 0; NULL != help[i]; ++i) {
		printf ("%s\n", help[i]);
		if (crossline_print_paging (&print_line, (int)strlen(help[i])))
			{ break; }
	}
}

static void str_to_lower (char *str)
{
	for (; '\0' != *str; ++str)
		{ *str = (char)tolower (*str); }
}

// Match including(no prefix) and excluding(with prefix: '-') patterns.
static int crossline_match_patterns (const char *str, char *word[], int num)
{
	int i;
	char buf[CROSS_HISTORY_BUF_LEN];

	strncpy (buf, str, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	str_to_lower (buf);
	for (i = 0; i < num; ++i) {
		if ('-' == word[i][0]) {
			if (NULL != strstr (buf, &word[i][1]))
				{ return 0; }
		} else if (NULL == strstr (buf, word[i]))
			{ return 0; }
	}
	return 1;
}

// Split pattern string to individual pattern list, handle composite words embraced with " ".
static int crossline_split_patterns (char *patterns, char *pat_list[], int max)
{
	int i, num = 0;
	char *pch = patterns;

	if (NULL == patterns) { return 0; }
	while (' ' == *pch)	{ ++pch; }
	while ((num < max) && (NULL != pch)) {
		if (('"' == *pch) || (('-' == *pch) && ('"' == *(pch+1)))) {
			if ('"' != *pch)	{ *(pch+1) = '-'; }
			pat_list[num++] = ++pch;
			if (NULL != (pch = strchr(pch, '"'))) {
				*pch++ = '\0';
				while (' ' == *pch)	{ ++pch; }
			}
		} else {
			pat_list[num++] = pch;
			if (NULL != (pch = strchr (pch, ' '))) {
				*pch = '\0';
				while (' ' == *(++pch))	;
			}
		}
	}
	for (i = 0; i < num; ++i)
		{ str_to_lower (pat_list[i]); }
	return num;
}

// If patterns is not NULL, will filter history.
// If sel_id > 0, return the real id+1 in history buf, else return history number dumped.
static int crossline_history_dump (FILE *file, int print_id, char *patterns, int sel_id, int paging)
{
	uint32_t i;
	int		print_line = 0, id = 0, num=0;
	char	*pat_list[CROSS_HIS_MATCH_PAT_NUM], *history;

	num = crossline_split_patterns (patterns, pat_list, CROSS_HIS_MATCH_PAT_NUM);
	for (i = s_history_id; i < s_history_id + CROSS_HISTORY_MAX_LINE; ++i) {
		history = s_history_buf[i % CROSS_HISTORY_MAX_LINE];
		if ('\0' != history[0]) {
			if ((NULL != patterns) && !crossline_match_patterns (history, pat_list, num))
				{ continue; }
			if (sel_id > 0) {
				if (++id == sel_id) 
					{ return (i % CROSS_HISTORY_MAX_LINE) + 1; }
				continue;
			}
			if (print_id)	{ fprintf (file, "%4d  %s\n", ++id, history); }
			else			{ fprintf (file, "%s\n", history); }
			if (paging) {
				if (crossline_print_paging (&print_line, (int)strlen(history)+(print_id?6:0)))
					{ break; }
			}
		}
	}
	return id;
}

// Search history, input will be initial search patterns.
static int crossline_history_search (char *input)
{
	uint32_t his_id = 0, count;
	char pattern[CROSS_HISTORY_BUF_LEN], buf[8] = "1";

	printf (" \b\n");
	if (NULL != input) {
		strncpy (pattern, input, sizeof(pattern) - 1);
		pattern[sizeof(pattern) - 1] = '\0';
	}
	// Get search patterns
	if (NULL == crossline_readline_input(pattern, sizeof (pattern), "Input Patterns <F1> help: ", (NULL!=input), 1))
		{ return 0; }
	strncpy (s_clip_buf, pattern, sizeof(s_clip_buf) - 1);
	s_clip_buf[sizeof(s_clip_buf) - 1] = '\0';
	count = crossline_history_dump (stdout, 1, pattern, 0, 1);
	if (0 == count)	{ return 0; } // Nothing found, just return
	// Get choice
	if (NULL == crossline_readline_input (buf, sizeof (buf), "Input history id: ", (1==count), 1))
		{ return 0; }
	his_id = atoi (buf);
	if (('\0' != buf[0]) && ((his_id > count) || (his_id <= 0))) { 
		printf ("Invalid history id: %s\n", buf);
		return 0; 
	}
	return crossline_history_dump (stdout, 1, pattern, his_id, 0);
}

// Show completions returned by callback.
static int crossline_show_completions (crossline_completions_t *pCompletions)
{
	int i, j, ret = 0, print_line = 0, word_len = 0, with_help = 0, rows, cols, word_num;

	if (('\0' != pCompletions->hints[0]) || (pCompletions->num > 0)) {
		printf (" \b\n");
		ret = 1;
	}
	// Print syntax hints.
	if ('\0' != pCompletions->hints[0])
		{ printf ("Please input: %s\n", pCompletions->hints); }
	if (0 == pCompletions->num)	{ return ret; }
	for (i = 0; i < pCompletions->num; ++i) {
		if ((int)strlen(pCompletions->word[i]) > word_len)
			{ word_len = (int)strlen(pCompletions->word[i]); }
		if ('\0' != pCompletions->help[i][0])	{ with_help = 1; }
	}
	if (with_help) {
		// Print words with help format.
		for (i = 0; i < pCompletions->num; ++i) {
			printf ("%s", pCompletions->word[i]);
			for (j = 0; j < 4+word_len-(int)strlen(pCompletions->word[i]); ++j)
				{ printf (" "); }
			printf ("%s\n", pCompletions->help[i]);
			if (crossline_print_paging(&print_line, (int)strlen(pCompletions->help[i])+4+word_len))
				{ break; }
		}
		return ret;
	}

	// Print words list in multiple columns.
	crossline_screen_get (&rows, &cols);
	word_num = (cols + 4) / (word_len + 4);
	word_num = word_num > 0 ? word_num : 1;
	for (i = 0; i < pCompletions->num; ++i) {
		if ((i > 0) && (0 == (i % word_num))) {
			printf ("\n");
			if (crossline_print_paging(&print_line, word_len))
				{ return ret; }
		}
		printf ("%s", pCompletions->word[i]);
		for (j = 0; j < 2+word_len-(int)strlen(pCompletions->word[i]); ++j)
			{ printf (" "); }
	}

	printf ("\n");
	return ret;
}

// Refreash current print line and move cursor to new_pos.
static void crossline_refreash (char *buf, int *pCurPos, int *pCurNum, int new_pos, int new_num)
{
	int i = *pCurPos;
	buf[new_num] = '\0';
	for (i = *pCurPos; i > 0; --i)	{ printf ("\b"); }	// move cursor to beginning
	printf ("%s", buf);
	i = *pCurNum - new_num;
	for (i = *pCurNum - new_num; i > 0; --i)	{ printf (" "); } // clear previous extra print
	i = ((*pCurNum > new_num) ? *pCurNum : new_num) - new_pos;
	for (; i > 0; --i)	{ printf ("\b"); } // move cursor to new_pos
	*pCurPos = new_pos;
	*pCurNum = new_num;
}

// Copy part text[cut_beg, cut_end] from src to dest
static void crossline_text_copy (char *dest, const char *src, int cut_beg, int cut_end)
{
	int len = cut_end - cut_beg;
	len = (len < CROSS_HISTORY_BUF_LEN) ? len : (CROSS_HISTORY_BUF_LEN - 1);
	if (len > 0) {
		memcpy (dest, &src[cut_beg], len);
		dest[len] = '\0';
	}
}

// Copy from history buffer to dest
static void crossline_history_copy (char *dest, int size, int *pos, int *num, int history_id)
{
	strncpy (dest, s_history_buf[history_id % CROSS_HISTORY_MAX_LINE], size - 1);
	dest[size - 1] = '\0';
	crossline_refreash (dest, pos, num, (int)strlen(dest), (int)strlen(dest));
}

/*****************************************************************************/

// Convert ESC+Key to Alt-Key
static int crossline_key_esc2alt (int ch)
{
	switch (ch) {
	case KEY_DEL:	ch = KEY_ALT_DEL;	break;
	case KEY_HOME:	ch = KEY_ALT_HOME;	break;
	case KEY_END:	ch = KEY_ALT_END;	break;
	case KEY_UP:	ch = KEY_ALT_UP;	break;
	case KEY_DOWN:	ch = KEY_ALT_DOWN;	break;
	case KEY_LEFT:	ch = KEY_ALT_LEFT;	break;
	case KEY_RIGHT:	ch = KEY_ALT_RIGHT;	break;
	case KEY_BACKSPACE:	ch = KEY_ALT_BACKSPACE;	break;
	}
	return ch;
}

// Map other function keys to main key
static int crossline_key_mapping (int ch)
{
	switch (ch) {
#ifndef _WIN32
	case KEY_HOME2:			ch = KEY_HOME;			break;
	case KEY_END2:			ch = KEY_END;			break;
	case KEY_CTRL_UP2:		ch = KEY_CTRL_UP;		break;
	case KEY_CTRL_DOWN2:	ch = KEY_CTRL_DOWN;		break;
	case KEY_CTRL_LEFT2:	ch = KEY_CTRL_LEFT;		break;
	case KEY_CTRL_RIGHT2:	ch = KEY_CTRL_RIGHT;	break;
	case KEY_F1_2:			ch = KEY_F1;			break;
	case KEY_F2_2:			ch = KEY_F2;			break;
	case KEY_F3_2:			ch = KEY_F3;			break;
	case KEY_F4_2:			ch = KEY_F4;			break;
#endif
	case KEY_DEL2:			ch = KEY_BACKSPACE;		break;
	}
	return ch;
}

#ifdef _WIN32	// Windows
// Read a character from keyboard.
static int crossline_getch (void)
{
	fflush (stdout);
	return _getch();
}

// Read a KEY from keyboard, is_esc indicats whether it's a function key.
static int crossline_getkey (int *is_esc)
{
	int ch = crossline_getch (), esc;
	if ((GetKeyState (VK_CONTROL) & 0x8000) && (KEY_DEL2 == ch)) {
		ch = KEY_CTRL_BACKSPACE;
	} else if ((224 == ch) || (0 == ch)) {
		*is_esc = 1;
		ch = crossline_getch ();
		ch = (GetKeyState (VK_MENU) & 0x8000) ? ALT_KEY(ch) : ch + (KEY_ESC<<8);
	} else if (KEY_ESC == ch) { // Handle ESC+Key
		ch = crossline_getkey (&esc);
		ch = crossline_key_esc2alt (ch);
	} else if (GetKeyState (VK_MENU) & 0x8000)
		{ ch = ALT_KEY(ch); }
	return ch;
}

#else // Linux

// Read a character from keyboard
static int crossline_getch ()
{
	char ch = 0;
	struct termios old_term, cur_term;
	fflush (stdout);
	if (tcgetattr(0, &old_term) < 0)	{ perror("tcsetattr()"); }
	cur_term = old_term;
	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
	cur_term.c_cc[VMIN] = 1;
	cur_term.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr ICANON"); }
	if (read(0, &ch, 1) < 0)	{ perror("read()"); }
	if (tcsetattr(0, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr ~ICANON"); }
	return ch;
}

// Convert escape sequences to internal special function key
static int crossline_get_esckey (int ch)
{
	int ch2;
	if (0 == ch)	{ ch = crossline_getch (); }
	if ('[' == ch) {
		ch = crossline_getch ();
		if ((ch>='0') && (ch<='6')) {
			ch2 = crossline_getch ();
			if ('~' == ch2)	{ ch = ESC_KEY4 (ch, ch2); } // ex. Esc[4~
			else if (';' == ch2) {
				ch2 = crossline_getch();
				if (('5' != ch2) && ('3' != ch2))
					{ return 0; }
				ch = ESC_KEY6 (ch, ch2, crossline_getch()); // ex. Esc[1;5B
			}
		} else if ('[' == ch) {
			ch = ESC_KEY4 ('[', crossline_getch());	// ex. Esc[[A
		} else { ch = ESC_KEY3 (ch); }	// ex. Esc[A
	} else if ('O' == ch) {
		ch = ESC_OKEY (crossline_getch());	// ex. EscOP
	} else { ch = ALT_KEY (ch); } // ex. Alt+Backspace
	return ch;
}

// Read a KEY from keyboard, is_esc indicats whether it's a function key.
static int crossline_getkey (int *is_esc)
{
	int ch = crossline_getch();
	if (KEY_ESC == ch) {
		*is_esc = 1;
		ch = crossline_getch ();
		if (KEY_ESC == ch) { // Handle ESC+Key
			ch = crossline_get_esckey (0);
			ch = crossline_key_mapping (ch);
			ch = crossline_key_esc2alt (ch);
		} else { ch = crossline_get_esckey (ch); }
	}
	return ch;
}
#endif

/*****************************************************************************/

/* Internal readline from terminal
 * has_input indicates buf has inital input.
 * in_his will disable history and complete shortcuts
 */
static char* crossline_readline_input (char *buf, int size, const char *prompt, int has_input, int in_his)
{
	int		pos = 0, num = 0, read_end = 0, is_esc;
	int		ch, len, new_pos, copy_buf = 0;
	uint32_t history_id = s_history_id, search_his;
	char	input[CROSS_HISTORY_BUF_LEN];
	crossline_completions_t		completions;

	prompt = (NULL != prompt) ? prompt : "";
	if (has_input) {
		num = pos = (int)strlen (buf);
		crossline_text_copy (input, buf, pos, num);
	} else
		{ buf[0] = input[0] = '\0'; }
	printf ("%s%s", prompt, buf);

	do {
		is_esc = 0;
		ch = crossline_getkey (&is_esc);
		ch = crossline_key_mapping (ch);

		switch (ch) {
/* Misc Commands */
		case KEY_F1:	// Show help
			crossline_show_help (in_his);
			printf ("%s%s", prompt, buf);
			new_pos = pos; pos = num;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case KEY_DEBUG:	// Enter keyboard debug mode
			printf(" \b\nEnter keyboard debug mode, <Ctrl-C> to exit debug\n");
			while (CTRL_KEY('C') != (ch=crossline_getch()))
				{ printf ("%3d 0x%02x (%c)\n", ch, ch, isprint(ch) ? ch : ' '); }
			printf ("%s%s", prompt, buf);
			new_pos = pos; pos = num;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

/* Move Commands */
		case KEY_LEFT:	// Move back a character.
		case CTRL_KEY('B'):
			if (pos > 0) 
				{ crossline_refreash (buf, &pos, &num, pos-1, num); }
			break;

		case KEY_RIGHT:	// Move forward a character.
		case CTRL_KEY('F'):
			if (pos < num)
				{ crossline_refreash (buf, &pos, &num, pos+1, num); }
			break;

		case ALT_KEY('b'):	// Move back a word.
		case ALT_KEY('B'):
		case KEY_CTRL_LEFT:
		case KEY_ALT_LEFT:
			for (new_pos=pos-1; (new_pos > 0) && isdelim(buf[new_pos]); --new_pos)	;
			for (; (new_pos > 0) && !isdelim(buf[new_pos]); --new_pos)	;
			crossline_refreash (buf, &pos, &num, new_pos?new_pos+1:new_pos, num);
			break;

		case ALT_KEY('f'):	 // Move forward a word.
		case ALT_KEY('F'):
		case KEY_CTRL_RIGHT:
		case KEY_ALT_RIGHT:
			for (new_pos=pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)	;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case CTRL_KEY('A'):	// Move cursor to start of line.
		case KEY_HOME:
			crossline_refreash (buf, &pos, &num, 0, num);
			break;

		case CTRL_KEY('E'):	// Move cursor to end of line
		case KEY_END:
			crossline_refreash (buf, &pos, &num, num, num);
			break;

		case CTRL_KEY('L'):	// Clear screen and redisplay line
#ifdef _WIN32
			system ("cls");
#else
			system ("clear");
#endif
			printf ("%s%s", prompt, buf);
			new_pos = pos; pos = num;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

/* Edit Commands */
		case KEY_BACKSPACE: // Delete char to left of cursor (same with CTRL_KEY('H'))
			if (pos > 0) {
				memmove (&buf[pos-1], &buf[pos], num - pos);
				crossline_refreash (buf, &pos, &num, pos-1, num-1);
			}
			break;

		case KEY_DEL:	// Delete character under cursor
		case CTRL_KEY('D'):
			if (pos < num) {
				memmove (&buf[pos], &buf[pos+1], num - pos - 1);
				crossline_refreash (buf, &pos, &num, pos, num - 1);
			} else if ((0 == num) && (ch == CTRL_KEY('D'))) // On an empty line, EOF
				 { printf (" \b\n"); fflush(stdout); return NULL; }
			break;

		case ALT_KEY('u'):	// Uppercase current or following word.
		case ALT_KEY('U'):
		case KEY_CTRL_UP:
		case KEY_ALT_UP:
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)
				{ buf[new_pos] = (char)toupper (buf[new_pos]); }
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case ALT_KEY('l'):	// Lowercase current or following word.
		case ALT_KEY('L'):
		case KEY_ALT_DOWN:
		case KEY_CTRL_DOWN:
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)
				{ buf[new_pos] = (char)tolower (buf[new_pos]); }
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case ALT_KEY('c'):	// Capitalize current or following word.
		case ALT_KEY('C'):
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			if (new_pos<num)
				{ buf[new_pos] = (char)toupper (buf[new_pos]); }
			for (; new_pos<num && !isdelim(buf[new_pos]); ++new_pos)	;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case ALT_KEY('\\'): // Delete whitespace around cursor.
			for (new_pos = pos; (new_pos > 0) && (' ' == buf[new_pos]); --new_pos)	;
			memmove (&buf[new_pos], &buf[pos], num - pos);
			crossline_refreash (buf, &pos, &num, new_pos, num - (pos-new_pos));
			for (new_pos = pos; (new_pos < num) && (' ' == buf[new_pos]); ++new_pos)	;
			memmove (&buf[pos], &buf[new_pos], num - new_pos);
			crossline_refreash (buf, &pos, &num, pos, num - (new_pos-pos));
			break;

		case CTRL_KEY('T'): // Transpose previous character with current character.
			if ((pos > 0) && !isdelim(buf[pos]) && !isdelim(buf[pos-1])) {
				ch = buf[pos];
				buf[pos] = buf[pos-1];
				buf[pos-1] = (char)ch;
				crossline_refreash (buf, &pos, &num, pos<num?pos+1:pos, num);
			} else if ((pos > 1) && !isdelim(buf[pos-1]) && !isdelim(buf[pos-2])) {
				ch = buf[pos-1];
				buf[pos-1] = buf[pos-2];
				buf[pos-2] = (char)ch;
				crossline_refreash (buf, &pos, &num, pos, num);
			}
			break;

/* Cut&Paste Commands */
		case CTRL_KEY('K'): // Cut from cursor to end of line.
		case KEY_CTRL_END:
		case KEY_ALT_END:
			crossline_text_copy (s_clip_buf, buf, pos, num);
			crossline_refreash (buf, &pos, &num, pos, pos);
			break;

		case CTRL_KEY('U'): // Cut from start of line to cursor.
		case KEY_CTRL_HOME:
		case KEY_ALT_HOME:
			crossline_text_copy (s_clip_buf, buf, 0, pos);
			memmove (&buf[0], &buf[pos], num-pos);
			crossline_refreash (buf, &pos, &num, 0, num - pos);
			break;

		case CTRL_KEY('X'):	// Cut whole line.
			crossline_text_copy (s_clip_buf, buf, 0, num);
			// fall through
		case ALT_KEY('r'):	// Revert line
		case ALT_KEY('R'):
			crossline_refreash (buf, &pos, &num, 0, 0);
			break;

		case CTRL_KEY('W'): // Cut whitespace (not word) to left of cursor.
		case KEY_ALT_BACKSPACE: // Cut word to left of cursor.
		case KEY_CTRL_BACKSPACE:
			new_pos = pos;
			if ((new_pos > 1) && (' ' == buf[new_pos-1]))	{ --new_pos; }
			for (; (new_pos > 0) && isdelim(buf[new_pos]); --new_pos)	;
			if (CTRL_KEY('W') == ch) {
				for (; (new_pos > 0) && (' ' != buf[new_pos]); --new_pos)	;
			} else {
				for (; (new_pos > 0) && !isdelim(buf[new_pos]); --new_pos)	;
			}
			crossline_text_copy (s_clip_buf, buf, new_pos, pos);
			memmove (&buf[new_pos], &buf[pos], num - pos);
			crossline_refreash (buf, &pos, &num, new_pos, num - (pos-new_pos));
			break;

		case ALT_KEY('d'): // Cut word following cursor.
		case ALT_KEY('D'):
		case KEY_ALT_DEL:
		case KEY_CTRL_DEL:
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)	;
			crossline_text_copy (s_clip_buf, buf, pos, new_pos);
			memmove (&buf[pos], &buf[new_pos], num - new_pos);
			crossline_refreash (buf, &pos, &num, pos, num - (new_pos-pos));
			break;

		case CTRL_KEY('Y'):	// Paste last cut text.
		case CTRL_KEY('V'):
		case KEY_INSERT:
			if ((len=(int)strlen(s_clip_buf)) + num < size) {
				memmove (&buf[pos+len], &buf[pos], num - pos);
				memcpy (&buf[pos], s_clip_buf, len);
				crossline_refreash (buf, &pos, &num, pos+len, num+len);
			}
			break;

/* Complete Commands */
		case KEY_TAB:		// Autocomplete (same with CTRL_KEY('I'))
		case ALT_KEY('='):	// List possible completions.
		case ALT_KEY('?'):
			if (in_his || (NULL == s_completion_callback) || (pos != num))
				{ break; }
			buf[pos] = '\0';
			completions.num = 0;
			completions.hints[0] = '\0';
			s_completion_callback (buf, &completions);
			if ((1 == completions.num) && (KEY_TAB == ch)) {
				for (new_pos=pos; new_pos > 0 && !isdelim(buf[new_pos-1]); --new_pos)	;
				snprintf (&buf[new_pos], size - new_pos, "%s " , completions.word[0]);
				buf[size-1] = '\0';
				len = (int)strlen(completions.word[0]) + 1;
				crossline_refreash (buf, &pos, &num, new_pos+len, new_pos+len);
			} else if (crossline_show_completions(&completions))
				{ printf ("%s%s", prompt, buf); }
			break;

/* History Commands */
		case KEY_UP:		// Fetch previous line in history.
		case CTRL_KEY('P'):
			if (in_his) { break; }
			if (!copy_buf)
				{ crossline_text_copy (input, buf, 0, num); copy_buf = 1; }
			if ((history_id > 0) && (history_id+CROSS_HISTORY_MAX_LINE > s_history_id))
				{ crossline_history_copy (buf, size, &pos, &num, --history_id); }
			break;

		case KEY_DOWN:		// Fetch next line in history.
		case CTRL_KEY('N'):
			if (in_his) { break; }
			if (!copy_buf)
				{ crossline_text_copy (input, buf, 0, num); copy_buf = 1; }
			if (history_id+1 < s_history_id)
				{ crossline_history_copy (buf, size, &pos, &num, ++history_id); }
			else {
				history_id = s_history_id;
				strncpy (buf, input, size - 1);
				buf[size - 1] = '\0';
				crossline_refreash (buf, &pos, &num, (int)strlen(buf), (int)strlen(buf));
			}
			break; //case UP/DOWN

		case ALT_KEY('<'):	// Move to first line in history.
		case KEY_PGUP:
			if (in_his) { break; }
			if (!copy_buf)
				{ crossline_text_copy (input, buf, 0, num); copy_buf = 1; }
			if (s_history_id > 0) {
				history_id = s_history_id < CROSS_HISTORY_MAX_LINE ? 0 : s_history_id-CROSS_HISTORY_MAX_LINE;
				crossline_history_copy (buf, size, &pos, &num, history_id);
			}
			break;

		case ALT_KEY('>'):	// Move to end of input history.
		case KEY_PGDN:
			if (in_his) { break; }
			if (!copy_buf)	
				{ crossline_text_copy (input, buf, 0, num); copy_buf = 1; }
			history_id = s_history_id;
			strncpy (buf, input, size-1);
			buf[size-1] = '\0';
			crossline_refreash (buf, &pos, &num, (int)strlen(buf), (int)strlen(buf));
			break;

		case CTRL_KEY('R'):	// Search history
		case CTRL_KEY('S'):
		case KEY_F4:		// Search history with current input.
			if (in_his) { break; }
			crossline_text_copy (input, buf, 0, num);
			search_his = crossline_history_search ((KEY_F4 == ch) ? buf : NULL);
			if (search_his > 0)	
				{ strncpy (buf, s_history_buf[search_his-1], size-1); }
			else { strncpy (buf, input, size-1); }
			buf[size-1] = '\0';
			printf ("%s%s", prompt, buf);
			pos = num = (int)strlen (buf);
			break;

		case KEY_F2:	// Show history
			if (in_his || (0 == s_history_id)) { break; }
			printf (" \b\n");
			crossline_history_show ();
			printf ("%s%s", prompt, buf);
			new_pos = pos; pos = num;
			crossline_refreash (buf, &pos, &num, new_pos, num);
			break;

		case KEY_F3:	// Clear history
			if (in_his) { break; }
			printf(" \b\n!!! Confirm to clear history [y]: ");
			if ('y' == crossline_getch()) {
				printf(" \b\nHistory are cleared!");
				crossline_history_clear ();
				history_id = 0;
			}
			printf (" \b\n%s%s", prompt, buf);
			crossline_refreash (buf, &pos, &num, pos, num);
			break;

/* Control Commands */
		case KEY_ENTER:		// Accept line (same with CTRL_KEY('M'))
		case KEY_ENTER2:	// same with CTRL_KEY('J')
			crossline_refreash (buf, &pos, &num, num, num);
			printf (" \b\n");
			read_end = 1;
			break;

		case CTRL_KEY('C'):	// Abort line.
		case CTRL_KEY('G'):
			crossline_refreash (buf, &pos, &num, num, num);
			if (CTRL_KEY('C') == ch)	{ printf (" \b^C\n"); }
			else	{ printf (" \b\n"); }
			num = pos = 0;
			errno = EAGAIN;
			fflush(stdout);
			return NULL;

		case CTRL_KEY('Z'):
#ifndef _WIN32
			raise(SIGSTOP);    // Suspend current process
			printf ("%s%s", prompt, buf);
			new_pos = pos; pos = num;
			crossline_refreash (buf, &pos, &num, new_pos, num);
#endif
			break;

		default:
			if (!is_esc && isprint(ch) && (num < size-1)) {
				memmove (&buf[pos+1], &buf[pos], num - pos);
				buf[pos] = (char)ch;
				crossline_refreash (buf, &pos, &num, pos+1, num+1);
				copy_buf = 0;
			}
			break;
        } // switch( ch )
	 	fflush(stdout);
	} while ( !read_end );

	if ((num > 0) && (' ' == buf[num-1]))	{ num--; }
	buf[num] = '\0';
	if (!in_his && (num > 0) && strcmp(buf,"history")) { // Save history
		if ((0 == s_history_id) || strncmp (buf, s_history_buf[(s_history_id-1)%CROSS_HISTORY_MAX_LINE], CROSS_HISTORY_BUF_LEN)) {
			strncpy (s_history_buf[s_history_id % CROSS_HISTORY_MAX_LINE], buf, CROSS_HISTORY_BUF_LEN);
			s_history_buf[s_history_id % CROSS_HISTORY_MAX_LINE][CROSS_HISTORY_BUF_LEN - 1] = '\0';
			history_id = ++s_history_id;
			copy_buf = 0;
		}
	}

	return buf;
}
