/*****************************************************************************
 *                                                                           *
 * libsimpleparser - a simple parsing library                                *
 * Copyright (C) 2001 Radu Constantin Rendec                                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with this library; if not, write to the Free Software       *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *                                                                           *
 * The full text of the license can be found in lgpl.txt                     *
 *                                                                           *
 *****************************************************************************/

#include <stdarg.h>
#include "simpleparser.h"


#define BUF_SLICE 256

#define is_blank(c) ((c)=='\t'||(c)==' '||(c)=='\n')
#define BLANK "\t \n"
#define add_char(chr) \
	if (j>=buf_sz) { \
		buf_sz+=BUF_SLICE; \
		if ((buf=(char *)realloc(buf, buf_sz))==NULL) return 10; \
	} \
	buf[j++]=(chr)

static int spa_debug_level=0;

static void spa_debug(int level, ...) {
	va_list ap;
	char *fmt;

	if (spa_debug_level < level) return;
	va_start(ap, level);
	fmt=va_arg(ap, char *);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

/* FIXME:
 * #ifdef DEBUG
 * #define SPA_DEBUG(lev,param) printf param;  -- ce fac cu stderr?
 * #else
 * #define SPA_DEBUG(lev,param) ;
 * #endif
 */

static int flush_line(FILE *f, char *fbuf) {
	while (fgets(fbuf, BUF_SLICE, f)!=NULL) {
		if (fbuf[strlen(fbuf)-1]=='\n') return 0;
	}
	return 1;
}

static const struct spa_keyword *find_keyword(char *key, const struct spa_keyword *dict, int n) {
	int l, q;
	while (n) {
		l=n >> 1;
		q=strcasecmp(key, dict[l].keyword);
		if (!q) return dict+l;
		if (q<0) {
			n=l;
			continue;
		}
		/* q>0 */
		dict+=l+1;
		n-=l+1;
	}
	return NULL;
}

struct parse_block_stack {
	const struct spa_keyword *lang, *key;
	int nw;
	struct parse_block_stack *prev, *next;
};

struct stack_data {
	struct parse_block_stack *base;		/* pointer to the first element in the stack */
	struct parse_block_stack **last;	/* pointer to the 'next' field of the last element in the stack */
	struct parse_block_stack *top;		/* pointer to the last element in the stack */
};

static int push(struct stack_data *sd, const struct spa_keyword *lang, int nw,
		const struct spa_keyword *key) {
	if ((*sd->last=(struct parse_block_stack *)malloc(sizeof(struct parse_block_stack)))==NULL) {
		return 1;
		/* out of memory */
	}
	(*sd->last)->prev=sd->top;
	(*sd->last)->next=NULL;
	(*sd->last)->lang=lang;
	(*sd->last)->nw=nw;
	(*sd->last)->key=key;
	sd->top=*sd->last;
	sd->last=&(*sd->last)->next;
	return 0;
}

static int pop(struct stack_data *sd, const struct spa_keyword **lang, int *nw,
		const struct spa_keyword **key) {
	struct parse_block_stack *oldtop=sd->top;

	if (sd->top==NULL) return 1; /* stack empty */
	*lang=sd->top->lang;
	*nw=sd->top->nw;
	*key=sd->top->key;
	sd->top=sd->top->prev;
	sd->last=(sd->top==NULL)?&sd->base:&sd->top->next;
	*sd->last=NULL;
	free(oldtop);
	return 0;
}

static void init(struct stack_data *sd) {
	sd->base=sd->top=NULL;
	sd->last=&sd->base;
}

static void flush_stack(struct stack_data *sd) {
	struct parse_block_stack *p1, *p2;

	p1=sd->base;
	while (p1!=NULL) {
		p2=p1->next;
		free(p1);
		p1=p2;
	}
}

#ifdef DEBUG
void spa_test_stack(void) {
	struct stack_data sd;
	const struct spa_keyword *l, *q;
	int i,j,k;
	
	init(&sd);
	for (j=0; j<3; j++) {
		for (i=0, l=NULL; i<5; i++,l++) {
			printf("push: ");
			if (push(&sd, l, i, NULL)) printf("out of memory\n");
			else printf("%p\n", l);
		}
		for (i=(j==1?1:0); i<=5; i++) {
			printf("pop: ");
			if (pop(&sd, &l, &k, &q)) printf("stack empty\n");
			else printf("%p\n", l);
		}
	}
}	
#endif

static int count_words(const struct spa_keyword *lang) {
	const struct spa_keyword *p;
	for (p=lang; p->keyword!=NULL; p++);
	return p-lang;
}

#define CALL_EXTERNAL(ptr,arg,error) \
if ((ptr)!=NULL) {\
	if (ptr arg) {\
		flush_stack(&sd);\
		return pv->err=error;\
	}\
}

int spa_parse(FILE *f, struct spa_vars *pv, void *context, const struct spa_keyword *lang) {
	int words, i, j;
	int mod=0;			/* state of the parsing machine */
	int new_line=1;		/* whether new line was reached before the current buffer
						   (previous buffer ended in \n) */
	int esc=0;			/* whether the current character was escaped with a '\' */
	int quote=0;		/* whether quotes are open */
	const struct spa_keyword *key;
	struct stack_data sd;
	char *buf, fbuf[BUF_SLICE], c;
	size_t buf_sz=BUF_SLICE;

	words=count_words(lang);

	if ((buf=(char *)malloc(BUF_SLICE))==NULL) return pv->err=SPA_OUT_OF_MEMORY;

	pv->line=1;
	pv->col=1;
	pv->word=NULL;
	init(&sd);
	while (fgets(fbuf, BUF_SLICE, f)!=NULL) {
		spa_debug(10, "Read line %d: '%s'\n", pv->line, fbuf);
		if (new_line && fbuf[0]=='#') {
			if (fbuf[strlen(fbuf)-1]!='\n') {
				if (flush_line(f, fbuf)) break;
			}
			pv->line++;
			pv->col=1;
			continue;
		}
		for (i=0; (c=fbuf[i])!='\0'; i++, pv->col++) {
			if (c=='\\') {
				esc=1;
				continue;
			}
			if (c=='\n' && esc) {
				esc=0;
				continue;
			}
			switch (mod) {
			case 0: /* expect keyword */
				if (c=='{' && !esc) {
					flush_stack(&sd);
					return pv->err=SPA_INVALID_CHARACTER;
				}
				if (c=='}' && !esc) {
					if (pop(&sd, &lang, &words, &key)) return pv->err=SPA_UNEXPECTED_EOB;
					CALL_EXTERNAL(key->eob,(context),SPA_BLOCK_FAILED);
					mod=2;
					break;
				}
				if (!is_blank(c)) {
					buf[0]=c;
					j=1;
					mod=1;
				}
				break;
			case 1: /* keyword */
				if ((c=='{' || c=='}') && !esc) {
					flush_stack(&sd);
					return pv->err=SPA_INVALID_CHARACTER;
				}
				if ((is_blank(c)||c==';') && !esc) {
					add_char('\0');
					if ((key=(struct spa_keyword*)find_keyword(buf, lang, words))==NULL) {
						pv->word=(char *)malloc(1+strlen(buf));
						strcpy(pv->word, buf);
						return pv->err=SPA_UNKNOWN_KEYWORD;
					}
					spa_debug(8, "parse: found keyword '%s'\n", buf);
					CALL_EXTERNAL(key->init,(context),SPA_KEYWORD_FAILED);
					mod=2;
					if (c==';') {
						CALL_EXTERNAL(key->end,(context),SPA_EOK_FAILED);
						mod=0;
					}
				} else {
					add_char(c);
				}
				break;		
			case 2: /* expect arg */
				if (c=='}' && !esc) {
					flush_stack(&sd);
					return pv->err=SPA_INVALID_CHARACTER;
				}
				if (c=='{' && !esc) {
					if (key->block==NULL) {
						flush_stack(&sd);
						return pv->err=SPA_UNEXPECTED_BLOCK;
					}
					if (push(&sd, lang, words, key)) {
						flush_stack(&sd);
						return pv->err=SPA_OUT_OF_MEMORY;
					}
					key->block(context, &lang);
					words=count_words(lang);
					mod=0;
					break;
				}
				if (c==';' && !esc) {
					/* don't need to call key->arg; since we are in mode 2 and ';' was reached, a white
					 * character had been reached before and key->arg had been called */
					CALL_EXTERNAL(key->end,(context),SPA_EOK_FAILED);
					mod=0;
					break;
				}
				if (!is_blank(c)) {
					buf[0]=c;
					j=1;
					mod=3;
				}
				break;
			case 3: /* arg */
				if ((c=='{' || c=='}') && !esc) {
					flush_stack(&sd);
					return pv->err=SPA_INVALID_CHARACTER;
				}
				if ((is_blank(c)||c==';') && !esc) {
					add_char('\0');
					if (key->arg==NULL) {
						flush_stack(&sd);
						return pv->err=SPA_UNEXPECTED_ARG;
					} else {
						CALL_EXTERNAL(key->arg,(context, buf),SPA_ARG_FAILED);
					}
					mod=2;
					if (c==';') {
						CALL_EXTERNAL(key->end,(context),SPA_EOK_FAILED);
						mod=0;
					}
				} else {
					add_char(c);
				}
			}
			esc=0;
		}
		if ((new_line=(fbuf[i-1]=='\n'))) {
			pv->line++;
			pv->col=1;
		}
	}
	return 0;
}

void spa_error(FILE *f, struct spa_vars *pv) {
	fprintf(f, "Parse error in line %d, col %d", pv->line, pv->col);
	switch (pv->err) {
	case SPA_UNKNOWN_KEYWORD:
		fprintf(f, ": unknown keyword '%s'", pv->word);
		break;
	case SPA_OUT_OF_MEMORY:
		fprintf(f, ": out of memory");
		break;
	}
	fprintf(f, ".\n");
	free(pv->word);
}
