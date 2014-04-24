/*
 * Splits a file on lines matching a regular expression and creates new files
 * The new files are named filename_suffix1, filename_suffix2, and so on
 * This is destructive of existing files with those names
 * The lines matching the regular expression are excluded from the output
 * TODO: add inclusive option, add option to stop if existing files will be overwritten
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

int errno;
void create_output_filename(char *target, char *base_filename, char *suffix, int num);

int main(int argc, char *argv[]) {
    int o;
    char *suffix;
    char *filename;
    char *pattern;

    while ((o = getopt(argc, argv, "s:")) != -1) {
	switch(o) {
	case 's': suffix = optarg;
	    break;
	default:
	    fprintf(stderr, "Usage: %s [-s suffix] filename pattern\n", argv[0]);
	    exit(1);
	}
    }

    if (argc < optind + 2) {
	fprintf(stderr, "Insufficient arguments:\n");
	fprintf(stderr, "Usage: %s [-s suffix] filename pattern\n", argv[0]);
	exit(1);
    }
    filename = argv[optind];
    pattern = argv[optind + 1];

    regex_t regex;
    int rc;
    if ((rc = regcomp(&regex, pattern, REG_EXTENDED))) {
	fprintf(stderr, "error compiling regular expression: %s\n", pattern);
    }
    regmatch_t *matches = malloc(sizeof(regex_t) * (regex.re_nsub + 1));

    FILE *f = fopen(filename, "r");
    if (NULL == f) {
	fprintf(stderr, "error opening file: %s: %s\n", filename, strerror(errno));
	exit(1);
    }

    int out_file_num = 0;
    FILE *out_file = NULL;
    char *out_filename = malloc(sizeof(char) * FILENAME_MAX);

    int bufsize = 999999; // TODO this is probably brittle
    char buf[bufsize];
    while (fgets(buf, bufsize, f) != NULL) {
	if (regexec(&regex, buf, regex.re_nsub + 1, matches, 0)) {
	    if (NULL == out_file) {
		create_output_filename(out_filename, filename, suffix, ++out_file_num);
		out_file = fopen(out_filename, "w");
		if (NULL == out_file) {
		    fprintf(stderr, "error creating output file: %s: %s\n", out_filename, strerror(errno));
		    exit(1);
		}
	    }
	    fputs(buf, out_file);
	} else {
	    // A match: close the file and start a new one
	    if (NULL != out_file) {
		fclose(out_file);
		out_file = NULL;
	    }
	}
    }

    fclose(out_file);
    fclose(f);
    exit(0);
}

void create_output_filename(char *target, char *base_filename, char *suffix, int num) {
    if (NULL != suffix) {
	sprintf(target, "%s_%s%d", base_filename, suffix, num);
    } else {
	sprintf(target, "%s_%d", base_filename, num);
    }
}
