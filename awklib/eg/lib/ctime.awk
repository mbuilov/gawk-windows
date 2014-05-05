# ctime.awk
#
# awk version of C ctime(3) function

function ctime(ts,    format)
{
    format = PROCINFO["strftime"]
    if (ts == 0)
        ts = systime()       # use current time as default
    return strftime(format, ts)
}
