# inplace --- load and invoke the inplace extension.

@load "inplace"

# Please set INPLACE_SUFFIX to make a backup copy.  For example, you may
# want to set INPLACE_SUFFIX to .bak on the command line or in a BEGIN rule.

BEGINFILE {
    inplace_begin(FILENAME, INPLACE_SUFFIX)
}

ENDFILE {
    inplace_end(FILENAME, INPLACE_SUFFIX)
}
