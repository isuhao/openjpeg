/*
 * Copyright (c) 1987, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* last review : october 29th, 2002 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getopt.c	8.3 (Berkeley) 4/27/95";
#endif				/* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr = 1,			/* if error message should be printed */
 optind = 1,			/* index into parent argv vector */
 optopt,			/* character checked for validity */
 optreset;			/* reset getopt */
const char *optarg;			/* argument associated with option */

typedef struct option
{
	char *name;
	int has_arg;
	int *flag;
	int val;
}option_t;

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
#define	EMSG	""



static void getopterror(int which) {
	static char error1[]="Unknown option `-x'.\n";
	static char error2[]="Missing argument for `-x'.\n";
	if (opterr) {
		if (which) {
			error2[23]=optopt;
			fprintf(stderr,"%s\n",error2);
			
		} else {
			error1[17]=optopt;
			fprintf(stderr,"%s\n",error1);
		}
	}
}


/*
 * getopt --
 *	Parse argc/argv argument vector.
 */
int getopt(int nargc, char *const *nargv, const char *ostr) {
#  define __progname nargv[0]
  static const char *place = EMSG;	/* option letter processing */
  char *oli;			/* option letter list index */

  if (optreset || !*place) {	/* update scanning pointer */
    optreset = 0;
    if (optind >= nargc || *(place = nargv[optind]) != '-') {
      place = EMSG;
      return (-1);
    }
    if (place[1] && *++place == '-') {	/* found "--" */
      ++optind;
      place = EMSG;
      return (-1);
    }
  }				/* option letter okay? */
  if ((optopt = (int) *place++) == (int) ':' ||
      !(oli = strchr(ostr, optopt))) {
    /*
     * if the user didn't specify '-' as an option,
     * assume it means -1.
     */
    if (optopt == (int) '-')
      return (-1);
    if (!*place)
      ++optind;
    if (opterr && *ostr != ':')
      (void) fprintf(stderr,
		     "%s: illegal option -- %c\n", __progname, optopt);
    return (BADCH);
  }
  if (*++oli != ':') {		/* don't need argument */
    optarg = NULL;
    if (!*place)
      ++optind;
  } else {			/* need an argument */
    if (*place)			/* no white space */
      optarg = place;
    else if (nargc <= ++optind) {	/* no arg */
      place = EMSG;
      if (*ostr == ':')
	return (BADARG);
      if (opterr)
	(void) fprintf(stderr,
		       "%s: option requires an argument -- %c\n",
		       __progname, optopt);
      return (BADCH);
    } else			/* white space */
      optarg = nargv[optind];
    place = EMSG;
    ++optind;
  }
  return (optopt);		/* dump back option letter */
}


int getopt_long(int argc, char * const argv[], const char *optstring,
struct option *longopts, int *longindex, int totlen) {
	static int lastidx,lastofs;
	char *tmp;
	int i,len;
again:
	if (optind>argc || !argv[optind] || *argv[optind]!='-' || argv[optind][1]==0)
		return -1;

	if (argv[optind][0]=='-' && argv[optind][1]==0) {
		++optind;
		return -1;
	}

	if (argv[optind][0]=='-') {	/* long option */
		char* arg=argv[optind]+1;
		char* max=strchr(arg,'=');
		const struct option* o;
		if (!max) max=arg+strlen(arg);
		o=longopts;
		len=sizeof(longopts[0]);

		for (i=0;i<totlen;i=i+len,o++) {
			if (!strncmp(o->name,arg,(size_t)(max-arg))) {	/* match */
				if (longindex) *longindex=o-longopts;
				if (o->has_arg>0) {
					if (*max=='=')
						optarg=max+1;
					else {
						optarg=argv[optind+1];
						if(optarg){
							if (strchr(optarg,'-')){ /* No argument */
								if (*optstring==':') return ':';
								if (opterr)
								(void) fprintf(stderr,"%s: option requires an argument %c\n",arg, optopt);
								return (BADCH);
								++optind;
							}
						}
						if (!optarg && o->has_arg==1) {	/* no argument there */
							if (*optstring==':') return ':';
							if (opterr)
							(void) fprintf(stderr,"%s: option requires an argument %c\n",arg, optopt);
							return (BADCH);
							++optind;
						}
						++optind;
					}
				}
				++optind;
				if (o->flag)
					*(o->flag)=o->val;
				else
					return o->val;
				return 0;
			}
		}//(end for)

		if (*optstring==':') return ':';

		if (lastidx!=optind) {
			lastidx=optind; lastofs=0;
		}
		optopt=argv[optind][lastofs+1];

		if ((tmp=strchr(optstring,optopt))) {
			if (*tmp==0) {	/* apparently, we looked for \0, i.e. end of argument */
				++optind;
				goto again;
			}

			if (tmp[1]==':') {	/* argument expected */
				if (tmp[2]==':' || argv[optind][lastofs+2]) {	/* "-foo", return "oo" as optarg */
					if (!*(optarg=argv[optind]+lastofs+2)) optarg=0;
					goto found;
				}

				optarg=argv[optind+1];
				if (!optarg) {	/* missing argument */
					++optind;
					if (*optstring==':') return ':';
					getopterror(1);
					return ':';
				}
				++optind;
			}
			else {
				++lastofs;
				return optopt;
			}
found:
			++optind;
			return optopt;
		} 
		else {	/* not found */
			getopterror(0);
			++optind;
			return '?';
		}
		
		fprintf(stderr,"Invalid option %s\n",arg);
		++optind;
		return '?';
	}// end of long option
}
