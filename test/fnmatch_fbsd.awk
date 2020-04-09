# fnmatch regression tests from:

# 
# Copyright (c) 2010 Jilles Tjoelker
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
#__FBSDID("$FreeBSD: head/tools/regression/lib/libc/gen/test-fnmatch.c 254231 2013-08-11 21:54:20Z jilles $");

@load "fnmatch"

BEGIN {
	printf(fnmatch("",                      "",            0))
	printf(fnmatch("a",                     "a",           0))
	printf(fnmatch("a",                     "b",           0))
	printf(fnmatch("a",                     "A",           0))
	printf(fnmatch("*",                     "a",           0))
	printf(fnmatch("*",                     "aa",          0))
	printf(fnmatch("*a",                    "a",           0))
	printf(fnmatch("*a",                    "b",           0))
	printf(fnmatch("*a*",                   "b",           0))
	printf(fnmatch("*a*b*",                 "ab",          0))
	printf(fnmatch("*a*b*",                 "qaqbq",       0))
	printf(fnmatch("*a*bb*",                "qaqbqbbq",    0))
	printf(fnmatch("*a*bc*",                "qaqbqbcq",    0))
	printf(fnmatch("*a*bb*",                "qaqbqbb",     0))
	printf(fnmatch("*a*bc*",                "qaqbqbc",     0))
	printf(fnmatch("*a*bb",                 "qaqbqbb",     0))
	printf(fnmatch("*a*bc",                 "qaqbqbc",     0))
	printf(fnmatch("*a*bb",                 "qaqbqbbq",    0))
	printf(fnmatch("*a*bc",                 "qaqbqbcq",    0))
	printf(fnmatch("*a*a*a*a*a*a*a*a*a*a*", "aaaaaaaaa",   0))
	printf(fnmatch("*a*a*a*a*a*a*a*a*a*a*", "aaaaaaaaaa",  0))
	printf(fnmatch("*a*a*a*a*a*a*a*a*a*a*", "aaaaaaaaaaa", 0))
	printf(fnmatch(".*.*.*.*.*.*.*.*.*.*",  ".........",   0))
	printf(fnmatch(".*.*.*.*.*.*.*.*.*.*",  "..........",  0))
	printf(fnmatch(".*.*.*.*.*.*.*.*.*.*",  "...........", 0))
	printf(fnmatch("*?*?*?*?*?*?*?*?*?*?*", "123456789",   0))
	printf(fnmatch("??????????*",           "123456789",   0))
	printf(fnmatch("*??????????",           "123456789",   0))
	printf(fnmatch("*?*?*?*?*?*?*?*?*?*?*", "1234567890",  0))
	printf(fnmatch("??????????*",           "1234567890",  0))
	printf(fnmatch("*??????????",           "1234567890",  0))
	printf(fnmatch("*?*?*?*?*?*?*?*?*?*?*", "12345678901", 0))
	printf(fnmatch("??????????*",           "12345678901", 0))
	printf(fnmatch("*??????????",           "12345678901", 0))
	printf("\n") # 0011000110000000011100100111000000

	printf(fnmatch("[x]",                   "x",           0))
	printf(fnmatch("[*]",                   "*",           0))
	printf(fnmatch("[?]",                   "?",           0))
	printf(fnmatch("[",                     "[",           0))
	printf(fnmatch("[[]",                   "[",           0))
	printf(fnmatch("[[]",                   "x",           0))
	printf(fnmatch("[*]",                   "",            0))
	printf(fnmatch("[*]",                   "x",           0))
	printf(fnmatch("[?]",                   "x",           0))
	printf(fnmatch("*[*]*",                 "foo*foo",     0))
	printf(fnmatch("*[*]*",                 "foo",         0))
	printf(fnmatch("[0-9]",                 "0",           0))
	printf(fnmatch("[0-9]",                 "5",           0))
	printf(fnmatch("[0-9]",                 "9",           0))
	printf(fnmatch("[0-9]",                 "/",           0))
	printf(fnmatch("[0-9]",                 ":",           0))
	printf(fnmatch("[0-9]",                 "*",           0))
	printf(fnmatch("[!0-9]",                "0",           0))
	printf(fnmatch("[!0-9]",                "5",           0))
	printf(fnmatch("[!0-9]",                "9",           0))
	printf(fnmatch("[!0-9]",                "/",           0))
	printf(fnmatch("[!0-9]",                ":",           0))
	printf(fnmatch("[!0-9]",                "*",           0))
	printf(fnmatch("*[0-9]",                "a0",          0))
	printf(fnmatch("*[0-9]",                "a5",          0))
	printf(fnmatch("*[0-9]",                "a9",          0))
	printf(fnmatch("*[0-9]",                "a/",          0))
	printf(fnmatch("*[0-9]",                "a:",          0))
	printf(fnmatch("*[0-9]",                "a*",          0))
	printf(fnmatch("*[!0-9]",               "a0",          0))
	printf(fnmatch("*[!0-9]",               "a5",          0))
	printf(fnmatch("*[!0-9]",               "a9",          0))
	printf(fnmatch("*[!0-9]",               "a/",          0))
	printf(fnmatch("*[!0-9]",               "a:",          0))
	printf(fnmatch("*[!0-9]",               "a*",          0))
	printf(fnmatch("*[0-9]",                "a00",         0))
	printf(fnmatch("*[0-9]",                "a55",         0))
	printf(fnmatch("*[0-9]",                "a99",         0))
	printf(fnmatch("*[0-9]",                "a0a0",        0))
	printf(fnmatch("*[0-9]",                "a5a5",        0))
	printf(fnmatch("*[0-9]",                "a9a9",        0))
	printf("\n") # 00000111101000111111000000111111000000000

	printf(fnmatch("\\*",                   "*",           0))
	printf(fnmatch("\\?",                   "?",           0))
	printf(fnmatch("\\[x]",                 "[x]",         0))
	printf(fnmatch("\\[",                   "[",           0))
	printf(fnmatch("\\\\",                  "\\",          0))
	printf(fnmatch("*\\**",                 "foo*foo",     0))
	printf(fnmatch("*\\**",                 "foo",         0))
	printf(fnmatch("*\\\\*",                "foo\\foo",    0))
	printf(fnmatch("*\\\\*",                "foo",         0))
	printf(fnmatch("\\(",                   "(",           0))
	printf(fnmatch("\\a",                   "a",           0))
	printf(fnmatch("\\*",                   "a",           0))
	printf(fnmatch("\\?",                   "a",           0))
	printf(fnmatch("\\*",                   "\\*",         0))
	printf(fnmatch("\\?",                   "\\?",         0))
	printf(fnmatch("\\[x]",                 "\\[x]",       0))
	printf(fnmatch("\\[x]",                 "\\x",         0))
	printf(fnmatch("\\[",                   "\\[",         0))
	printf(fnmatch("\\(",                   "\\(",         0))
	printf(fnmatch("\\a",                   "\\a",         0))
	printf(fnmatch("\\",                    "\\",          0))
	# unlike freebsd, this is _not_ matched
	printf(fnmatch("\\",                    "",            0))
	printf(fnmatch("\\*",                   "\\*",         FNM["NOESCAPE"]))
	printf(fnmatch("\\?",                   "\\?",         FNM["NOESCAPE"]))
	printf(fnmatch("\\",                    "\\",          FNM["NOESCAPE"]))
	printf(fnmatch("\\\\",                  "\\",          FNM["NOESCAPE"]))
	printf(fnmatch("\\\\",                  "\\\\",        FNM["NOESCAPE"]))
	printf(fnmatch("*\\*",                  "foo\\foo",    FNM["NOESCAPE"]))
	printf(fnmatch("*\\*",                  "foo",         FNM["NOESCAPE"]))
	printf("\n") # 00000010100111111111110001001

	printf(fnmatch("*",     ".",       FNM["PERIOD"]))
	printf(fnmatch("?",     ".",       FNM["PERIOD"]))
	printf(fnmatch(".*",    ".",       0))
	printf(fnmatch(".*",    "..",      0))
	printf(fnmatch(".*",    ".a",      0))
	printf(fnmatch("[0-9]", ".",       FNM["PERIOD"]))
	printf(fnmatch("a*",    "a.",      0))
	printf(fnmatch("a/a",   "a/a",     FNM["PATHNAME"]))
	printf(fnmatch("a/*",   "a/a",     FNM["PATHNAME"]))
	printf(fnmatch("*/a",   "a/a",     FNM["PATHNAME"]))
	printf(fnmatch("*/*",   "a/a",     FNM["PATHNAME"]))
	printf(fnmatch("a*b/*", "abbb/x",  FNM["PATHNAME"]))
	printf(fnmatch("a*b/*", "abbb/.x", FNM["PATHNAME"]))
	printf(fnmatch("*",     "a/a",     FNM["PATHNAME"]))
	printf(fnmatch("*/*",   "a/a/a",   FNM["PATHNAME"]))
	printf(fnmatch("b/*",   "b/.x",    or(FNM["PATHNAME"], FNM["PERIOD"])))
	printf(fnmatch("b*/*",  "a/.x",    or(FNM["PATHNAME"], FNM["PERIOD"])))
	printf(fnmatch("b/.*",  "b/.x",    or(FNM["PATHNAME"], FNM["PERIOD"])))
	printf(fnmatch("b*/.*", "b/.x",    or(FNM["PATHNAME"], FNM["PERIOD"])))
	printf(fnmatch("a",     "A",       FNM["CASEFOLD"]))
	printf(fnmatch("A",     "a",       FNM["CASEFOLD"]))
	printf(fnmatch("[a]",   "A",       FNM["CASEFOLD"]))
	printf(fnmatch("[A]",   "a",       FNM["CASEFOLD"]))
	printf(fnmatch("a",     "b",       FNM["CASEFOLD"]))
	printf(fnmatch("a",     "a/b",     FNM["PATHNAME"]))
	printf(fnmatch("*",     "a/b",     FNM["PATHNAME"]))
	printf(fnmatch("*b",    "a/b",     FNM["PATHNAME"]))
	printf(fnmatch("a",     "a/b",     or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
	printf(fnmatch("*",     "a/b",     or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
	printf(fnmatch("*",     ".a/b",    or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
	printf(fnmatch("*a",    ".a/b",    or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
	printf(fnmatch("*",     ".a/b",    or(or(FNM["PATHNAME"], FNM["PERIOD"]), FNM["LEADING_DIR"])))
	printf(fnmatch("*a",    ".a/b",    or(or(FNM["PATHNAME"], FNM["PERIOD"]), FNM["LEADING_DIR"])))
	printf(fnmatch("a*b/*", "abbb/.x", or(FNM["PATHNAME"], FNM["PERIOD"])))
	printf("\n") # 1100010000000111100000011110000111
}
