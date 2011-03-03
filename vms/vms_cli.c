/* vms_cli.c -- interface to CLI$xxx routines for fetching command line components

   Copyright (C) 1991-1993, 2003, 2011 the Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */
 

/*
 * vms_cli.c - command line interface routines.
 *							Pat Rankin, Nov'89
 *	Routines called from vms_gawk.c for DCL parsing.
 */

#include "config.h"	/* in case we want to suppress 'const' &c */
#include "vms.h"
#ifndef _STRING_H
#include <string.h>
#endif

extern U_Long cli$present(const Dsc *);
extern U_Long cli$get_value(const Dsc *, Dsc *, short *);
extern U_Long cli$dcl_parse(const Dsc *, const void *, ...);
extern U_Long sys$cli(void *, ...);
extern U_Long sys$filescan(const Dsc *, void *, long *);
extern void  *lib$establish(U_Long (*handler)(void *, void *));
extern U_Long lib$sig_to_ret(void *, void *);	/* condition handler */

/* Cli_Present() - call CLI$PRESENT to determine whether a parameter or     */
/*		  qualifier is present on the [already parsed] command line */
U_Long
Cli_Present( const char *item )
{
    Dsc item_dsc;
    (void)lib$establish(lib$sig_to_ret);

    item_dsc.len = strlen(item_dsc.adr = (char *)item);
    return cli$present(&item_dsc);
}

/* Cli_Get_Value() - call CLI$GET_VALUE to retreive the value of a */
/*		    parameter or qualifier from the command line   */
U_Long
Cli_Get_Value( const char *item, char *result, int size )
{
    Dsc item_dsc, res_dsc;
    U_Long sts;
    short len = 0;
    (void)lib$establish(lib$sig_to_ret);

    item_dsc.len = strlen(item_dsc.adr = (char *)item);
    res_dsc.len = size,  res_dsc.adr = result;
    sts = cli$get_value(&item_dsc, &res_dsc, &len);
    result[len] = '\0';
    return sts;
}

/* Cli_Parse_Command() - use the $CLI system service (undocumented) to	 */
/*			retreive the actual command line (which might be */
/*			"run prog" or "mcr prog [params]") and then call */
/*			CLI$DCL_PARSE to parse it using specified tables */
U_Long
Cli_Parse_Command( const void *cmd_tables, const char *cmd_verb )
{
    struct { short len, code; void *adr; } fscn[2];
    struct { char rqtype, rqindx, rqflags, rqstat; unsigned :32;
	     Dsc rdesc; unsigned :32; unsigned :32; unsigned :32; } cmd;
    U_Long sts;
    int    ltmp;
    char   longbuf[8200];
    (void)lib$establish(lib$sig_to_ret);

    memset(&cmd, 0, sizeof cmd);
    cmd.rqtype = CLI$K_GETCMD;		/* command line minus the verb */
    sts = sys$cli(&cmd, (void *)0, (void *)0);	/* get actual command line */

    if (vmswork(sts)) {		/* ok => cli available & verb wasn't "RUN" */
	/* invoked via symbol => have command line (which might be empty) */
	/*    [might also be invoked via mcr or dcl; that's ok]		  */
	if (cmd.rqstat == CLI$K_VERB_MCR) {
	    /* need to strip image name from MCR invocation   */
	    memset(fscn, 0, sizeof fscn);
	    fscn[0].code = FSCN$_FILESPEC;	/* full file specification */
	    (void)sys$filescan(&cmd.rdesc, fscn, (long *)0);
	    cmd.rdesc.len -= fscn[0].len;	/* shrink size */
	    cmd.rdesc.adr += fscn[0].len;	/* advance ptr */
	}
	/* prepend verb and then parse the command line */
	strcat(strcpy(longbuf, cmd_verb), " "),  ltmp = strlen(longbuf);
	if (cmd.rdesc.len + ltmp > sizeof longbuf)
	    cmd.rdesc.len = sizeof longbuf - ltmp;
	strncpy(&longbuf[ltmp], cmd.rdesc.adr, cmd.rdesc.len);
	cmd.rdesc.len += ltmp,	cmd.rdesc.adr = longbuf;
	sts = cli$dcl_parse(&cmd.rdesc, cmd_tables);
    }

    return sts;
}
