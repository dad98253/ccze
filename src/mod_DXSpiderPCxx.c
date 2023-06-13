/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_DXSpiderPCxx.c -- DXSpiderPCxx-based on related colorizers for CCZE
 * Copyright (C) 2002, 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
 * Modifications to incorporate DXSpider filtering:
 * Copyright (C) 2023 John Kuras <w7og@yahoo.com>
 *
 * This file is part of ccze.
 *
 * ccze is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ccze is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * The modifications to this file made by John Kuras are licensed without
 * hinderance. I.e., do what you want with them.
 */

#include <ccze.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ccze-compat.h"

static void ccze_DXSpiderPCxx_setup (void);
static void ccze_DXSpiderPCxx_shutdown (void);
static int ccze_DXSpiderPCxx_handle (const char *str, size_t length, char **rest);

static pcre *reg_DXSpiderPCxx;
static pcre_extra *hints_DXSpiderPCxx;

int isNumber_dx(char *text) {
    int j;
    j = strlen(text);
    while(j--)
    {
        if(isdigit(text[j])) continue;
        if( text[j] == '.' ) continue;
        return 0;
    }
    return 1;
}

static char *
ccze_DXSpiderPCxx_process (const char *str, int *offsets, int match)
{
  char *word = NULL, *toret = NULL;
  char *msg2 = NULL, *num = NULL, *msg = NULL, *hops = NULL, *send = NULL;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&num);
  pcre_get_substring (str, offsets, match, 2, (const char **)&msg);
  pcre_get_substring (str, offsets, match, 3, (const char **)&hops);
  pcre_get_substring (str, offsets, match, 4, (const char **)&send);
  if ( num[0] == '9' ) {
  	ccze_addstr (CCZE_COLOR_DXSPC9XNUM, "PC");
  	ccze_addstr (CCZE_COLOR_DXSPC9XNUM, num);  
  } else {
  	ccze_addstr (CCZE_COLOR_DXSPCNUM, "PC");
  	ccze_addstr (CCZE_COLOR_DXSPCNUM, num);
  }
  msg2 = xstrdup (msg);
  word = xstrdup (ccze_strbrk (msg2, '^'));
  do {
      if (word != NULL) {
      	  ccze_addstr (CCZE_COLOR_DXSPCDELIM, "^");
		  if ( isNumber_dx(word) ) {
		  	ccze_addstr (CCZE_COLOR_DXSPCMESSAGENUMS, word);
		  } else {
		  	ccze_addstr (CCZE_COLOR_DXSPCMESSAGEWORDS, word);
		  }
		  free(word);
		  word = NULL;
	  } else {
	  	  ccze_addstr (CCZE_COLOR_DXSPCDELIM, "^");
	  }
  } while ((word = xstrdup (ccze_strbrk (NULL, '^'))) != NULL);
  if ( word != NULL ) free(word);
  free (msg2);
  ccze_addstr (CCZE_COLOR_DXSPCDELIM, "^");
  ccze_addstr (CCZE_COLOR_DXSPCHOPS, "H");
  ccze_addstr (CCZE_COLOR_DXSPCHOPS, hops);
  ccze_addstr (CCZE_COLOR_DXSPCDELIM, "^");
  if ( send != NULL ) {
  	  toret = xstrdup (send);
  	  return(toret);
  }
  
  return NULL;
}

static void
ccze_DXSpiderPCxx_setup (void)
{
  const char *error;
  int errptr;
  reg_DXSpiderPCxx = pcre_compile
    ("^PC(\\d{2})\\^(.*?)\\^H(\\d{2})\\^(\\S*)", 0, &error,
     &errptr, NULL);
  hints_DXSpiderPCxx = pcre_study (reg_DXSpiderPCxx, 0, &error);
}

static void
ccze_DXSpiderPCxx_shutdown (void)
{
  free (reg_DXSpiderPCxx);
  free (hints_DXSpiderPCxx);
}

static int
ccze_DXSpiderPCxx_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];  
  
  if ( (match = pcre_exec (reg_DXSpiderPCxx, hints_DXSpiderPCxx, str, length, 0, 0, offsets, 99)) >= 0) {
      if (rest) {
		*rest = ccze_DXSpiderPCxx_process (str, offsets, match);
      } else {
		ccze_DXSpiderPCxx_process (str, offsets, match);
      }
      return 1;
  }

  return 0;
}

CCZE_DEFINE_PLUGIN (DXSpiderPCxx, PARTIAL, "Coloriser DXSpider PCxx messages.");
