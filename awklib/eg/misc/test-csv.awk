BEGIN {
     fp[0] = "([^,]+)|(\"[^\"]+\")"
     fp[1] = "([^,]*)|(\"[^\"]+\")"
     fp[2] = "([^,]*)|(\"([^\"]|\"\")+\")"
     FPAT = fp[fpat+0]
}

{
     print "<" $0 ">"
     printf("NF = %s ", NF)
     for (i = 1; i <= NF; i++) {
         printf("<%s>", $i)
     }
     print ""
}
