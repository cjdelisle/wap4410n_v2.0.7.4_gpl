dnl Copyright (C) 1993-2002 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl From Bruno Haible, Marcus Daniels.

AC_PREREQ(2.13)

AC_DEFUN([CL_WORDS_LITTLEENDIAN],
[AC_CACHE_CHECK(byte ordering, cl_cv_sys_endian, [
AC_TRY_RUN([int main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[0] == 1);
}],
cl_cv_sys_endian="big endian",
cl_cv_sys_endian="little endian",
: # must guess the endianness
)
if test -z "$cl_cv_sys_endian"; then
AC_EGREP_CPP(yes,[#if defined(m68k) || defined(__m68k__) || defined(mc68000) || defined(mc68020) || defined(__mc68020__) || defined(sparc) || defined(__sparc__) || defined(MIPSEB) || defined(__MIPSEB__) || defined(hppa) || defined(__hppa) || defined(m88000) || defined(__m88k__)
  yes
#endif
], cl_cv_sys_endian="big endian")
fi
if test -z "$cl_cv_sys_endian"; then
AC_EGREP_CPP(yes,[#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_I386) || defined(MIPSEL) || defined(__MIPSEL__) || defined(__alpha)
  yes
#endif
], cl_cv_sys_endian="little endian")
fi
if test -z "$cl_cv_sys_endian"; then
cl_cv_sys_endian="guessing little endian"
fi
])
case "$cl_cv_sys_endian" in
  *little*) AC_DEFINE(WORDS_LITTLEENDIAN) ;;
  *big*)    ;;
esac
])
