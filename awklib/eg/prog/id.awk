# id.awk --- implement id in awk
#
# Requires user and group library functions and getopt
#
# Arnold Robbins, arnold@skeeve.com, Public Domain
# May 1993
# Revised February 1996
# Revised May 2014
# Revised September 2014
# Revised September 2020

# output is:
# uid=12(foo) euid=34(bar) gid=3(baz) \
#             egid=5(blat) groups=9(nine),2(two),1(one)

# Options:
#   -G Output all group ids as space separated numbers (ruid, euid, groups)
#   -g Output only the euid as a number
#   -n Ouput name instead of the numeric value (with -g/-G/-u)
#   -r Output ruid/rguid instead of effective id
#   -u Output only effective user id, as a number

function usage()
{
    printf("Usage:\n" \
           "\tid [user]\n" \
           "\tid −G [−n] [user]\n" \
           "\tid −g [−nr] [user]\n" \
           "\tid −u [−nr] [user]\n") > "/dev/stderr"

    exit 1
}
BEGIN {
    # parse args
    while ((c = getopt(ARGC, ARGV, "Ggnru")) != -1) {
        if (c == "G")
            groupset_only++
        else if (c == "g")
            egid_only++
        else if (c == "n")
            names_not_groups++
        else if (c == "r")
            real_ids_only++
        else if (c == "u")
            euid_only++
        else
            usage()
    }
    if (groupset_only && real_ids_only)
        usage()
    else if (ARGC - Optind > 1)
        usage()
    if (ARGC - Optind == 0) {
        # gather info for current user
        uid = PROCINFO["uid"]
        euid = PROCINFO["euid"]
        gid = PROCINFO["gid"]
        egid = PROCINFO["egid"]
        for (i = 1; ("group" i) in PROCINFO; i++)
            groupset[i] = PROCINFO["group" i]
    } else {
        fill_info_for_user(ARGV[ARGC-1])
        real_ids_only++
    }
    if (groupset_only) {
        if (names_not_groups) {
            for (i = 1; i in groupset; i++) {
                entry = getgrgid(groupset[i])
                name = get_first_field(entry)
                printf("%s", name)
                if ((i + 1) in groupset)
                    printf(" ")
            }
        } else {
            for (i = 1; i in groupset; i++) {
                printf("%u", groupset[i])
                if ((i + 1) in groupset)
                    printf(" ")
            }
        }

        print ""    # final newline
        exit 0
    }
    else if (egid_only) {
        id = real_ids_only ? gid : egid
        if (names_not_groups) {
            entry = getgrgid(id)
            name = get_first_field(entry)
            printf("%s\n", name)
        } else {
            printf("%u\n", id)
        }

        exit 0
    }
    else if (euid_only) {
        id = real_ids_only ? uid : euid
        if (names_not_groups) {
            entry = getpwuid(id)
            name = get_first_field(entry)
            printf("%s\n", name)
        } else {
            printf("%u\n", id)
        }

        exit 0
    }
    printf("uid=%d", uid)
    pw = getpwuid(uid)
    print_first_field(pw)
    if (euid != uid && ! real_ids_only) {
        printf(" euid=%d", euid)
        pw = getpwuid(euid)
        print_first_field(pw)
    }
    printf(" gid=%d", gid)
    pw = getgrgid(gid)
    print_first_field(pw)

    if (egid != gid && ! real_ids_only) {
        printf(" egid=%d", egid)
        pw = getgrgid(egid)
        print_first_field(pw)
    }
    for (i = 1; i in groupset; i++) {
        if (i == 1)
            printf(" groups=")
        group = groupset[i]
        printf("%d", group)
        pw = getgrgid(group)
        print_first_field(pw)
        if ((i + 1) in groupset)
            printf(",")
    }

    print ""
}
function get_first_field(str,  a)
{
    if (str != "") {
        split(str, a, ":")
        return a[1]
    }
}
function print_first_field(str)
{
    first = get_first_field(str)
    printf("(%s)", first)
}
function fill_info_for_user(user,   
                            pwent, fields, groupnames, grent, groups, i)
{
    pwent = getpwnam(user)
    if (pwent == "") {
        printf("id: '%s': no such user\n", user) > "/dev/stderr"
        exit 1
    }

    split(pwent, fields, ":")
    uid = fields[3] + 0
    gid = fields[4] + 0

    groupnames = getgruser(user)
    split(groupnames, groups, " ")
    for (i = 1; i in groups; i++) {
        grent = getgrnam(groups[i])
        split(grent, fields, ":")
        groupset[i] = fields[3] + 0
    }
}
