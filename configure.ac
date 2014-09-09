#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([OpenNotrium], [1.0])
AC_CONFIG_SRCDIR([WinMain.cpp])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([GL], [glBegin])
AC_CHECK_LIB([SDL], [SDL_SetVideoMode])
AC_CHECK_LIB([SDL_image], [IMG_load])
AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio])
AC_CHECK_LIB([physfs], [PHYSFS_init])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stdlib.h cstring])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset sqrt])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
