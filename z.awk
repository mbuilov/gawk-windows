BEGIN { f = "foo" ; p = @/o/ ; gsub(p, "q", f); print f }
