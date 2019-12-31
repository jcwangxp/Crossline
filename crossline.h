/* crossline.h -- Version 1.0
 *
 * Crossline is a small, self-contained, zero-config, MIT licensed, 
 *   cross-platform, readline and libedit replacement.
 *
 * Press <F1> to get full shortcuts list.
 *
 * See crossline.c for more information.
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

#ifndef __CROSSLINE_H
#define __CROSSLINE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct crossline_completions_t crossline_completions_t;
typedef void (*crossline_completion_callback) (const char *buf, crossline_completions_t *pCompletions);

// Main API to read a line, return buf if get line, return NULL if EOF.
extern char* crossline_readline (char *buf, int size, const char *prompt);
// Set move/cut word delimiter, default is all not digital and alphabetic characters.
extern void  crossline_delimiter_set (const char *delim);

/* History APIs */
// Save history to file
extern int   crossline_history_save (const char *filename);
// Load history from file
extern int   crossline_history_load (const char *filename);
// Show history in buffer
extern void  crossline_history_show (void);
// Clear history
extern void  crossline_history_clear (void);

/* Completion APIs */
// Register completion callback
extern void  crossline_completion_register (crossline_completion_callback pCbFunc);
// Add completion in callback. Word is must, help for word is optional.
extern void  crossline_completion_add (crossline_completions_t *pCompletions, const char *word, const char *help);
// Set syntax hints in callback
extern void  crossline_hints_set (crossline_completions_t *pCompletions, const char *hints);

/* Paging APIs */
// Get screen rows and columns
extern void crossline_screen_get (int *rows, int *cols);
// Reset paging before starting paing control
extern void crossline_paging_reset (void);
// Check paging after print a line, return 1 means quit, 0 means continue
// if you know only one line is printed, just give print_line = 1
extern int  crossline_paging_check (int print_line);

#ifdef __cplusplus
}
#endif

#endif /* __CROSSLINE_H */
