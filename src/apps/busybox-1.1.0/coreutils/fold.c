/* fold -- wrap each input line to fit in specified width.

   Written by David MacKenzie, djm@gnu.ai.mit.edu.
   Copyright (C) 91, 1995-2002 Free Software Foundation, Inc.

   Modified for busybox based on coreutils v 5.0
   Copyright (C) 2003 Glenn McGrath <bug1@iinet.net.au>

   Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
*/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "busybox.h"

/* If nonzero, count bytes, not column positions. */
static unsigned long flags;
#define FLAG_COUNT_BYTES	1
#define FLAG_BREAK_SPACES	2
#define FLAG_WIDTH			4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */

static int adjust_column(int column, char c)
{
	if (!(flags & FLAG_COUNT_BYTES)) {
		if (c == '\b') {
			if (column > 0)
				column--;
		} else if (c == '\r')
			column = 0;
		else if (c == '\t')
			column = column + 8 - column % 8;
		else			/* if (isprint (c)) */
			column++;
	} else
		column++;
	return column;
}

extern int fold_main(int argc, char **argv)
{
	char *w_opt;
	int width = 80;
	int i;
	int errs = 0;


#ifdef CONFIG_FEATURE_SUSv2_OBSOLETE
	/* Turn any numeric options into -w options.  */
	for (i = 1; i < argc; i++) {
		char const *a = argv[i];

		if (a[0] == '-') {
			if (a[1] == '-' && !a[2])
				break;
			if (isdigit(a[1])) {
				char *s = xmalloc(strlen(a) + 2);

				s[0] = '-';
				s[1] = 'w';
				strcpy(s + 2, a + 1);
				argv[i] = s;
			}
		}
	}
#endif

	flags = bb_getopt_ulflags(argc, argv, "bsw:", &w_opt);
	if (flags & FLAG_WIDTH)
		width = bb_xgetlarg(w_opt, 10, 1, 10000);

	argv += optind;
	if (!*argv) {
		*--argv = "-";
	}

	do {
		FILE *istream = bb_wfopen_input(*argv);
		if (istream != NULL) {
			int c;
			int column = 0;		/* Screen column where next char will go. */
			int offset_out = 0;	/* Index in `line_out' for next char. */
			static char *line_out = NULL;
			static int allocated_out = 0;

			while ((c = getc(istream)) != EOF) {
				if (offset_out + 1 >= allocated_out) {
					allocated_out += 1024;
					line_out = xrealloc(line_out, allocated_out);
				}

				if (c == '\n') {
					line_out[offset_out++] = c;
					fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
					column = offset_out = 0;
					continue;
				}

rescan:
				column = adjust_column(column, c);

				if (column > width) {
					/* This character would make the line too long.
					   Print the line plus a newline, and make this character
					   start the next line. */
					if (flags & FLAG_BREAK_SPACES) {
						/* Look for the last blank. */
						int logical_end;

						for (logical_end = offset_out - 1; logical_end >= 0; logical_end--) {
							if (isblank(line_out[logical_end])) {
								break;
							}
						}
						if (logical_end >= 0) {
							/* Found a blank.  Don't output the part after it. */
							logical_end++;
							fwrite(line_out, sizeof(char), (size_t) logical_end, stdout);
							putchar('\n');
							/* Move the remainder to the beginning of the next line.
							   The areas being copied here might overlap. */
							memmove(line_out, line_out + logical_end, offset_out - logical_end);
							offset_out -= logical_end;
							for (column = i = 0; i < offset_out; i++) {
								column = adjust_column(column, line_out[i]);
							}
							goto rescan;
						}
					} else {
						if (offset_out == 0) {
							line_out[offset_out++] = c;
							continue;
						}
					}
					line_out[offset_out++] = '\n';
					fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
					column = offset_out = 0;
					goto rescan;
				}

				line_out[offset_out++] = c;
			}

			if (offset_out) {
				fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
			}

			if (ferror(istream) || bb_fclose_nonstdin(istream)) {
				bb_perror_msg("%s", *argv);	/* Avoid multibyte problems. */
				errs |= EXIT_FAILURE;
			}
		} else {
			errs |= EXIT_FAILURE;
		}
	} while (*++argv);

	bb_fflush_stdout_and_exit(errs);
}
/* vi: set sw=4 ts=4: */
