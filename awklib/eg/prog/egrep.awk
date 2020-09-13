# egrep.awk --- simulate egrep in awk
#
# Arnold Robbins, arnold@skeeve.com, Public Domain
# May 1993
# Revised September 2020

# Options:
#    -c    count of lines
#    -e    argument is pattern
#    -i    ignore case
#    -l    print filenames only
#    -n    add line number to output
#    -q    quiet - use exit value
#    -s    silent - don't print errors
#    -v    invert test, success if no match
#    -x    the entire line must match
#
# Requires getopt library function
# Uses IGNORECASE, BEGINFILE and ENDFILE
# Invoke using gawk -f egrep.awk -- options ...

BEGIN {
    while ((c = getopt(ARGC, ARGV, "ce:ilnqsvx")) != -1) {
        if (c == "c")
            count_only++
        else if (c == "e")
            pattern = Optarg
        else if (c == "i")
            IGNORECASE = 1
        else if (c == "l")
            filenames_only++
        else if (c == "n")
            line_numbers++
        else if (c == "q")
            no_print++
        else if (c == "s")
            no_errors++
        else if (c == "v")
            invert++
        else if (c == "x")
            full_line++
        else
            usage()
    }
    if (pattern == "")
        pattern = ARGV[Optind++]

    if (pattern == "")
      usage()

    for (i = 1; i < Optind; i++)
        ARGV[i] = ""

    if (Optind >= ARGC) {
        ARGV[1] = "-"
        ARGC = 2
    } else if (ARGC - Optind > 1)
        do_filenames++
}
BEGINFILE {
    fcount = 0
    if (ERRNO && no_errors)
        nextfile
}
ENDFILE {
    if (! no_print && count_only) {
        if (do_filenames)
            print file ":" fcount
        else
            print fcount
    }

    total += fcount
}
{
    matches = match($0, pattern)
    if (matches && full_line && (RSTART != 1 || RLENGTH != length()))
         matches = 0

    if (invert)
        matches = ! matches

    fcount += matches    # 1 or 0

    if (! matches)
        next

    if (! count_only) {
        if (no_print)
            nextfile

        if (filenames_only) {
            print FILENAME
            nextfile
        }

        if (do_filenames)
            if (line_numbers)
               print FILENAME ":" FNR ":" $0
            else
               print FILENAME ":" $0
        else
            print
    }
}
END {
    exit (total == 0)
}
function usage()
{
    print("Usage:\tegrep [-cilnqsvx] [-e pat] [files ...]") > "/dev/stderr"
    print("\tegrep [-cilnqsvx] pat [files ...]") > "/dev/stderr"
    exit 1
}
