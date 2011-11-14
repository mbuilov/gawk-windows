/* wcmisc.c - replace wcXXXX routines
   Copyright (C) 2011 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA */

#if !defined(HAVE_WCTYPE) || !defined(HAVE_ISWCTYPE)
static const char *classes[] = {
	"<dummy>",
	"alnum",
	"alpha",
	"blank",
	"cntrl",
	"digit",
	"graph",
	"lower",
	"print",
	"punct",
	"space",
	"upper",
	"xdigit",
	NULL
};
#endif

#ifndef HAVE_ISWCTYPE
static int is_blank (int c)
{
   return (c == ' ' || c == '\t');
}
#endif

#ifndef HAVE_WCTYPE
wctype_t wctype(const char *name)
{
	int i;

	for (i = 1; classes[i] != NULL; i++)
		if (strcmp(name, classes[i]) == 0)
			return i;

	return 0;
}
#endif

#ifndef HAVE_ISWCTYPE
int iswctype(wint_t wc, wctype_t desc)
{
	int j = sizeof(classes) / sizeof(classes[0]);

	if (desc >= j || desc == 0)
		return 0;

	switch (desc) {
	case 1:		return isalnum(wc);
	case 2:		return isalpha(wc);
	case 3:		return is_blank(wc);
	case 4:		return iscntrl(wc);
	case 5:		return isdigit(wc);
	case 6:		return isgraph(wc);
	case 7:		return islower(wc);
	case 8:		return isprint(wc);
	case 9:		return ispunct(wc);
	case 10:	return isspace(wc);
	case 11:	return isupper(wc);
	case 12:	return isxdigit(wc);
	default:	return 0;
	}
}
#endif

#ifndef HAVE_WCSCOLL
int wcscoll(const wchar_t *ws1, const wchar_t *ws2)
{
	size_t i;

	for (i = 0; ws1[i] != 0 && ws2[i] != 0; i++) {
		if (ws1[i] < ws2[i])
			return -1;
		else if (ws1[i] > ws2[i])
			return 1;
	}

	return (ws1[i] - ws2[i]);
}
#endif

/*wcmisc.c*/
