/* OutFile.c
 *	Write value to file
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/08/2016	- LF - creation
 * 20/08/2016	- LF - handles Disabled
 */

#include "OutFile.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

void processOutFile( struct _OutFile *ctx, const char *msg){
	if(ctx->disabled){
		if(verbose)
			printf("*I* Writing to OutFile '%s' is disabled\n", ctx->file);
		return;
	}

	FILE *f=fopen( ctx->file, "w" );
	if(!f){
		fprintf(stderr, "*E* '%s' : %s\n", ctx->file, strerror(errno));
		return;
	}
	fputs(msg, f);
	if(ferror(f))
		fprintf(stderr, "*E* '%s' : %s\n", ctx->file, strerror(errno));
	else if(verbose)
		printf("*I* '%s' written in '%s'\n", msg, ctx->file);
	fclose(f);
	return;
}
