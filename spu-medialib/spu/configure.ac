AC_INIT(spu-medialib/spe, VERSION)

host_alias=spu-elf
AC_SUBST(host_alias)

AC_PROG_CC(spu-elf-gcc)
#AC_PROG_CXX(spu-elf-g++)
m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

AC_PATH_TOOL(EMBEDSPU, embedspu)

AC_SUBST(DESTDIR)

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
