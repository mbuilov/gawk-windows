BEGIN {
        if (PROCINFO["platform"] == "windows") {
                pipe =      "("
                pipe = pipe "(echo.select * from user)&"
                pipe = pipe "(echo.  where Name = 'O\\'Donell')"
                pipe = pipe ")"
        } else {
                pipe =      "cat <<EOF\n"
                pipe = pipe "select * from user\n"
                pipe = pipe "  where Name = 'O\\'Donell'\n"
                pipe = pipe "EOF\n"
        }
        
        while ((pipe | getline) > 0)
                print
        
        exit 0
}       
