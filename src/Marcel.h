/* Marcel.h
 *
 * Shared definition
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */

#ifndef MARCEL_H
#define MARCEL_H

#define VERSION "2.0"
#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

	/* Helper functions */
extern int debug;

extern char *removeLF(char *);
extern char *striKWcmp( char *, const char * );
extern char *mystrdup(const char *);
#define strdup(s) mystrdup(s)

extern size_t socketreadline( int, char *, size_t);

#endif
