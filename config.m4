PHP_ARG_ENABLE(cgss, whether to enable cgss support,
Make sure that the comment is aligned:
[  --enable-cgss           Enable cgss support])

if test "$PHP_CGSS" != "no"; then
  PHP_REQUIRE_CXX()
  PHP_SUBST(CGSS_SHARED_LIBADD)
  PHP_ADD_LIBRARY(cgss, , CGSS_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, , CGSS_SHARED_LIBADD)
  PHP_ADD_INCLUDE([./libcgss/src/lib])
  PHP_ADD_INCLUDE([./libcgss/src/lib/ichinose])
  PHP_NEW_EXTENSION(cgss, cgss.cpp libcgss/src/apps/acbunpack/acbunpack.cpp libcgss/src/lib/ichinose/*.cpp libcgss/src/lib/takamori/*.cpp libcgss/src/lib/takamori/*/*.cpp, $ext_shared)
fi