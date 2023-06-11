/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_DXSpider.c -- DXSpider-related colorizers for CCZE
 * Copyright (C) 2002, 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
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
 */

#include <ccze.h>
#include <string.h>
#include <stdlib.h>

#include "ccze-compat.h"

static void ccze_DXSpider_setup (void);
static void ccze_DXSpider_shutdown (void);
static int ccze_DXSpider_handle (const char *str, size_t length, char **rest);

static pcre *reg_DXSpider_chan;
static pcre_extra *hints_DXSpider_chan;
static pcre *reg_DXSpider_chanerr;
static pcre_extra *hints_DXSpider_chanerr;
static pcre *reg_DXSpider_progress;
static pcre_extra *hints_DXSpider_progress;

static char *
ccze_DXSpider_process_chan (const char *str, int *offsets, int match)
{
  char *date = NULL, *chan = NULL,  *direction = NULL, *mtype = NULL, *connection = NULL;
  char *protocolMnumber = NULL, *message = NULL;
  char *toret;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
//  pcre_get_substring (str, offsets, match, 2, (const char **)&chan);
  pcre_get_substring (str, offsets, match, 2, (const char **)&direction);
  pcre_get_substring (str, offsets, match, 3, (const char **)&mtype);
  pcre_get_substring (str, offsets, match, 4, (const char **)&connection);	/* call */
  pcre_get_substring (str, offsets, match, 5, (const char **)&protocolMnumber);	/* [PC]## */
  pcre_get_substring (str, offsets, match, 6, (const char **)&message);  

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_PIDB, "(");
  ccze_addstr (CCZE_COLOR_HOST, "chan");
  ccze_addstr (CCZE_COLOR_PIDB, ")");
  ccze_space ();
  
  ccze_addstr (CCZE_COLOR_PID, direction);
  ccze_space ();
  
  ccze_addstr (CCZE_COLOR_PROC, mtype);
  ccze_space ();
  
  ccze_addstr (CCZE_COLOR_HOST, connection);
  ccze_space ();
    
//  ccze_addstr (CCZE_COLOR_PID, "PC");
  ccze_addstr (CCZE_COLOR_PID, protocolMnumber);
  ccze_space ();

  toret = strdup (message);

  free (date);
  free (chan);
  free (direction);
  free (mtype);
  free (connection);
  free (protocolMnumber);
  free (message);

  return toret;
}

static char *
ccze_DXSpider_process_chanerr (const char *str, int *offsets, int match)
{
  char *date = NULL, *chanerr = NULL,  *errortype = NULL;
  char *message = NULL;
  char *toret;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
//  pcre_get_substring (str, offsets, match, 2, (const char **)&chanerr);
  pcre_get_substring (str, offsets, match, 2, (const char **)&errortype);
  pcre_get_substring (str, offsets, match, 3, (const char **)&message);
  

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_HOST, "(");
  ccze_addstr (CCZE_COLOR_PIDB, "chanerr");
  ccze_addstr (CCZE_COLOR_HOST, ")");
  ccze_space ();
  
  ccze_addstr (CCZE_COLOR_PID, errortype);
  ccze_addstr (CCZE_COLOR_HOST, ":");
  ccze_space ();


  toret = strdup (message);

  free (date);
  free (chanerr);
  free (errortype);
  free (message);

  return toret;
}

static char *
ccze_DXSpider_process_progress (const char *str, int *offsets, int match)
{
  char *date = NULL, *chanerr = NULL,  *errortype = NULL;
  char *message = NULL;
  char *toret;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
//  pcre_get_substring (str, offsets, match, 2, (const char **)&chanerr);
  pcre_get_substring (str, offsets, match, 2, (const char **)&errortype);
  pcre_get_substring (str, offsets, match, 3, (const char **)&message);
  

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_HOST, "(");
  ccze_addstr (CCZE_COLOR_PIDB, "progress");
  ccze_addstr (CCZE_COLOR_HOST, ")");
  ccze_space ();
  
  ccze_addstr (CCZE_COLOR_PID, errortype);
  ccze_addstr (CCZE_COLOR_HOST, ":");
  ccze_space ();


  toret = strdup (message);

  free (date);
  free (chanerr);
  free (errortype);
  free (message);

  return toret;
}

static void
ccze_DXSpider_setup (void)
{
  const char *error;
  int errptr;

  reg_DXSpider_chan = pcre_compile ("^(\\d{1,2}:\\d\\d:\\d\\d)"
			     "\\s\\(chan\\)\\s(->|<-)\\s([IDX])\\s(\\S+)\\s(\\S+)[\\s^](.*)$", 0, &error,
			     &errptr, NULL);
  hints_DXSpider_chan = pcre_study (reg_DXSpider_chan, 0, &error);
  
  reg_DXSpider_chanerr = pcre_compile ("^(\\d{1,2}:\\d\\d:\\d\\d)"
			     "\\s\\(chanerr\\)\\s(.*):\\s(.*)$", 0, &error,
			     &errptr, NULL);
  hints_DXSpider_chanerr = pcre_study (reg_DXSpider_chanerr, 0, &error);
  
  reg_DXSpider_progress = pcre_compile ("^(\\d{1,2}:\\d\\d:\\d\\d)"
			     "\\s\\(progress\\)\\s(.*):\\s(.*)$", 0, &error,
			     &errptr, NULL);
  hints_DXSpider_progress = pcre_study (reg_DXSpider_progress, 0, &error);
  
}

static void
ccze_DXSpider_shutdown (void)
{
  free (reg_DXSpider_chan);
  free (hints_DXSpider_chan);
  free (reg_DXSpider_chanerr);
  free (hints_DXSpider_chanerr);
  free (reg_DXSpider_progress);
  free (hints_DXSpider_progress);
}

static int
ccze_DXSpider_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_DXSpider_chan, hints_DXSpider_chan, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_DXSpider_process_chan (str, offsets, match);
      return 1;
    }
    
  if ((match = pcre_exec (reg_DXSpider_chanerr, hints_DXSpider_chanerr, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_DXSpider_process_chanerr (str, offsets, match);
      return 1;
    }
    
  if ((match = pcre_exec (reg_DXSpider_progress, hints_DXSpider_progress, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_DXSpider_process_progress (str, offsets, match);
      return 1;
    }

  return 0;
}

CCZE_DEFINE_PLUGIN (DXSpider, FULL, "Coloriser for DXSpider debug logs.");
