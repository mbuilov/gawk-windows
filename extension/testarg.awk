BEGIN {
        extension("./testarg.so", "dlload")
        check_arg(x, a);
        check_arg(y, b, z);
        check_arg(p, q, r, s);
}
