/* Helpers.c
 *
 * Utilities functions
 *
 * 30/09/2022 - Emancipate from Marcel.c
 */

#include "Marcel.h"
#include "MQTT_tools.h"

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

/**
 * @brief compares a string against a keyword
 *
 * @param s string to compare
 * @param kw keyword
 * @return remaining string if the keyword matches or NULL if not
 */
const char *striKWcmp( const char *s, const char *kw ){
	size_t klen = strlen(kw);
	if( strncasecmp(s,kw,klen) )
		return NULL;
	else
		return s+klen;
}

/**
 * @brief removes a potential LF at the end of the string
 */
char *removeLF(char *s){
	size_t l=strlen(s);
	if(l && s[--l] == '\n')
		s[l] = 0;
	return s;
}

/**
 * @brief returns the checksum of a string
 */
int chksum(const char *s){
	int h = 0;
	while(*s)
		h += *s++;
	return h;
}

/**
 * @brief read a line from a file descriptor
 *
 * @param fd file descriptor to read
 * @param l buffer to store the result
 * @param sz max size of the result
 * @return size read, -1 if error or EoF
 */
size_t socketreadline( int fd, char *l, size_t sz){
	int s=0;
	char *p = l, c;

	for( ; p-l< sz; ){
		int r = read( fd, &c, 1);

		if(r == -1)
			return -1;
		else if(!r){	/* EOF */
			return -1;
		} else if(c == '\n')
			break;
		else {
			*p++ = c;
			s++;
		}
	}
	*p = '\0';

	return s;
}


	/* ***
	 * Logging 
	 * ***/

/**
 * @brief Display and publish log information
 *
 * @param l Log level ('F'atal and 'E'rror goes to stderr, others to stdout)
 * @param msg Message to publish, in printf() format
 */
void publishLog( char l, const char *msg, ...){
	va_list args;

	if(cfg.client){
		va_start(args, msg);

		char *sub;
		switch(l){
		case 'F':
			sub = "/Log/Fatal";
			break;
		case 'E':
			sub = "/Log/Error";
			break;
		case 'W':
			sub = "/Log/Warning";
			break;
		case 'I':
			sub = "/Log/Information";
			break;
		case 'R':
			sub = "/Log/Corrected";
			break;
		default :	/* Trace */
			sub = "/Log";
		}

		char tmsg[1024];	/* No simple way here to know the message size */
		char ttopic[ strlen(cfg.ClientID) + strlen(sub) + 1 ];
		sprintf(ttopic, "%s%s", cfg.ClientID, sub);
		vsnprintf(tmsg, sizeof(tmsg), msg, args);

		mqttpublish( cfg.client, ttopic, strlen(tmsg), tmsg, 0);

		va_end(args);
	}

	if(cfg.verbose || l=='E' || l=='F'){
		va_start(args, msg);

		char t[ strlen(msg) + 7 ];
		sprintf(t, "*%c* %s\n", l, msg);
		vfprintf((l=='E' || l=='F')? stderr : stdout, t, args);

		va_end(args);
	}

}

	/* ***
	 * Variable substitution
	 * ***/

/**
 * @brief Compute all hashes of a variables table
 *
 * @param tbl Table to work with
 */
void init_VarSubstitution( struct _VarSubstitution *tbl ){
	while(tbl->var){
		tbl->lvar = strlen(tbl->var);
		tbl->h = chksum(tbl->var);
		tbl++;
	}
}

/**
 * @brief Set a variable value
 *
 * @param vars Table of variables
 * @param name Variable name's keyword
 * @param val Value to set (only it's pointer is copied)
 * @param freeval it true, free() current value
 * @return false if the variable hasn't been found
 */
bool setSubstitutionVar(struct _VarSubstitution *vars, const char *name, const char *val, bool freeval){
	int h = chksum(name);

	for(struct _VarSubstitution *tbl = vars; tbl->var; tbl++){
		if( h == tbl->h && !strcmp(name, tbl->var) ){
			if(freeval && tbl->val)
				free((void *)tbl->val);
			tbl->val = val;
			tbl->lval = strlen(val);
			return true;
		}
	}

#ifdef DEBUG
	if(cfg.debug)
		publishLog('E', "Variable '%s' not found", name);
#endif

	return false;
}

/**
 * @brief get a variable's value
 *
 * @param lookup Table of variables
 * @param name Variable name's keyword
 * @return Variable's content or NULL if not found
 */
const char *getSubstitutionVar( struct _VarSubstitution *lookup, const char *name ){
	int  h = chksum(name);

	for(struct _VarSubstitution *tbl = lookup; tbl->var; tbl++){
		if( h == tbl->h && !strcmp(name, tbl->var) )
			return tbl->val;
	}

	return NULL;
}

/**
 * @brief Replace variables found in the string by their value
 *
 * @param arg original string
 * @param lookup variables lookup tables (name, value)
 * @return malloced resulting string
 */
char *replaceVar( const char *arg, struct _VarSubstitution *lookup ){
	size_t idx, idxd, 				/* source and destination indexes */
		sz, max=strlen(arg);		/* size of allocated area and max index */

	char *s = malloc( sz=max+1 );	/* resulting string */
	assert(s);

	for(idx = idxd = 0; idx<max; idx++){
		if(arg[idx] == '%'){
			bool found=false;
			struct _VarSubstitution *t;

			for(t=lookup; t->var; t++){
				if(!strncmp(arg+idx, t->var, t->lvar)){
					sz += t->lval - t->lvar;
					assert((s = realloc(s, sz)));
					strcpy(s+idxd, t->val);		/* Insert the value */
					idxd += t->lval;			/* Skip variable's content */
					idx += t->lvar-1;			/* Skip variable's name */

					found=true;
					break;
				}
			}

			if(!found)
				s[idxd++] = arg[idx];
		} else
			s[idxd++] = arg[idx];
	}

	s[idxd] = 0;
	return s;
}

