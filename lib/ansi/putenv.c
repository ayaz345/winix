/*
 * (c) copyright 1989 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: /cvsup/minix/src/lib/other/putenv.c,v 1.1.1.1 2005/04/21 14:56:27 beng Exp $ */

#include	<stdlib.h>
#include	<string.h>

#define	ENTRY_INC	10
#define	rounded(x)	(((x / ENTRY_INC) + 1) * ENTRY_INC)

int putenv(char *name)
{
	const char **v;
	char *r;
	static int size = 0;
	/* When size != 0, it contains the number of entries in the
	 * table (including the final NULL pointer). This means that the
	 * last non-null entry  is environ[size - 2].
	 */

    if (!name) return 0;
    if(_environ == NULL)
        init_environ();
	if (_environ == NULL) return 1;
	v = _environ;
	if ((r = strchr(name, '='))) {
		char *p, *q;

		*r = '\0';

		if (v != NULL) {
			while ((p = (char *)(unsigned long)*v) != NULL) {
				q = name;
				while (*q && (*q++ == *p++))
					/* EMPTY */ ;
				if (*q || (*p != '=')) {
					v++;
				} else {
					/* The name was already in the
					 * environment.
					 */
					*r = '=';
					*v = name;
					return 0;
				}
			}
		}
		*r = '=';
		v = _environ;
	}

	if (!size) {
		const char **p;
		int i = 0;

		if (v){
			do {
				i++;
			} while (*v++);
		}
			
		if (!(v = malloc(rounded(i) * sizeof(char **))))
			return 1;
		size = i;
		p = _environ;
		_environ = v;
		while ((*v++ = *p++));		/* copy the environment */
		v = _environ;
	} else if (!(size % ENTRY_INC)) {
		if (!(v = realloc((void *)_environ, rounded(size) * sizeof(char **))))
			return 1;
		_environ = v;
	}
	v[size - 1] = name;
	v[size] = NULL;
	size++;
	return 0;
}
