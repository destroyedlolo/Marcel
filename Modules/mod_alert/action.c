/* mod_alert/action
 *
 * Actions related to notifications
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 23/11//2022 - LF - First version
 */

#include "mod_alert.h"	/* module's own stuffs */

#include <stdlib.h>

void execOSCmd(const char *cmd, const char *id, const char *msg){
	const char *p = cmd;
	size_t nbre=0;	/* # of %t% in the string */

	if(!p)
		return;

	while((p = strstr(p, "%t%"))){
		nbre++;
		p += 3;	/* add '%t%' length */
	}

	char tcmd[ strlen(cmd) + nbre*strlen(id) + 1 ];
	char *d = tcmd;
	p = cmd;

	while(*p){
		if(*p =='%' && !strncmp(p, "%t%",3)){
			for(const char *s = id; *s; s++)
				if(*s =='"')
					*d++ = '\'';
				else
					*d++ = *s;
			p += 3;
		} else
			*d++ = *p++;
	}
	*d = 0;

	FILE *f = popen( tcmd, "w");
	if(!f){
		perror("popen()");
		return;
	}
	fputs(msg, f);
	fclose(f);
}
