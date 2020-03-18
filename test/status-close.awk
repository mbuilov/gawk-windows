BEGIN {
    if (PROCINFO["platform"] == "windows") {
        cat  = "find /v \"\" & exit 3"
        print system("echo xxx| (find /v \"\" & exit 4)")
    } else {
        cat  = "cat ; exit 3"
        print system("echo xxx | (cat ; exit 4)")
    }

    print "YYY" | cat

    print close(cat)

    if (PROCINFO["platform"] == "windows")
        echo = "echo boo& exit 5"
    else
        echo = "echo boo ; exit 5"
    echo | getline boo
    print "got", boo

    print close(echo)
}
