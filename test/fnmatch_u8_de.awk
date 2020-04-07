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
  # Following are tests outside the scope of IEEE 2003.2 since they are using
  # locales other than the C locale.  The main focus of the tests is on the
  # handling of ranges and the recognition of character (vs bytes).

  # And with a multibyte character set.
  printf(fnmatch("[a-z]"      , "a", 0))
  printf(fnmatch("[a-z]"      , "z", 0))
  printf(fnmatch("[a-z]"      , "ä", 0))
  printf(fnmatch("[a-z]"      , "ö", 0))
  printf(fnmatch("[a-z]"      , "ü", 0))
  printf(fnmatch("[a-z]"      , "A", 0))
  printf(fnmatch("[a-z]"      , "Z", 0))
  printf(fnmatch("[a-z]"      , "Ä", 0))
  printf(fnmatch("[a-z]"      , "Ö", 0))
  printf(fnmatch("[a-z]"      , "Ü", 0))
  printf(fnmatch("[A-Z]"      , "a", 0))
  printf(fnmatch("[A-Z]"      , "z", 0))
  printf(fnmatch("[A-Z]"      , "ä", 0))
  printf(fnmatch("[A-Z]"      , "ö", 0))
  printf(fnmatch("[A-Z]"      , "ü", 0))
  printf(fnmatch("[A-Z]"      , "A", 0))
  printf(fnmatch("[A-Z]"      , "Z", 0))
  printf(fnmatch("[A-Z]"      , "Ä", 0))
  printf(fnmatch("[A-Z]"      , "Ö", 0))
  printf(fnmatch("[A-Z]"      , "Ü", 0))
  printf(fnmatch("[[:lower:]]", "a", 0))
  printf(fnmatch("[[:lower:]]", "z", 0))
  printf(fnmatch("[[:lower:]]", "ä", 0))
  printf(fnmatch("[[:lower:]]", "ö", 0))
  printf(fnmatch("[[:lower:]]", "ü", 0))
  printf(fnmatch("[[:lower:]]", "A", 0))
  printf(fnmatch("[[:lower:]]", "Z", 0))
  printf(fnmatch("[[:lower:]]", "Ä", 0))
  printf(fnmatch("[[:lower:]]", "Ö", 0))
  printf(fnmatch("[[:lower:]]", "Ü", 0))
  printf(fnmatch("[[:upper:]]", "a", 0))
  printf(fnmatch("[[:upper:]]", "z", 0))
  printf(fnmatch("[[:upper:]]", "ä", 0))
  printf(fnmatch("[[:upper:]]", "ö", 0))
  printf(fnmatch("[[:upper:]]", "ü", 0))
  printf(fnmatch("[[:upper:]]", "A", 0))
  printf(fnmatch("[[:upper:]]", "Z", 0))
  printf(fnmatch("[[:upper:]]", "Ä", 0))
  printf(fnmatch("[[:upper:]]", "Ö", 0))
  printf(fnmatch("[[:upper:]]", "Ü", 0))
  printf(fnmatch("[[:alpha:]]", "a", 0))
  printf(fnmatch("[[:alpha:]]", "z", 0))
  printf(fnmatch("[[:alpha:]]", "ä", 0))
  printf(fnmatch("[[:alpha:]]", "ö", 0))
  printf(fnmatch("[[:alpha:]]", "ü", 0))
  printf(fnmatch("[[:alpha:]]", "A", 0))
  printf(fnmatch("[[:alpha:]]", "Z", 0))
  printf(fnmatch("[[:alpha:]]", "Ä", 0))
  printf(fnmatch("[[:alpha:]]", "Ö", 0))
  printf(fnmatch("[[:alpha:]]", "Ü", 0))
  printf("\n") # 00000111111111100000000001111111111000000000000000

if (PROCINFO["platform"] != "windows") {
  printf(fnmatch("[[=a=]b]", "a", 0))
  printf(fnmatch("[[=a=]b]", "â", 0))
  printf(fnmatch("[[=a=]b]", "à", 0))
  printf(fnmatch("[[=a=]b]", "á", 0))
  printf(fnmatch("[[=a=]b]", "ä", 0))
  printf(fnmatch("[[=a=]b]", "b", 0))
  printf(fnmatch("[[=a=]b]", "c", 0))
  printf(fnmatch("[[=â=]b]", "a", 0))
  printf(fnmatch("[[=â=]b]", "â", 0))
  printf(fnmatch("[[=â=]b]", "à", 0))
  printf(fnmatch("[[=â=]b]", "á", 0))
  printf(fnmatch("[[=â=]b]", "ä", 0))
  printf(fnmatch("[[=â=]b]", "b", 0))
  printf(fnmatch("[[=â=]b]", "c", 0))
  printf(fnmatch("[[=à=]b]", "a", 0))
  printf(fnmatch("[[=à=]b]", "â", 0))
  printf(fnmatch("[[=à=]b]", "à", 0))
  printf(fnmatch("[[=à=]b]", "á", 0))
  printf(fnmatch("[[=à=]b]", "ä", 0))
  printf(fnmatch("[[=à=]b]", "b", 0))
  printf(fnmatch("[[=à=]b]", "c", 0))
  printf(fnmatch("[[=á=]b]", "a", 0))
  printf(fnmatch("[[=á=]b]", "â", 0))
  printf(fnmatch("[[=á=]b]", "à", 0))
  printf(fnmatch("[[=á=]b]", "á", 0))
  printf(fnmatch("[[=á=]b]", "ä", 0))
  printf(fnmatch("[[=á=]b]", "b", 0))
  printf(fnmatch("[[=á=]b]", "c", 0))
  printf(fnmatch("[[=ä=]b]", "a", 0))
  printf(fnmatch("[[=ä=]b]", "â", 0))
  printf(fnmatch("[[=ä=]b]", "à", 0))
  printf(fnmatch("[[=ä=]b]", "á", 0))
  printf(fnmatch("[[=ä=]b]", "ä", 0))
  printf(fnmatch("[[=ä=]b]", "b", 0))
  printf(fnmatch("[[=ä=]b]", "c", 0))
  printf("\n") # 00000010000001000000100000010000001

  printf(fnmatch("[[.a.]]a", "aa", 0))
  printf(fnmatch("[[.a.]]a", "ba", 0))
  printf("\n") # 01
}

}
