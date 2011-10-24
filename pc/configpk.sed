s/^AC_INIT([[]\{0,1\}\([^],]*\)[]]\{0,1\}, *[[]\{0,1\}\([^],]*\)[]]\{0,1\}, *[[]\{0,1\}\([^],]*\)[]]\{0,1\}, *[[]\{0,1\}\([^],)]*\).*$/\
s|^#undef PACKAGE_NAME *$|#define PACKAGE_NAME "\1"|\
s|^#undef VERSION *$|#define VERSION "\2"\|\
s|^#undef PACKAGE_VERSION *$|#define PACKAGE_VERSION "\2"|\
s|^#undef PACKAGE_STRING *$|#define PACKAGE_STRING "\1 \2"|\
s|^#undef PACKAGE_BUGREPORT *$|#define PACKAGE_BUGREPORT "\3"|\
s|^#undef PACKAGE *$|#define PACKAGE "\4"|\
s|^#undef PACKAGE_TARNAME *$|#define PACKAGE_TARNAME "\4"|\
/p
