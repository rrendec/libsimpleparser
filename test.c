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

#include <stdio.h>
#include "simpleparser.h"

extern void spa_test_stack(void);

struct context {
	int n;
};

#define C(x) ((struct context *)(x))

int boil_a(void *c, char *s) {
	printf("Keyword 'boil' - arg '%s'\n", s);
	return 0;
}

int add_a(void *c, char *s) {
	printf("Keyword 'add' - arg '%s'\n", s);
	return 0;
}

struct spa_keyword lang2[]={
	"add",		NULL,		add_a,		NULL,		NULL,		NULL,
	"boil",		NULL,		boil_a,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL
};

int make_s(void *c) {
	C(c)->n=0;
	printf("Keyword 'make' found\n");
	return 0;
}

int make_a(void *c, char *s) {
	printf("Keyword 'make', arg %d: '%s'\n", C(c)->n++, s);
	return 0;
}

int make_e(void *c) {
	printf("Keyword 'make' - no more arguments\n");
	return 0;
}

int make_b(void *c, const struct spa_keyword **lang) {
	printf("Keyword 'make' - found block\n");
	*lang=lang2;
	return 0;
}

int make_n(void *c) {
	printf("Keyword 'make' - end of block\n");
	return 0;
}

const struct spa_keyword lang1[]={
	"make",		make_s,		make_a,		make_e,		make_b,		make_n,
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL
};

int main(void) {
	int err;
	struct spa_vars pv={0, 0};
	struct context ctx;

#ifdef DEBUG
	printf("Testing stack functions\n");
	spa_test_stack();
#endif
	if (err=spa_parse(stdin, &pv, &ctx, lang1)) {
		spa_error(stderr, &pv);
		return 1;
	}
	return 0;
}
