Crossline
=========
**Crossline** is a small, self-contained, zero-config, MIT licensed, cross-platform, readline and libedit replacement.

When should you use Crossline:
* When you need a cross-platform readline: Windows, Linux, Unix, MacOS.
* When you need a simple but versatile readline: more shortcuts, advanced search, simple autocomplete, paging, embedded help.
* When you need a customized readline: easy to extend.
* When you need a small readline to build into program.

Features and Highlights
-----------------------
* Support Windows, Linux, vt100 and xterm.
* Support total `79 shortcuts` and `37 functions`.
* Support most readline shortcuts (Emacs Standard bindings): fast move, edit, cut&paste, complete, history, control.
* Support some Windows command line shortcuts and extend some new convenient shortcuts.
* Support history navigation, history show/clear and history save/load.
* Support autocomplete, key word help and syntax hints.
* Support powerful interactive history search with multiple case insensitive including and excluding match patterns.
* Support same edit shortcuts (except complete and history shortcuts) in history search mode.
* Support autocomplete, history show/search, help info paging with auto resizing.
* Support convenient embedded `F1` help in edit and history search mode, and you can call it anytime without losing current input.
* Support convenient embedded `Ctrl-^` keyboard debug mode to watch key code sequences.
* Support `Ctrl-C` to exit edit and `Ctrl-Z` to suspend and resume job(Linux) in both edit and search mode.
* Support pipe as input.
* Pure C MIT license source code, no 3rd library dependency.
* No any dynamic memory operations: malloc/free/realloc/new/delete/strdup/etc.
* Very small only about 1000 LOC, and code logic is simple and easy to read.
* Easy to customize your own shortcuts and new features.
* Unicode is to be supported later.

Background
----------
I'm developing a cross-platfrom command line tool which can support autocomplete and history. **Readline** library is the first choice, but it can't use on Windows directly and you need to link the readline library explicitly. You also need to install the libreadline-dev package to build and if target machine version mismatches with build machine, the program can't run. The libreadline has over 30K LOC and has two so files: libreadline.so libhistory.so, and it depends on libncurses also.

Then I searched and found there's a small readline replacement **linenoise** written by Redis's author. Linenoise is used in Redis, MongoDB, and Android. It's very small (about 1,100 LOC), but it can only run on Linux and supports few shortcuts and features. Then I found there's a more powerful library **linenoise-ng** used in ArangoDB. It's based on MongoDB's linenoise which enhanced original linenoise to support Windows, more shortcuts, features and Unicode. But the code is embedded in MongoDB source code, then ArangoDB ported it out, did some improvements and made it an independent library.

At first I planned to use linenoise-ng, but I found it's still big (about 4,300 LOC) and complex. It uses C++ and C together, and C/C++ dynamic memory is used also which is there in original linenoise. I think it can be much simpler, then I did some prototype verification, and use a different method to implement this brand new, simple, pure C cross-platform enhanced readline replacement library.

Shortcuts
---------

**Misc Commands**

Shortcut                | Action
---------               | ------
F1                      |   Show edit shortcuts help.
Ctrl-^                  |   Enter keyboard debugging mode.  

**Move Commands**

Shortcut                | Action
---------               | ------
Ctrl-B, Left            |   Move back a character.
Ctrl-F, Right           |   Move forward a character.
Alt-B, ESC+Left, Ctrl-Left, Alt-Left    | Move back a word. (Ctrl-Left, Alt-Left only support Windows/Xterm)
Alt-F, ESC+Right, Ctrl-Right, Alt-Right | Move forward a word. (Ctrl-Right, Alt-Right only support Windows/Xterm)
Ctrl-A, Home            |   Move cursor to start of line.
Ctrl-E, End             |   Move cursor to end of line.
Ctrl-L                  |   Clear screen and redisplay line.

**Edit Commands**

Shortcut                | Action
---------               | ------
Ctrl-H, Backspace       |   Delete character before cursor.
Ctrl-D, DEL             |   Delete character under cursor.
Alt-U,  ESC+Up, Ctrl-Up, Alt-Up      | Uppercase current or following word.(Ctrl-Up, Alt-Up only supports Windows/Xterm)
Alt-L, ESC+Down, Ctrl-Down, Alt-Down | Lowercase current or following word. (Ctrl-Down, Alt-Down only support Windows/Xterm)
Alt-C                   |   Capitalize current or following word.
Alt-\                   |   Delete whitespace around cursor.
Ctrl-T                  |   Transpose previous character with current character.

**Cut&Paste Commands**

Shortcut                | Action
---------               | ------
Ctrl-K, ESC+End, Ctrl-End, Alt-End       | Cut from cursor to end of line. (Ctrl-End, Alt-End only support Windows/Xterm)
Ctrl-U, ESC+Home, Ctrl-Home, Alt-Home    | Cut from start of line to cursor. (Ctrl-Home, Alt-Home only support Windows/Xterm)
Ctrl-X                  |   Cut whole line.
Alt-Backspace, Esc+Backspace, Clt-Backspace  | Cut word to left of cursor. (Clt-Backspace only supports Windows/Xterm)
Alt-D, ESC+Del, Alt-Del, Ctrl-Del        | Cut word following cursor. (Alt-Del,Ctrl-Del only support Windows/Xterm)
Ctrl-W                  |   Cut to left till whitespace (not word).
Ctrl-Y, Ctrl-V, Insert  |   Paste last cut text.

  **Complete Commands**

Shortcut                | Action
---------               | ------
TAB, Ctrl-I             |   Autocomplete.
Alt-=, Alt-?            |   List possible completions.

  **History Commands**

Shortcut                | Action
---------               | ------
Ctrl-P, Up              |   Fetch previous line in history.
Ctrl-N, Down            |   Fetch next line in history.
Alt-<,  PgUp            |   Move to first line in history.
Alt->,  PgDn            |   Move to end of input history.
Ctrl-R, Ctrl-S          |   Search history.
F4                      |   Search history with current input.
F1                      |   Show search help when in search mode.
F2                      |   Show history.
F3                      |   Clear history (need confirm).

  **Control Commands**

Shortcut                | Action
---------               | ------
Enter,  Ctrl-J, Ctrl-M  |   EOL and accept line.
Ctrl-C, Ctrl-G          |   EOF and abort line.
Ctrl-D                  |   EOF if line is empty.
Alt-R                   |   Revert line. Undo all changes made to this line.
Ctrl-Z                  |   Suspend Job. (Linux only, fg will resume edit)

  **Notes**
* For Windows and xterm, almost all shortcuts are supported.
* For some terminal tools you need to enable Alt as meta key.
  SecureCRT: check `Terminal->Emulation->Emacs->use ALT as meta key`
* Backspace key is 8 or 127, and Del key should use Escape code.
  SecurCRT: check `Terminal->Emulation->Mapped Keys->Backspace sends delete`
* For vt100 and Linux terminals, many `Alt-key` doesn't work, and an alternate way is to press `ESC` first then press key, see above `ESC+Key`.
  Putty can send `Alt` as `ESC`, so no need to do this way.
* `Ctrl+S` is readline shortcut to search history also, but it'll halt terminal, so don't use it with Linux system, use `Ctrl+Q` to exit freezing state if pressing by mistake.

**Terminal Limitations**
* Some terminals only support `left Alt`.
* Linux console doesn't support: `Alt-?`, `Alt-<`, `Alt->`.
* SecureCRT vt100 doesn't support: `Home`, `End`, `Del`, `Insert`, `PgUp`, `PgDn`.
* SecureCRT xterm doesn't support: `Ctrl-Home`, `Alt-Home`, `Ctrl-End`, `Alt-End`, `Alt-Del`, `Ctrl-Del`, `Clt-Backspace`.

History Search
--------------
Original readline supports incremental search(`Ctrl-R`,`Ctrl-S`) and none-incremental search(`Alt-N`,`Alt-P`). I tried both and think they're not convenient or efficient to use, so I implemented a brand new interactive search method.

**Enter interactive history search mode**
* `Ctrl-R`: Search history.
* `F4`: Search history with current input as search patterns.

**Exit interactive history search mode**
* `Ctrl-C`: You can exit search mode anytime and keep your original input.
* `Ctrl-D`: On empty line only.

**Input patterns**
* You can use all edit shortcuts except complete and history shortcuts.
* You can use `Insert`,`Ctrl-Y`,`Ctrl-S` to paste last search patterns.
* If nothing input will show all history (`F2` can show history too).

**Patterns syntax**
You can press `F1` to get help.
Patterns are separated by `' '`, patter match is `case insensitive`.
* `select`:   choose line including 'select'
* `-select`:  choose line excluding 'select'
* `"select from"`:  choose line including "select from"
* `-"select from"`: choose line excluding "select from"
* `"select from" where -"order by" -limit` : choose line including "select from" and 'where' and excluding "order by" or 'limit'

**Select history**
* If only one history is found, id `"1"` is provided automatically, and you just press `Enter` to select.
* You can press `Ctrl-C` to skip choosing.
* You can press `Enter` or `Ctrl-D` on empty line to skip choosing.

**Example**

    SQL> <F2> // show history
       1  hello world
       2  select from user
       3  from select table
       4  SELECT from student
       5  Select from teacher
    SQL> <Ctrl+R>
    Input Patterns <F1> help: select from
       1  select from user
       2  from select table
       3  SELECT from student
       4  Select from teacher
    Input history id: 3
    SQL> SELECT from student<Alt+R> // Revert line
    SQL> <F4>
    Input Patterns <F1> help: <Insert> // paste last history pattern: select from
    Input Patterns <F1> help: "select from"
       1  select from user
       2  SELECT from student
       3  Select from teacher
    Input history id: 3
    SQL> Select from teacher<Alt+R>
    SQL> SELECT from<F4> // search with pattern: Select from
    Input Patterns <F1> help: SELECT from
    Input Patterns <F1> help: "SELECT from" -user -teacher
       1  SELECT from student
    Input history id: 1
    SQL> SELECT from student

Keyboard Debug
--------------
**Enter keyboard debug mode**
Press `Ctrl-^` to enter keyboard debug mode, then you can type any key or composite key, and the code sequence will be displaced. This can be used to discover new key code sequences or debug especially for Linux system with different terminals.
Note: For Windows, `Alt` key is a state, so it's not displayed.

**Exit keyboard debug mode**
Press `Ctrl-C` to exit.

**Example**

    SQL> <Ctrl-^>
    Enter keyboard debug mode, <Ctrl-C> to exit debug
     27 0x1b ( )
     91 0x5b ([)
     65 0x41 (A)
     31 0x1f ( )
     27 0x1b ( )
     91 0x5b ([)
     72 0x48 (H)

Embedded Help
-------------

**Edit mode**

    SQL> <F1>
    Misc Commands
     +-------------------------+--------------------------------------------------+
     | F1                      |  Show edit shortcuts help.                       |
     | Ctrl-^                  |  Enter keyboard debugging mode.                  |
     +-------------------------+--------------------------------------------------+
     Move Commands
     +-------------------------+--------------------------------------------------+
     | Ctrl-B, Left            |  Move back a character.                          |
     | Ctrl-F, Right           |  Move forward a character.                       |
     | Alt-B, ESC+Left,        |  Move back a word.                               |
     |    Ctrl-Left, Alt-Left  |  (Ctrl-Left, Alt-Left only support Windows/Xterm)|
    *** Press <Space> or <Enter> to continue . . .

**Search mode**

    SQL> <F4> or <Ctrl+R>
    Input Patterns <F1> help: <F1>
    Patterns are separated by ' ', patter match is case insensitive:
        select:   choose line including 'select'
        -select:  choose line excluding 'select'
        "select from":  choose line including "select from"
        -"select from": choose line excluding "select from"
    Example:
        "select from" where -"order by" -limit:
             choose line including "select from" and 'where'
             and excluding "order by" or 'limit'

Extend Crossline
----------------
You can extend crossline to add new shortcuts and action easily.

**Use keyboard debug mode to find the key code sequences**
* For Windows, most key has only one code, some function key has 2, first is 224 or 0, then follows another code. `Alt` key in Windows is a state, please refer `crossline_getkey`.
* For Linux system, most function key has an escape sequences(up to 6), and one function key may have many different escape sequences for different terminal modes, so you need to test case by case.

**Define new key**
* For `Ctrl-key`, most are single code, and you can use `CTRL_KEY(key)` directly. 
* For windows, most `Alt-key` can use `ALT_KEY(num)`.
* For Linux, most `Alt` key is escape sequences, and you can use existing macro like `ESC_KEY6`, example: `KEY_ALT_LEFT    = ESC_KEY6('1','3','D'), // xterm Esc[1;3D: Move back a word.`

**Support new Esc+key**
Please add the conversion in `crossline_key_esc2alt`

**Map new key to existing action**
* If it's a new different escape sequences for same key, please map the new key to main action key in `crossline_key_mapping`.
* If it's a new key, please add case to `crossline_readline_input`.

**Add new action**
Please add case and action code in `crossline_readline_input`.
You can refer existing case of similar action to write your new action.
Use `crossline_refreash` to print line after updating buf.

Customized Config
-----------------
**Word delimiters for move and cut**
Default is defined by `CROSS_DFT_DELIMITER`.
```c
#define CROSS_DFT_DELIMITER            " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
```
You can modify it or use `crossline_delimiter_set` to change it.

**History**
The history line len can be less than user buf len, and it'll cut to history line len when storing to history buf.
```c
#define CROSS_HISTORY_MAX_LINE        256        // Maximum history line number
#define CROSS_HISTORY_BUF_LEN        1024        // History line length
#define CROSS_HIS_MATCH_PAT_NUM        16        // History search pattern number
```

**Completion**
```c
#define CROSS_COMPLET_MAX_LINE        256        // Maximum completion word number
#define CROSS_COMPLET_WORD_LEN        64         // Completion word length
#define CROSS_COMPLET_HELP_LEN        128        // Completion word's help length
#define CROSS_COMPLET_HINT_LEN        128        // Completion syntax hints length
```

Crossline APIs
--------------
```c
// Main API to read a line, return buf if get line, return NULL if EOF.
char* crossline_readline (char *buf, int size, const char *prompt);
// Set move/cut word delimiter, default is all not digital and alphabetic characters
void  crossline_delimiter_set (const char *delim);

// History APIs
int   crossline_history_save (const char *filename);
int   crossline_history_load (const char *filename);
void  crossline_history_show (void);
void  crossline_history_clear (void);

/* Completion APIs */
// Register completion callback
void  crossline_completion_register (crossline_completion_callback pCbFunc);
// Add completion in callback. Word is must, help for word is optional.
void  crossline_completion_add (crossline_completions_t *pCompletions, const char *word, const char *help);
// Set syntax hints in callback
void  crossline_hints_set (crossline_completions_t *pCompletions, const char *hints);
```

Simple Example
--------------
example.c
```c
static void completion_hook (char const *buf, crossline_completions_t *pCompletion)
{
    int i;
    static const char *cmd[] = {"insert", "select", "update", "delete", "create", "drop", "show", "describe", "help", "exit", "history", NULL};

    for (i = 0; NULL != cmd[i]; ++i) {
        if (0 == strncmp(buf, cmd[i], strlen(buf))) {
            crossline_completion_add (pCompletion, cmd[i], NULL);
        }
    }

}

int main ()
{
    char buf[256];
    
    crossline_completion_register (completion_hook);
    crossline_history_load ("history.txt");

    while (NULL != crossline_readline (buf, sizeof(buf), "Crossline> ")) {
        printf ("Read line: \"%s\"\n", buf);
    }    

    crossline_history_save ("history.txt");
    return 0;
}
```

SQL Parser Example
------------------
example_sql.c
This example implements a simple SQL syntax parser with following syntax format, please read code for details.
```sql
insert into <table> set column1=value1,column2=value2,...
select <* | column1,columnm2,...> from <table> [where] [order by] [limit] [offset]
update <table> set column1=value1,column2=value2 [where] [order by] [limit] [offset]
delete from <table> [where] [order by] [limit] [offset]
create [unique] index <name> on <table> (column1,column2,...)
drop {table | index} <name>
show {tables | databases}
describe <table>
help {insert | select | update | delete | create | drop | show | describe | help | exit | history}
```
You can use this example to practice the shortcuts above.

     SQL> <TAB>  // show autocomplete words and help
     insert      Insert a record to table
     select      Select records from table
     update      Update records in table
     delete      Delete records from table
     create      Create index on table
     drop        Drop index or table
     show        Show tables or databases
     describe    Show table schema
     help        Show help for topic
     exit        Exit shell
     history     Show history
      *** Press <Space> or <Enter> to continue . . .
     SQL> help <TAB> // show autocomplete words list
     insert    select    update    delete    create    drop      show
     describe  help      exit      history
     SQL> create index <TAB> // show autocomplete hints
     Please input: index name

Build and Test
--------------
On Windows, you can add the source code to a Visual Studio project to build or enter `Tools Command Prompt for VS` from menu to build in command line which is more efficient.

**Windows MSVC**

    cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example.c /Feexample.exe
    cl -D_CRT_SECURE_NO_WARNINGS -W4 User32.Lib crossline.c example_sql.c /Feexample_sql.exe

**Windows Clang**

    clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example.c -o example.exe
    clang -D_CRT_SECURE_NO_WARNINGS -Wall -lUser32 crossline.c example_sql.c -o example_sql.exe

**Linux Clang**

    clang -Wall crossline.c example.c -o example
    clang -Wall crossline.c example_sql.c -o example_sql
    
**GCC(Linux, MinGW, Cygwin, MSYS2)**

    gcc -Wall crossline.c example.c -o example
    gcc -Wall crossline.c example_sql.c -o example_sql

Related Projects
----------------
* [Linenoise](https://github.com/antirez/linenoise) a small self-contained alternative to readline and libedit
* [Linenoise-ng](https://github.com/arangodb/linenoise-ng) is a fork of Linenoise that aims to add more advanced features like UTF-8 support, Windows support and other features. Uses C++ instead of C as development language.

Thanks
-JC
