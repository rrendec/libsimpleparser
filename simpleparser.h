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

#ifndef _LIBSIMPLEPARSER_H
#define _LIBSIMPLEPARSER_H

#include <stdio.h>

struct spa_keyword {
	const char 	*keyword;
	int			(*init)		(void *);
	int			(*arg)		(void *, char *);
	int			(*end)		(void *);
	int			(*block)	(void *, const struct spa_keyword **);
	int			(*eob)		(void *);
};

/* init		->	called immediately after a keyword is recognized;
 * 				is passed a pointer to the context structure that parse was called with;
 * arg		->	called after each argument;
 * 				is passed a pointer to the context structure and a pointer to the argument string;
 * 				the pointer to the argument string points to a parse internal buffer, so one must
 * 				strcpy the contents of the argument in order to keep it;
 * end		->	called when ';' is reached;
 * 				is passed a pointer to the context structure;
 * block	->	called when a new block begins ('{' is reached);
 * 				is passed a pointer to the context structure;
 * 				it must provide a pointer to the language definition to use inside the block;
 * 				the pointer to the language definition must be left unchanged if an error occurs;
 * eob		->	called when a block ends;
 * 				is passed a pointer to the context structure;
 */

struct spa_vars {
	int			line, col;		/* current line and column in input */
	char		*word;			/* a string associated with the error */
	int			err;			/* error code */
};

extern int spa_parse(FILE *, struct spa_vars *, void *, const struct spa_keyword *);
extern void spa_error(FILE *, struct spa_vars *);

/* errors */
#define SPA_UNKNOWN_KEYWORD				1
#define SPA_OUT_OF_MEMORY				2
#define SPA_INVALID_CHARACTER			3
#define SPA_UNEXPECTED_EOB				4
#define SPA_UNEXPECTED_BLOCK			5
#define SPA_UNEXPECTED_ARG				6
#define SPA_BLOCK_FAILED				7
#define SPA_KEYWORD_FAILED				8
#define SPA_EOK_FAILED					9
#define SPA_ARG_FAILED					10

#endif
