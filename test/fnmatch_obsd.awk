# fnmatch regression tests from:

# 
# Public domain, 2008, Todd C. Miller <Todd.Miller@courtesan.com>
# 

# https://github.com/busterb/libc-regress-openbsd/tree/master/fnmatch

@load "fnmatch"

BEGIN {
  printf(fnmatch("/bin/[[:alpha:][:alnum:]]*", "/bin/ls", FNM["PATHNAME"]))
  # do not matches by fnmatch from glibc
  printf(fnmatch("/bin/[[:upper:]][[:alnum:]]", "/bin/ls", FNM["CASEFOLD"]))
  printf(fnmatch("/bin/[[:opper:][:alnum:]]*", "/bin/ls", 0))
  printf(fnmatch("[[:alpha:][:alnum:]]*.c", "foo1.c", FNM["PERIOD"]))
  printf(fnmatch("[[:upper:]]*", "FOO", 0))
  printf("\n") # 01100

  # 'te\st' 'test'; no match if FNM_NOESCAPE
  printf(fnmatch("te\\st", "test", 0))
  printf(fnmatch("te\\st", "test", FNM["NOESCAPE"]))
  printf(fnmatch("te\\st", "test", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"])))
  printf("\n") # 010

  # 'te\\st' 'te\st'; no match if FNM_NOESCAPE
  printf(fnmatch("te\\\\st", "te\\st", 0))
  printf(fnmatch("te\\\\st", "te\\st", FNM["NOESCAPE"]))
  printf(fnmatch("te\\\\st", "te\\st", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"])))
  printf("\n") # 010

  # 'te\*t' 'te*t'; no match if FNM_NOESCAPE
  printf(fnmatch("te\\*t", "te*t", 0))
  printf(fnmatch("te\\*t", "te*t", FNM["NOESCAPE"]))
  printf(fnmatch("te\\*t", "te*t", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"])))
  printf("\n") # 010

  # 'te\*t' 'test'; no match
  printf(fnmatch("te\\*t", "test", 0))
  printf(fnmatch("te\\*t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'te\?t' 'te?t'; no match if FNM_NOESCAPE
  printf(fnmatch("te\\?t", "te?t", 0))
  printf(fnmatch("te\\?t", "te?t", FNM["NOESCAPE"]))
  printf(fnmatch("te\\?t", "te?t", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"])))
  printf("\n") # 010

  # 'te\?t' 'test'; no match
  printf(fnmatch("te\\?t", "test", 0))
  printf(fnmatch("te\\?t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'tesT' 'test'; match if FNM_CASEFOLD
  printf(fnmatch("tesT", "test", 0))
  printf(fnmatch("tesT", "test", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("tesT", "test", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'test' 'Test'; match if FNM_CASEFOLD
  printf(fnmatch("test", "Test", 0))
  printf(fnmatch("test", "Test", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("test", "Test", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'tEst' 'teSt'; match if FNM_CASEFOLD
  printf(fnmatch("tEst", "teSt", 0))
  printf(fnmatch("tEst", "teSt", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("tEst", "teSt", FNM["CASEFOLD"]))
  printf("\n") # 110

  # '?est' 'test'; match always
  printf(fnmatch("?est", "test", 0))
  printf(fnmatch("?est", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te?t' 'test'; match always
  printf(fnmatch("te?t", "test", 0))
  printf(fnmatch("te?t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'tes?' 'test'; match always
  printf(fnmatch("tes?", "test", 0))
  printf(fnmatch("tes?", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test?' 'test'; no match
  printf(fnmatch("test?", "test", 0))
  printf(fnmatch("test?", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # '*' always matches anything
  printf(fnmatch("*", "test", 0))
  printf(fnmatch("*", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '*test' 'test'; match always
  printf(fnmatch("*test", "test", 0))
  printf(fnmatch("*test", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '*est' 'test'; match always
  printf(fnmatch("*est", "test", 0))
  printf(fnmatch("*est", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '*st' 'test'; match always
  printf(fnmatch("*st", "test", 0))
  printf(fnmatch("*st", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 't*t' 'test'; match always
  printf(fnmatch("t*t", "test", 0))
  printf(fnmatch("t*t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te*t' 'test'; match always
  printf(fnmatch("te*t", "test", 0))
  printf(fnmatch("te*t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te*st' 'test'; match always
  printf(fnmatch("te*st", "test", 0))
  printf(fnmatch("te*st", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te*' 'test'; match always
  printf(fnmatch("te*", "test", 0))
  printf(fnmatch("te*", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'tes*' 'test'; match always
  printf(fnmatch("tes*", "test", 0))
  printf(fnmatch("tes*", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test*' 'test'; match always
  printf(fnmatch("test*", "test", 0))
  printf(fnmatch("test*", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '.[\-\t]' '.t'; match always
  printf(fnmatch(".[\\-\\t]", ".t", 0))
  printf(fnmatch(".[\\-\\t]", ".t", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test*?*[a-z]*' 'testgoop'; match always
  printf(fnmatch("test*?*[a-z]*", "testgoop", 0))
  printf(fnmatch("test*?*[a-z]*", "testgoop", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[^abc]t' 'test'; match always
  printf(fnmatch("te[^abc]t", "test", 0))
  printf(fnmatch("te[^abc]t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[^x]t' 'test'; match always
  printf(fnmatch("te[^x]t", "test", 0))
  printf(fnmatch("te[^x]t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[!x]t' 'test'; match always
  printf(fnmatch("te[!x]t", "test", 0))
  printf(fnmatch("te[^x]t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[^x]t' 'text'; no match
  printf(fnmatch("te[^x]t", "text", 0))
  printf(fnmatch("te[^x]t", "text", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'te[^\x]t' 'text'; no match
  printf(fnmatch("te[^\\x]t", "text", 0))
  printf(fnmatch("te[^\\x]t", "text", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'te[^\x' 'text'; no match
  printf(fnmatch("te[^\\x", "text", 0))
  printf(fnmatch("te[^\\x", "text", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'te[/]t' 'text'; no match
  printf(fnmatch("te[/]t", "text", 0))
  printf(fnmatch("te[/]t", "text", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'te[S]t' 'test'; match if FNM_CASEFOLD
  printf(fnmatch("te[S]t", "test", 0))
  printf(fnmatch("te[S]t", "test", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("te[S]t", "test", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'te[r-t]t' 'test'; match always
  printf(fnmatch("te[r-t]t", "test", 0))
  printf(fnmatch("te[r-t]t", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[r-t]t' 'teSt'; match if FNM_CASEFOLD
  printf(fnmatch("te[r-t]t", "teSt", 0))
  printf(fnmatch("te[r-t]t", "teSt", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("te[r-t]t", "teSt", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'te[r-T]t' 'test'; match if FNM_CASEFOLD
  printf(fnmatch("te[r-T]t", "test", 0))
  printf(fnmatch("te[r-T]t", "test", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("te[r-T]t", "test", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'te[R-T]t' 'test'; match if FNM_CASEFOLD
  printf(fnmatch("te[R-T]t", "test", 0))
  printf(fnmatch("te[R-T]t", "test", or(or(or(FNM["LEADING_DIR"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("te[R-T]t", "test", FNM["CASEFOLD"]))
  printf("\n") # 110

  # 'te[r-Tz]t' 'tezt'; match always
  printf(fnmatch("te[r-Tz]t", "tezt", 0))
  printf(fnmatch("te[r-Tz]t", "tezt", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'te[R-T]t' 'tent'; no match
  printf(fnmatch("te[R-T]t", "tent", 0))
  printf(fnmatch("te[R-T]t", "tent", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'tes[]t]' 'test'; match always
  printf(fnmatch("tes[]t]", "test", 0))
  printf(fnmatch("tes[]t]", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'tes[t-]' 'test'; match always
  printf(fnmatch("tes[t-]", "test", 0))
  printf(fnmatch("tes[t-]", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'tes[t-]]' 'test]'; match always
  printf(fnmatch("tes[t-]]", "test]", 0))
  printf(fnmatch("tes[t-]]", "test]", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'tes[t-]]' 'test'; no match
  printf(fnmatch("tes[t-]]", "test", 0))
  printf(fnmatch("tes[t-]]", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'tes[u-]' 'test'; no match
  printf(fnmatch("tes[u-]", "test", 0))
  printf(fnmatch("tes[u-]", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'tes[t-]' 'tes[t-]'; no match
  printf(fnmatch("tes[t-]", "test[t-]", 0))
  printf(fnmatch("tes[t-]", "test[t-]", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test[/-/]' 'test[/-/]'; no match
  printf(fnmatch("test[/-/]", "test/-/", 0))
  printf(fnmatch("test[/-/]", "test/-/", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test[\/-/]' 'test[/-/]'; no match
  printf(fnmatch("test[\\/-/]", "test/-/", 0))
  printf(fnmatch("test[\\/-/]", "test/-/", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test[/-\/]' 'test[/-/]'; no match
  printf(fnmatch("test[/-\\/]", "test/-/", 0))
  printf(fnmatch("test[/-\\/]", "test/-/", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test[/-/]' 'test/'; no match if APR_FNM_PATHNAME
  printf(fnmatch("test[/-/]", "test/", 0))
  printf(fnmatch("test[/-/]", "test/", FNM["PATHNAME"]))
  printf(fnmatch("test[/-/]", "test/", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test[\/-/]' 'test/'; no match if APR_FNM_PATHNAME
  printf(fnmatch("test[\\/-/]", "test/", 0))
  printf(fnmatch("test[\\/-/]", "test/", FNM["PATHNAME"]))
  printf(fnmatch("test[\\/-/]", "test/", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test[/-\/]' 'test/'; no match if APR_FNM_PATHNAME
  printf(fnmatch("test[/-\\/]", "test/", 0))
  printf(fnmatch("test[/-\\/]", "test/", FNM["PATHNAME"]))
  printf(fnmatch("test[/-\\/]", "test/", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # '/test' 'test'; no match
  printf(fnmatch("/test", "test", 0))
  printf(fnmatch("/test", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test' '/test'; no match
  printf(fnmatch("test", "/test", 0))
  printf(fnmatch("test", "/test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test/' 'test'; no match
  printf(fnmatch("test/", "test", 0))
  printf(fnmatch("test/", "test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test' 'test/'; match if FNM_LEADING_DIR
  printf(fnmatch("test", "test/", 0))
  printf(fnmatch("test", "test/", or(or(or(FNM["CASEFOLD"],FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf(fnmatch("test", "test/", FNM["LEADING_DIR"]))
  printf("\n") # 110

  # '\/test' '/test'; match unless FNM_NOESCAPE
  printf(fnmatch("\\/test", "/test", 0))
  printf(fnmatch("\\/test", "/test", FNM["NOESCAPE"]))
  printf(fnmatch("\\/test", "/test", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"])))
  printf("\n") # 010

  # '*test' '/test'; match unless FNM_PATHNAME
  printf(fnmatch("*test", "/test", 0))
  printf(fnmatch("*test", "/test", FNM["PATHNAME"]))
  printf(fnmatch("*test", "/test", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # '/*/test' '/test'; no match
  printf(fnmatch("/*/test", "/test", 0))
  printf(fnmatch("/*/test", "/test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # '/*/test' '/test/test'; match always
  printf(fnmatch("/*/test", "/test/test", 0))
  printf(fnmatch("/*/test", "/test/test", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test/this' 'test/'; match never
  printf(fnmatch("test/this", "test/", 0))
  printf(fnmatch("test/this", "test/", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test/' 'test/this'; match never
  printf(fnmatch("test/", "test/this", 0))
  printf(fnmatch("test/", "test/this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test*/this' 'test/this'; match always
  printf(fnmatch("test*/this", "test/this", 0))
  printf(fnmatch("test*/this", "test/this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test*/this' 'test/that'; match never
  printf(fnmatch("test*/this", "test/that", 0))
  printf(fnmatch("test*/this", "test/that", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 11

  # 'test/*this' 'test/this'; match always
  printf(fnmatch("test/*this", "test/this", 0))
  printf(fnmatch("test/*this", "test/this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '.*' '.this'; match always
  printf(fnmatch(".*", ".this", 0))
  printf(fnmatch(".*", ".this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # '*' '.this'; fails if FNM_PERIOD
  printf(fnmatch("*", ".this", 0))
  printf(fnmatch("*", ".this", FNM["PERIOD"]))
  printf(fnmatch("*", ".this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # '?this' '.this'; fails if FNM_PERIOD
  printf(fnmatch("?this", ".this", 0))
  printf(fnmatch("?this", ".this", FNM["PERIOD"]))
  printf(fnmatch("?this", ".this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # '[.]this' '.this'; fails if FNM_PERIOD
  printf(fnmatch("[.]this", ".this", 0))
  printf(fnmatch("[.]this", ".this", FNM["PERIOD"]))
  printf(fnmatch("[.]this", ".this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test/this' 'test/this'; match always
  printf(fnmatch("test/this", "test/this", 0))
  printf(fnmatch("test/this", "test/this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test?this' 'test/this'; fails if FNM_PATHNAME
  printf(fnmatch("test?this", "test/this", 0))
  printf(fnmatch("test?this", "test/this", FNM["PATHNAME"]))
  printf(fnmatch("test?this", "test/this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test*this' 'test/this'; fails if FNM_PATHNAME
  printf(fnmatch("test*this", "test/this", 0))
  printf(fnmatch("test*this", "test/this", FNM["PATHNAME"]))
  printf(fnmatch("test*this", "test/this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test[/]this' 'test/this'; fails if FNM_PATHNAME
  printf(fnmatch("test[/]this", "test/this", 0))
  printf(fnmatch("test[/]this", "test/this", FNM["PATHNAME"]))
  printf(fnmatch("test[/]this", "test/this", or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test/.*' 'test/.this'; match always
  printf(fnmatch("test/.*", "test/.this", 0))
  printf(fnmatch("test/.*", "test/.this", or(or(or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["PERIOD"]),FNM["PATHNAME"]),FNM["NOESCAPE"])))
  printf("\n") # 00

  # 'test/*' 'test/.this'; fails if FNM_PERIOD and FNM_PATHNAME
  printf(fnmatch("test/*", "test/.this", 0))
  printf(fnmatch("test/*", "test/.this", or(FNM["PERIOD"],FNM["PATHNAME"])))
  printf(fnmatch("test/*", "test/.this", or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test/?' 'test/.this'; fails if FNM_PERIOD and FNM_PATHNAME
  printf(fnmatch("test/?this", "test/.this", 0))
  printf(fnmatch("test/?this", "test/.this", or(FNM["PERIOD"],FNM["PATHNAME"])))
  printf(fnmatch("test/?this", "test/.this", or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["NOESCAPE"])))
  printf("\n") # 010

  # 'test/[.]this' 'test/.this'; fails if FNM_PERIOD and FNM_PATHNAME
  printf(fnmatch("test/[.]this", "test/.this", 0))
  printf(fnmatch("test/[.]this", "test/.this", or(FNM["PERIOD"],FNM["PATHNAME"])))
  printf(fnmatch("test/[.]this", "test/.this", or(or(FNM["CASEFOLD"],FNM["LEADING_DIR"]),FNM["NOESCAPE"])))
  printf("\n") # 010
}
