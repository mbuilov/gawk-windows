# Parts of this file were derived from glibc-2.31/posix/tst-fnmatch.input

## Tests for fnmatch.
## Copyright (C) 2000-2020 Free Software Foundation, Inc.
## This file is part of the GNU C Library.
## Contributes by Ulrich Drepper <drepper@redhat.com>.
##
#
## The GNU C Library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
#
## The GNU C Library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## Lesser General Public License for more details.
#
## You should have received a copy of the GNU Lesser General Public
## License along with the GNU C Library; if not, see
## <https://www.gnu.org/licenses/>.

@load "fnmatch"

BEGIN {
  # B.6 004(C)
  printf(fnmatch("!#%+,-./01234567889", "!#%+,-./01234567889", 0))
  printf(fnmatch(":;=@ABCDEFGHIJKLMNO", ":;=@ABCDEFGHIJKLMNO", 0))
  printf(fnmatch("PQRSTUVWXYZ]abcdefg", "PQRSTUVWXYZ]abcdefg", 0))
  printf(fnmatch("hijklmnopqrstuvwxyz", "hijklmnopqrstuvwxyz", 0))
  printf(fnmatch("^_{}~"              , "^_{}~"              , 0))
  printf("\n") # 00000

  # B.6 005(C)
  printf(fnmatch("\\\"\\$\\&\\'\\(\\)", "\"$&'()", 0))
  printf(fnmatch("\\*\\?\\[\\\\\\`\\|", "*?[\\`|", 0))
  printf(fnmatch("\\<\\>"             , "<>"     , 0))
  printf("\n") # 000

  # B.6 006(C)
  printf(fnmatch("[?*[][?*[][?*[]", "?*[", 0))
  printf(fnmatch("?/b"            , "a/b", 0))
  printf("\n") # 00

  # B.6 007(C)
  printf(fnmatch("a?b", "a/b" , 0))
  printf(fnmatch("a/?", "a/b" , 0))
  printf(fnmatch("?/b", "aa/b", 0))
  printf(fnmatch("a?b", "aa/b", 0))
  printf(fnmatch("a/?", "a/bb", 0))
  printf("\n") # 00111

  # B.6 009(C)
  printf(fnmatch("[abc]"  , "abc", 0))
  printf(fnmatch("[abc]"  , "x"  , 0))
  printf(fnmatch("[abc]"  , "a"  , 0))
  printf(fnmatch("[[abc]" , "["  , 0))
  printf(fnmatch("[][abc]", "a"  , 0))
  printf(fnmatch("[]a]]"  , "a]" , 0))
  printf("\n") # 110000

  # B.6 010(C)
  printf(fnmatch("[!abc]", "xyz", 0))
  printf(fnmatch("[!abc]", "x"  , 0))
  printf(fnmatch("[!abc]", "a"  , 0))
  printf("\n") # 101

  # B.6 011(C)
  printf(fnmatch("[][abc]", "]"    , 0))
  printf(fnmatch("[][abc]", "abc]" , 0))
  printf(fnmatch("[][]abc", "[]abc", 0))
  printf(fnmatch("[!]]"   , "]"    , 0))
  printf(fnmatch("[!]a]"  , "aa]"  , 0))
  printf(fnmatch("[!a]"   , "]"    , 0))
  printf(fnmatch("[!a]]"  , "]]"   , 0))
  printf("\n") # 0111100

  # B.6 012(C)
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[.a.]]"         , "a", 0))
  printf(fnmatch("[[.-.]]"         , "-", 0))
  printf(fnmatch("[[.-.][.].]]"    , "-", 0))
  printf(fnmatch("[[.].][.-.]]"    , "-", 0))
  printf(fnmatch("[[.-.][=u=]]"    , "-", 0))
  printf(fnmatch("[[.-.][:alpha:]]", "-", 0))
  printf(fnmatch("[![.a.]]"        , "a", 0))
  printf("\n") # 0000001
}

  # B.6 013(C)
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[.b.]]"      , "a", 0))
  printf(fnmatch("[[.b.][.c.]]" , "a", 0))
  printf(fnmatch("[[.b.][=b=]]" , "a", 0))
  printf("\n") # 111
}

  # B.6 015(C)
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[=a=]]"         , "a", 0))
  printf(fnmatch("[[=a=]b]"        , "b", 0))
  printf(fnmatch("[[=a=][=b=]]"    , "b", 0))
  printf(fnmatch("[[=a=][=b=]]"    , "a", 0))
  printf(fnmatch("[[=a=][.b.]]"    , "a", 0))
  printf(fnmatch("[[=a=][:digit:]]", "a", 0))
  printf("\n") # 000000
}

  # B.6 016(C)
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[=a=]b]"        , "=", 0))
  printf(fnmatch("[[=a=]b]"        , "]", 0))
  printf(fnmatch("[[=b=][=c=]]"    , "a", 0))
  printf(fnmatch("[[=b=][.].]]"    , "a", 0))
  printf(fnmatch("[[=b=][:digit:]]", "a", 0))
  printf("\n") # 11111
}

  # B.6 017(C)
  printf(fnmatch("[[:alnum:]]"         , "a"  , 0))
  printf(fnmatch("[![:alnum:]]"        , "a"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "-"  , 0))
  printf(fnmatch("[[:alnum:]]a"        , "a]a", 0))
  printf(fnmatch("[[:alnum:]-]"        , "-"  , 0))
  printf(fnmatch("[[:alnum:]]a"        , "aa" , 0))
  printf(fnmatch("[![:alnum:]]"        , "-"  , 0))
  printf(fnmatch("[!][:alnum:]]"       , "]"  , 0))
  printf(fnmatch("[![:alnum:][]"       , "["  , 0))
  printf("\n") # 011100011
  printf(fnmatch("[[:alnum:]]"         , "a"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "b"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "c"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "d"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "e"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "f"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "g"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "h"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "i"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "j"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "k"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "l"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "m"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "n"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "o"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "p"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "q"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "r"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "s"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "t"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "u"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "v"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "w"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "x"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "y"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "z"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "A"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "B"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "C"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "D"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "E"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "F"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "G"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "H"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "I"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "J"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "K"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "L"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "M"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "N"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "O"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "P"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "Q"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "R"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "S"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "T"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "U"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "V"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "W"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "X"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "Y"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "Z"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "0"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "1"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "2"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "3"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "4"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "5"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "6"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "7"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "8"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "9"  , 0))
  printf("\n") # 00000000000000000000000000000000000000000000000000000000000000
  printf(fnmatch("[[:alnum:]]"         , "!"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "#"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "%"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "+"  , 0))
  printf(fnmatch("[[:alnum:]]"         , ","  , 0))
  printf(fnmatch("[[:alnum:]]"         , "-"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "."  , 0))
  printf(fnmatch("[[:alnum:]]"         , "/"  , 0))
  printf(fnmatch("[[:alnum:]]"         , ":"  , 0))
  printf(fnmatch("[[:alnum:]]"         , ";"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "="  , 0))
  printf(fnmatch("[[:alnum:]]"         , "@"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "["  , 0))
  printf(fnmatch("[[:alnum:]]"         , "\\" , 0))
  printf(fnmatch("[[:alnum:]]"         , "]"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "^"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "_"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "{"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "}"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "~"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "\"" , 0))
  printf(fnmatch("[[:alnum:]]"         , "$"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "&"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "'"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "("  , 0))
  printf(fnmatch("[[:alnum:]]"         , ")"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "*"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "?"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "`"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "|"  , 0))
  printf(fnmatch("[[:alnum:]]"         , "<"  , 0))
  printf(fnmatch("[[:alnum:]]"         , ">"  , 0))
  printf("\n") # 11111111111111111111111111111111
  printf(fnmatch("[[:cntrl:]]"         , "\t" , 0))
  printf(fnmatch("[[:cntrl:]]"         , "t"  , 0))
  printf(fnmatch("[[:lower:]]"         , "t"  , 0))
  printf(fnmatch("[[:lower:]]"         , "\t" , 0))
  printf(fnmatch("[[:lower:]]"         , "T"  , 0))
  printf(fnmatch("[[:space:]]"         , "\t" , 0))
  printf(fnmatch("[[:space:]]"         , "t"  , 0))
  printf(fnmatch("[[:alpha:]]"         , "t"  , 0))
  printf(fnmatch("[[:alpha:]]"         , "\t" , 0))
  printf(fnmatch("[[:digit:]]"         , "0"  , 0))
  printf(fnmatch("[[:digit:]]"         , "\t" , 0))
  printf(fnmatch("[[:digit:]]"         , "t"  , 0))
  printf(fnmatch("[[:print:]]"         , "\t" , 0))
  printf(fnmatch("[[:print:]]"         , "t"  , 0))
  printf(fnmatch("[[:upper:]]"         , "T"  , 0))
  printf(fnmatch("[[:upper:]]"         , "\t" , 0))
  printf(fnmatch("[[:upper:]]"         , "t"  , 0))
  printf("\n") # 01011010101110011
  # note: not matched on Windows
  printf(fnmatch("[[:blank:]]"         , "\t" , 0))
  printf(fnmatch("[[:blank:]]"         , "t"  , 0))
  printf(fnmatch("[[:graph:]]"         , "\t" , 0))
  printf(fnmatch("[[:graph:]]"         , "t"  , 0))
  printf(fnmatch("[[:punct:]]"         , "."  , 0))
  printf(fnmatch("[[:punct:]]"         , "t"  , 0))
  printf(fnmatch("[[:punct:]]"         , "\t" , 0))
  printf(fnmatch("[[:xdigit:]]"        , "0"  , 0))
  printf(fnmatch("[[:xdigit:]]"        , "\t" , 0))
  printf(fnmatch("[[:xdigit:]]"        , "a"  , 0))
  printf(fnmatch("[[:xdigit:]]"        , "A"  , 0))
  printf(fnmatch("[[:xdigit:]]"        , "t"  , 0))
  printf(fnmatch("[[alpha]]"           , "a"  , 0))
  printf(fnmatch("[[alpha:]]"          , "a"  , 0))
  printf(fnmatch("[[alpha]]"           , "a]" , 0))
  printf(fnmatch("[[alpha:]]"          , "a]" , 0))
  # win:  1110011010011100
  # else: 0110011010011100
  printf("\n")
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[:alpha:][.b.]]"    , "a"  , 0))
  printf(fnmatch("[[:alpha:][=b=]]"    , "a"  , 0))
  printf("\n") # 00
}
  printf(fnmatch("[[:alpha:][:digit:]]", "a"  , 0))
  printf(fnmatch("[[:digit:][:alpha:]]", "a"  , 0))
  printf("\n") # 00

  # B.6 018(C)
  printf(fnmatch("[a-c]"          , "a"   , 0))
  printf(fnmatch("[a-c]"          , "b"   , 0))
  printf(fnmatch("[a-c]"          , "c"   , 0))
  printf(fnmatch("[b-c]"          , "a"   , 0))
  printf(fnmatch("[b-c]"          , "d"   , 0))
  printf(fnmatch("[a-c]"          , "B"   , 0))
  printf(fnmatch("[A-C]"          , "b"   , 0))
  printf(fnmatch("[a-c]"          , ""    , 0))
  printf(fnmatch("[a-ca-z]"       , "as"  , 0))
  printf("\n") # 000111111
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[.a.]-c]"      , "a"   , 0))
  printf(fnmatch("[a-[.c.]]"      , "a"   , 0))
  printf(fnmatch("[[.a.]-[.c.]]"  , "a"   , 0))
  printf(fnmatch("[[.a.]-c]"      , "b"   , 0))
  printf(fnmatch("[a-[.c.]]"      , "b"   , 0))
  printf(fnmatch("[[.a.]-[.c.]]"  , "b"   , 0))
  printf(fnmatch("[[.a.]-c]"      , "c"   , 0))
  printf(fnmatch("[a-[.c.]]"      , "c"   , 0))
  printf(fnmatch("[[.a.]-[.c.]]"  , "c"   , 0))
  printf(fnmatch("[[.a.]-c]"      , "d"   , 0))
  printf(fnmatch("[a-[.c.]]"      , "d"   , 0))
  printf(fnmatch("[[.a.]-[.c.]]"  , "d"   , 0))
  printf("\n") # 000000000111
}

  # B.6 019(C)
  printf(fnmatch("[c-a]"            , "a", 0))
  printf("\n") # 1
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[.c.]-a]"        , "a", 0))
  printf(fnmatch("[c-[.a.]]"        , "a", 0))
  printf(fnmatch("[[.c.]-[.a.]]"    , "a", 0))
  printf("\n") # 111
}
  printf(fnmatch("[c-a]"            , "c", 0))
  printf("\n") # 1
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[.c.]-a]"        , "c", 0))
  printf(fnmatch("[c-[.a.]]"        , "c", 0))
  printf(fnmatch("[[.c.]-[.a.]]"    , "c", 0))
  printf("\n") # 111
}

  # B.6 020(C)
  printf(fnmatch("[a-c0-9]"       , "a", 0))
  printf(fnmatch("[a-c0-9]"       , "d", 0))
  printf(fnmatch("[a-c0-9]"       , "B", 0))
  printf("\n") # 011

  # B.6 021(C)
  printf(fnmatch("[-a]"      , "-"  , 0))
  printf(fnmatch("[-b]"      , "a"  , 0))
  printf(fnmatch("[!-a]"     , "-"  , 0))
  printf(fnmatch("[!-b]"     , "a"  , 0))
  printf(fnmatch("[a-c-0-9]" , "-"  , 0))
  printf(fnmatch("[a-c-0-9]" , "b"  , 0))
  printf(fnmatch("a[0-9-a]"  , "a:" , 0))
  printf(fnmatch("a[09-a]"   , "a:" , 0))
  printf("\n") # 01100010

  # B.6 024(C)
  printf(fnmatch("*", ""       , 0))
  printf(fnmatch("*", "asd/sdf", 0))
  printf("\n") # 00

  # B.6 025(C)
  printf(fnmatch("[a-c][a-z]", "as", 0))
  printf(fnmatch("??"        , "as", 0))
  printf("\n") # 00

  # B.6 026(C)
  printf(fnmatch("as*df" , "asd/sdf", 0))
  printf(fnmatch("as*"   , "asd/sdf", 0))
  printf(fnmatch("*df"   , "asd/sdf", 0))
  printf(fnmatch("as*dg" , "asd/sdf", 0))
  printf(fnmatch("as*df" , "asdf"   , 0))
  printf(fnmatch("as*df?", "asdf"   , 0))
  printf(fnmatch("as*??" , "asdf"   , 0))
  printf(fnmatch("a*???" , "asdf"   , 0))
  printf(fnmatch("*????" , "asdf"   , 0))
  printf(fnmatch("????*" , "asdf"   , 0))
  printf(fnmatch("??*?"  , "asdf"   , 0))
  printf("\n") # 00010100000

  # B.6 027(C)
  printf(fnmatch("/"     , "/"  , 0))
  printf(fnmatch("/*"    , "/"  , 0))
  printf(fnmatch("*/"    , "/"  , 0))
  printf(fnmatch("/?"    , "/"  , 0))
  printf(fnmatch("?/"    , "/"  , 0))
  printf(fnmatch("?"     , "/"  , 0))
  printf(fnmatch("?"     , "."  , 0))
  printf(fnmatch("??"    , "/." , 0))
  printf(fnmatch("[!a-c]", "/"  , 0))
  printf(fnmatch("[!a-c]", "."  , 0))
  printf("\n") # 0001100000

  # B.6 029(C)
  printf(fnmatch("/"        , "/"     , FNM["PATHNAME"]))
  printf(fnmatch("//"       , "//"    , FNM["PATHNAME"]))
  printf(fnmatch("/*"       , "/.a"   , FNM["PATHNAME"]))
  printf(fnmatch("/?a"      , "/.a"   , FNM["PATHNAME"]))
  printf(fnmatch("/[!a-z]a" , "/.a"   , FNM["PATHNAME"]))
  printf(fnmatch("/*/?b"    , "/.a/.b", FNM["PATHNAME"]))
  printf("\n") # 000000

  # B.6 030(C)
  printf(fnmatch("?"   , "/"     , FNM["PATHNAME"]))
  printf(fnmatch("*"   , "/"     , FNM["PATHNAME"]))
  printf(fnmatch("a?b" , "a/b"   , FNM["PATHNAME"]))
  printf(fnmatch("/*b" , "/.a/.b", FNM["PATHNAME"]))
  printf("\n") # 1111

  # B.6 031(C)
  printf(fnmatch("\\/\\$" , "/$" , 0))
  printf(fnmatch("\\/\\[" , "/[" , 0))
  printf(fnmatch("\\/["   , "/[" , 0))
  printf(fnmatch("\\/\\[]", "/[]", 0))
  printf("\n") # 0000

  # B.6 032(C)
  printf(fnmatch("\\/\\$", "/$"    , FNM["NOESCAPE"]))
  printf(fnmatch("\\/\\$", "/\\$"  , FNM["NOESCAPE"]))
  printf(fnmatch("\\/\\$", "\\/\\$", FNM["NOESCAPE"]))
  printf("\n") # 110

  # B.6 033(C)
  printf(fnmatch(".*"      , ".asd"   , FNM["PERIOD"]))
  printf(fnmatch("*"       , "/.asd"  , FNM["PERIOD"]))
  printf(fnmatch("*/?*f"   , "/as/.df", FNM["PERIOD"]))
  printf(fnmatch(".[!a-z]*", "..asd"  , FNM["PERIOD"]))
  printf("\n") # 0000

  # B.6 034(C)
  printf(fnmatch("*"      , ".asd", FNM["PERIOD"]))
  printf(fnmatch("?asd"   , ".asd", FNM["PERIOD"]))
  printf(fnmatch("[!a-z]*", ".asd", FNM["PERIOD"]))
  printf("\n") # 111

  # B.6 035(C)
  printf(fnmatch("/."      , "/."      , or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/.*/.*"  , "/.a./.b.", or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/.??/.??", "/.a./.b.", or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf("\n") # 000

  # B.6 036(C)
  printf(fnmatch("*"      , "/."     , or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/*"     , "/."     , or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/?"     , "/."     , or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/[!a-z]", "/."     , or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/*/*"   , "/a./.b.", or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf(fnmatch("/??/???", "/a./.b.", or(FNM["PATHNAME"], FNM["PERIOD"])))
  printf("\n") # 111111

  # Some home-grown tests.
  printf(fnmatch("foo*[abc]z"      , "foobar", 0))
  printf(fnmatch("foo*[abc][xyz]"  , "foobaz", 0))
  printf(fnmatch("foo?*[abc][xyz]" , "foobaz", 0))
  printf(fnmatch("foo?*[abc][x/yz]", "foobaz", 0))
  printf(fnmatch("foo?*[abc]/[xyz]", "foobaz", FNM["PATHNAME"]))
  printf(fnmatch("a/"              , "a"     , FNM["PATHNAME"]))
  printf(fnmatch("a"               , "a/"    , FNM["PATHNAME"]))
  printf(fnmatch("/a"              , "//a"   , FNM["PATHNAME"]))
  printf(fnmatch("//a"             , "/a"    , FNM["PATHNAME"]))
  printf(fnmatch("[a-]z"           , "az"    , 0))
  printf(fnmatch("[ab-]z"          , "bz"    , 0))
  printf(fnmatch("[ab-]z"          , "cz"    , 0))
  printf(fnmatch("[ab-]z"          , "-z"    , 0))
  printf(fnmatch("[-a]z"           , "az"    , 0))
  printf(fnmatch("[-ab]z"          , "bz"    , 0))
  printf(fnmatch("[-ab]z"          , "cz"    , 0))
  printf(fnmatch("[-ab]z"          , "-z"    , 0))
  printf(fnmatch("[\\\\-a]"        , "\\"    , 0))
  printf(fnmatch("[\\\\-a]"        , "_"     , 0))
  printf(fnmatch("[\\\\-a]"        , "a"     , 0))
  printf(fnmatch("[\\\\-a]"        , "-"     , 0))
  printf(fnmatch("[\\]-a]"         , "\\"    , 0))
  printf(fnmatch("[\\]-a]"         , "_"     , 0))
  printf(fnmatch("[\\]-a]"         , "a"     , 0))
  printf(fnmatch("[\\]-a]"         , "]"     , 0))
  printf(fnmatch("[\\]-a]"         , "-"     , 0))
  printf(fnmatch("[!\\\\-a]"       , "\\"    , 0))
  printf(fnmatch("[!\\\\-a]"       , "_"     , 0))
  printf(fnmatch("[!\\\\-a]"       , "a"     , 0))
  printf(fnmatch("[!\\\\-a]"       , "-"     , 0))
  printf(fnmatch("[\\!-]"          , "!"     , 0))
  printf(fnmatch("[\\!-]"          , "-"     , 0))
  printf(fnmatch("[\\!-]"          , "\\"    , 0))
  printf(fnmatch("[Z-\\\\]"        , "Z"     , 0))
  printf(fnmatch("[Z-\\\\]"        , "["     , 0))
  printf(fnmatch("[Z-\\\\]"        , "\\"    , 0))
  printf(fnmatch("[Z-\\\\]"        , "-"     , 0))
  printf(fnmatch("[Z-\\]]"         , "Z"     , 0))
  printf(fnmatch("[Z-\\]]"         , "["     , 0))
  printf(fnmatch("[Z-\\]]"         , "\\"    , 0))
  printf(fnmatch("[Z-\\]]"         , "]"     , 0))
  printf(fnmatch("[Z-\\]]"         , "-"     , 0))
  printf("\n") # 100011111001000100001100011110001000100001

  # Test of GNU extensions.
  printf(fnmatch("x"  , "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x"  , "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x"  , "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*"  , "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*"  , "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*"  , "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*x" , "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*x" , "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("*x" , "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x*" , "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x*" , "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x*" , "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("a"  , "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("a"  , "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("a"  , "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x/y", "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x/y", "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x/y", "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x?y", "x"    , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x?y", "x/y"  , or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf(fnmatch("x?y", "x/y/z", or(FNM["PATHNAME"], FNM["LEADING_DIR"])))
  printf("\n") # 000000000000111100111

  # ksh style matching.
if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("?@(a|b)*@(c)d"                        , "abcd"                          , FNM["EXTMATCH"]))
  printf(fnmatch("/dev/@(tcp|udp)/*/*"                  , "/dev/udp/129.22.8.102/45"      , or(FNM["PATHNAME"], FNM["EXTMATCH"])))
  printf(fnmatch("[1-9]*([0-9])"                        , "12"                            , FNM["EXTMATCH"]))
  printf(fnmatch("[1-9]*([0-9])"                        , "12abc"                         , FNM["EXTMATCH"]))
  printf(fnmatch("[1-9]*([0-9])"                        , "1"                             , FNM["EXTMATCH"]))
  printf(fnmatch("+([0-7])"                             , "07"                            , FNM["EXTMATCH"]))
  printf(fnmatch("+([0-7])"                             , "0377"                          , FNM["EXTMATCH"]))
  printf(fnmatch("+([0-7])"                             , "09"                            , FNM["EXTMATCH"]))
  printf(fnmatch("para@(chute|graph)"                   , "paragraph"                     , FNM["EXTMATCH"]))
  printf(fnmatch("para@(chute|graph)"                   , "paramour"                      , FNM["EXTMATCH"]))
  printf(fnmatch("para?([345]|99)1"                     , "para991"                       , FNM["EXTMATCH"]))
  printf(fnmatch("para?([345]|99)1"                     , "para381"                       , FNM["EXTMATCH"]))
  printf(fnmatch("para*([0-9])"                         , "paragraph"                     , FNM["EXTMATCH"]))
  printf(fnmatch("para*([0-9])"                         , "para"                          , FNM["EXTMATCH"]))
  printf(fnmatch("para*([0-9])"                         , "para13829383746592"            , FNM["EXTMATCH"]))
  printf(fnmatch("para+([0-9])"                         , "paragraph"                     , FNM["EXTMATCH"]))
  printf(fnmatch("para+([0-9])"                         , "para"                          , FNM["EXTMATCH"]))
  printf(fnmatch("para+([0-9])"                         , "para987346523"                 , FNM["EXTMATCH"]))
  printf(fnmatch("para!(*.[0-9])"                       , "paragraph"                     , FNM["EXTMATCH"]))
  printf(fnmatch("para!(*.[0-9])"                       , "para.38"                       , FNM["EXTMATCH"]))
  printf(fnmatch("para!(*.[0-9])"                       , "para.graph"                    , FNM["EXTMATCH"]))
  printf(fnmatch("para!(*.[0-9])"                       , "para39"                        , FNM["EXTMATCH"]))
  printf(fnmatch("*(0|1|3|5|7|9)"                       , ""                              , FNM["EXTMATCH"]))
  printf(fnmatch("*(0|1|3|5|7|9)"                       , "137577991"                     , FNM["EXTMATCH"]))
  printf("\n") # 000100010101100110000000

  printf(fnmatch("*(0|1|3|5|7|9)"                       , "2468"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*(0|1|3|5|7|9)"                       , "1358"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*.c?(c)"                              , "file.c"                        , FNM["EXTMATCH"]))
  printf(fnmatch("*.c?(c)"                              , "file.C"                        , FNM["EXTMATCH"]))
  printf(fnmatch("*.c?(c)"                              , "file.cc"                       , FNM["EXTMATCH"]))
  printf(fnmatch("*.c?(c)"                              , "file.ccc"                      , FNM["EXTMATCH"]))
  printf(fnmatch("!(*.c|*.h|Makefile.in|config*|README)", "parse.y"                       , FNM["EXTMATCH"]))
  printf(fnmatch("!(*.c|*.h|Makefile.in|config*|README)", "shell.c"                       , FNM["EXTMATCH"]))
  printf(fnmatch("!(*.c|*.h|Makefile.in|config*|README)", "Makefile"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*;[1-9]*([0-9])"                      , "VMS.FILE;1"                    , FNM["EXTMATCH"]))
  printf(fnmatch("*;[1-9]*([0-9])"                      , "VMS.FILE;0"                    , FNM["EXTMATCH"]))
  printf(fnmatch("*;[1-9]*([0-9])"                      , "VMS.FILE;"                     , FNM["EXTMATCH"]))
  printf(fnmatch("*;[1-9]*([0-9])"                      , "VMS.FILE;139"                  , FNM["EXTMATCH"]))
  printf(fnmatch("*;[1-9]*([0-9])"                      , "VMS.FILE;1N"                   , FNM["EXTMATCH"]))
  printf(fnmatch("ab**(e|f)"                            , "abcfefg"                       , FNM["EXTMATCH"]))
  printf(fnmatch("ab**(e|f)g"                           , "abcfefg"                       , FNM["EXTMATCH"]))
  printf(fnmatch("ab*+(e|f)"                            , "ab"                            , FNM["EXTMATCH"]))
  printf(fnmatch("ab***ef"                              , "abef"                          , FNM["EXTMATCH"]))
  printf(fnmatch("ab**"                                 , "abef"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "fofo"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "ffo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "foooofo"                       , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "foooofof"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "fooofoofofooo"                 , FNM["EXTMATCH"]))
  printf("\n") # 110101010011010010000000

  printf(fnmatch("*(f+(o))"                             , "foooofof"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "xfoooofof"                     , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "foooofofx"                     , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofxoofxo"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o))"                             , "ofooofoofofooo"                , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o)x)"                            , "foooxfooxfoxfooox"             , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o)x)"                            , "foooxfooxofoxfooox"            , FNM["EXTMATCH"]))
  printf(fnmatch("*(f*(o)x)"                            , "foooxfooxfxfooox"              , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofxoofxo"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofoooxoofxo"                   , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofoooxoofxoofoooxoofxo"        , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofoooxoofxoofoooxoofxoo"       , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofoooxoofxoofoooxoofxofo"      , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(of*(o)x)o)"                       , "ofoooxoofxoofoooxoofxooofxofxo", FNM["EXTMATCH"]))
  printf(fnmatch("*(@(a))a@(c)"                         , "aac"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(@(a))a@(c)"                         , "ac"                            , FNM["EXTMATCH"]))
  printf(fnmatch("*(@(a))a@(c)"                         , "c"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*(@(a))a@(c)"                         , "aaac"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*(@(a))a@(c)"                         , "baaac"                         , FNM["EXTMATCH"]))
  printf(fnmatch("?@(a|b)*@(c)d"                        , "abcd"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(ab|a*@(b))*(c)d"                    , "abcd"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(ab|a*(b))*(c)d"                     , "acd"                           , FNM["EXTMATCH"]))
  printf(fnmatch("@(ab|a*(b))*(c)d"                     , "abbcd"                         , FNM["EXTMATCH"]))
  printf(fnmatch("@(b+(c)d|e*(f)g?|?(h)i@(j|k))"        , "effgz"                         , FNM["EXTMATCH"]))
  printf("\n") # 111010100000100010100000

  printf(fnmatch("@(b+(c)d|e*(f)g?|?(h)i@(j|k))"        , "efgz"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(b+(c)d|e*(f)g?|?(h)i@(j|k))"        , "egz"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(b+(c)d|e*(f)g?|?(h)i@(j|k))"        , "egzefffgzbcdij"                , FNM["EXTMATCH"]))
  printf(fnmatch("@(b+(c)d|e+(f)g?|?(h)i@(j|k))"        , "egz"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(of+(o))"                            , "ofoofo"                        , FNM["EXTMATCH"]))
  printf(fnmatch("*(oxf+(ox))"                          , "oxfoxoxfox"                    , FNM["EXTMATCH"]))
  printf(fnmatch("*(oxf+(ox))"                          , "oxfoxfox"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(of+(o)|f)"                          , "ofoofo"                        , FNM["EXTMATCH"]))
  printf(fnmatch("@(foo|f|fo)*(f|of+(o))"               , "foofoofo"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(of|oof+(o))"                        , "oofooofo"                      , FNM["EXTMATCH"]))
  printf(fnmatch("*(*(f)*(o))"                          , "fffooofoooooffoofffooofff"     , FNM["EXTMATCH"]))
  printf(fnmatch("*(fo|foo)"                            , "fofoofoofofoo"                 , FNM["EXTMATCH"]))
  printf(fnmatch("!(x)"                                 , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(x)*"                                , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)"                               , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)*"                              , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)"                               , "foobar"                        , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)*"                              , "foobar"                        , FNM["EXTMATCH"]))
  printf(fnmatch("!(*.*).!(*.*)"                        , "moo.cow"                       , FNM["EXTMATCH"]))
  printf(fnmatch("!(*.*).!(*.*)"                        , "mad.moo.cow"                   , FNM["EXTMATCH"]))
  printf(fnmatch("mu!(*(c))?.pa!(*(z))?"                , "mucca.pazza"                   , FNM["EXTMATCH"]))
  printf(fnmatch("!(f)"                                 , "fff"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(!(f))"                              , "fff"                           , FNM["EXTMATCH"]))
  printf(fnmatch("+(!(f))"                              , "fff"                           , FNM["EXTMATCH"]))
  printf("\n") # 000100100000001000011000

  printf(fnmatch("!(f)"                                 , "ooo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(!(f))"                              , "ooo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("+(!(f))"                              , "ooo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(f)"                                 , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("*(!(f))"                              , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("+(!(f))"                              , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(f)"                                 , "f"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*(!(f))"                              , "f"                             , FNM["EXTMATCH"]))
  printf(fnmatch("+(!(f))"                              , "f"                             , FNM["EXTMATCH"]))
  printf(fnmatch("@(!(z*)|*x)"                          , "foot"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(!(z*)|*x)"                          , "zoot"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(!(z*)|*x)"                          , "foox"                          , FNM["EXTMATCH"]))
  printf(fnmatch("@(!(z*)|*x)"                          , "zoox"                          , FNM["EXTMATCH"]))
  printf(fnmatch("*(!(foo))"                            , "foo"                           , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)b*"                             , "foob"                          , FNM["EXTMATCH"]))
  printf(fnmatch("!(foo)b*"                             , "foobb"                         , FNM["EXTMATCH"]))
  printf(fnmatch("*([a[])"                              , "["                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([]a[])"                             , "]"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([]a[])"                             , "a"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([!]a[])"                            , "b"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([!]a[]|[[])"                        , "["                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([!]a[]|[]])"                        , "]"                             , FNM["EXTMATCH"]))
  printf(fnmatch("!([!]a[])"                            , "["                             , FNM["EXTMATCH"]))
  printf(fnmatch("!([!]a[])"                            , "]"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([)])"                               , ")"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*([*(])"                              , "*"                             , FNM["EXTMATCH"]))
  printf(fnmatch("*!(|a)cd"                             , "abcd"                          , FNM["EXTMATCH"]))
  printf(fnmatch("+([abc])/*"                           , "ab/.a"                         , or(or(FNM["EXTMATCH"], FNM["PATHNAME"]), FNM["PERIOD"])))
  printf(fnmatch(""                                     , ""                              , 0))
  printf(fnmatch(""                                     , ""                              , FNM["EXTMATCH"]))
  printf(fnmatch("*([abc])"                             , ""                              , FNM["EXTMATCH"]))
  printf(fnmatch("?([abc])"                             , ""                              , FNM["EXTMATCH"]))
  printf("\n") # 00000011101000100000000000010000
}

}
