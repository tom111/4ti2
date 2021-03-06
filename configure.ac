#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(4ti2, 1.5.2)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)
AM_MAINTAINER_MODE
AC_CONFIG_FILES([Makefile src/Makefile src/4ti2/Makefile
		 src/groebner/Makefile src/util/Makefile 
		 src/ppi/Makefile src/zsolve/Makefile
		 src/fiber/Makefile
		 m4/Makefile])

AC_CONFIG_HEADERS([src/config.h src/4ti2/4ti2_config.h])

AC_CONFIG_FILES([src/groebner/script.template])

# Groebner unittests
AC_CONFIG_FILES([src/groebner/unittests/Makefile])

# 4ti2 functionality tests
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([test/circuits/Makefile])
AC_CONFIG_FILES([test/genmodel/Makefile])
AC_CONFIG_FILES([test/gensymm/Makefile])
AC_CONFIG_FILES([test/graver/Makefile])
AC_CONFIG_FILES([test/groebner/Makefile])
AC_CONFIG_FILES([test/groebner/gmp/Makefile])
AC_CONFIG_FILES([test/hilbert/Makefile])
AC_CONFIG_FILES([test/markov/Makefile])
AC_CONFIG_FILES([test/markov/gmp/Makefile])
AC_CONFIG_FILES([test/markov/saturation/Makefile])
AC_CONFIG_FILES([test/markov/maxmin/Makefile])
AC_CONFIG_FILES([test/minimize/Makefile])
AC_CONFIG_FILES([test/normalform/Makefile])
AC_CONFIG_FILES([test/output/Makefile])
AC_CONFIG_FILES([test/output/binomials/Makefile])
AC_CONFIG_FILES([test/output/maple/Makefile])
AC_CONFIG_FILES([test/output/positive/Makefile])
AC_CONFIG_FILES([test/ppi/Makefile])
AC_CONFIG_FILES([test/qsolve/Makefile])
AC_CONFIG_FILES([test/qsolve/api/Makefile])
AC_CONFIG_FILES([test/qsolve/gmp/Makefile])
AC_CONFIG_FILES([test/rays/Makefile])
AC_CONFIG_FILES([test/walk/Makefile])
AC_CONFIG_FILES([test/zbasis/Makefile])
AC_CONFIG_FILES([test/zsolve/Makefile])
AC_CONFIG_FILES([test/zsolve/api/Makefile])

AC_CONFIG_FILES([test/check.template], 		[chmod +x test/check.template])
AC_CONFIG_FILES([test/subdirsum], 		[chmod +x test/subdirsum])
AC_CONFIG_FILES([test/flavorsum], 		[chmod +x test/flavorsum])
AC_CONFIG_FILES([test/reportfailures], 		[chmod +x test/reportfailures])
AC_CONFIG_FILES([test/ppi/check-ppi], 		[chmod +x test/ppi/check-ppi])

## Check what components to enable
AC_ARG_ENABLE(groebner, AS_HELP_STRING([--disable-groebner], [Disable building the groebner components]),
			[enable_groebner=${enableval}], [enable_groebner=yes])
AC_ARG_ENABLE(zsolve, AS_HELP_STRING([--disable-zsolve], [Disable building the zsolve components]),
			[enable_zsolve=${enableval}], [enable_zsolve=yes])
AC_ARG_ENABLE(fiber, AS_HELP_STRING([--enable-fiber], [Disable building the fiber components]),
			[enable_fiber=${enableval}], [enable_fiber=no])
AM_CONDITIONAL([ENABLE_ZSOLVE], [ test x${enable_zsolve} == xyes ])
AM_CONDITIONAL([ENABLE_FIBER], [ test x${enable_fiber} == xyes ])

## Using the C compiler
AC_PROG_CC
AX_COMPILER_VENDOR
AX_CC_MAXOPT
AX_CFLAGS_WARN_ALL

## Using the C++ compiler
AC_PROG_CXX
# Check for specific compiler flags
AC_LANG(C++)
# Pick good default optimization flags
AX_COMPILER_VENDOR
AX_CXX_MAXOPT
# Turn on most compiler warnings
AX_CXXFLAGS_WARN_ALL
# Try std=c++0x to enable int32_t and int64_t for C++; note some old
# C++ compiler versions don't have this flag, but do support int32_t,
# int64_t
AX_CHECK_COMPILE_FLAG([-std=c++0x], [
                        CXXFLAGS="$CXXFLAGS -std=c++0x"])
# The flag -ftrapv means that arithmetic overflow checking is enabled.
CHECK_TRAPV

### Taken from GMP's configure.in:
# The dead hand of AC_REQUIRE makes AC_PROG_LIBTOOL expand and execute
# AC_PROG_F77, even when F77 is not in the selected with_tags.  This is
# probably harmless, but it's unsightly and bloats our configure, so pretend
# AC_PROG_F77 has been expanded already.
#
# FIXME: Rumour has it libtool will one day provide a way for a configure.in
# to say what it wants from among supported languages etc.
#
AC_PROVIDE([AC_PROG_F77])

# Change the default behaviour to disable shared libraries.  The user
# may still override this default by specifying `--enable-shared'.
# The reasons for the changed default is that the complexities of
# building shared libraries are avoided, and also that static build
# can result in faster code on some architectures.
AC_DISABLE_SHARED
# This triggers selection of some optimization flags (-g -O2), 
# so it should appear _after_ the AX_CXX_MAXOPT, AX_CC_MAXOPT calls.
AC_PROG_LIBTOOL

# Checks for header files.
#AC_HEADER_STDC
#AC_CHECK_HEADERS([malloc.h stdlib.h string.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_HEADER_STDBOOL
#AC_C_CONST

# Checks for library functions.
#AC_FUNC_MALLOC

# Check for GNU MP.
AC_LANG(C++)
LB_CHECK_GMP(,,[AC_MSG_WARN([GNU MP not found.
4ti2 will be built without support for arbitrary-precision computations.])])
if test -z "${GMP_HAVE_CXX}" ; then 
  AC_MSG_WARN([Unable to use the GNU MP C++ library.  
Please make sure that GNU MP was configured with "--enable-cxx".
4ti2 will be built without support for arbitrary-precision computations.])
fi

if test x${enable_groebner} == xyes ; then
# Check for GLPK.
LB_CHECK_GLPK(,,[
  AC_MSG_WARN([GNU Linear Programming Kit not found!
It is required for building the Groebner component of 4ti2.
4ti2 will be built without the Groebner component.])
  enable_groebner=no
])
fi
AM_CONDITIONAL([ENABLE_GROEBNER], [ test x${enable_groebner} == xyes ])

# Check whether the C99 and C++0xx int32_t and int64_t types work in C++.
# This requires the option -std=c++0xx on some modern g++ versions,
# but this option is unknown on some old g++ versions.
AC_MSG_CHECKING(whether we can use the int32_t and in64_t types)
AC_TRY_COMPILE(
  [ #include <cstdint>
  ], 
  [ int32_t xyzzy = 47; int64_t zyxxy = 11; 
  ],
  [ have_int32_t_int_64_t=yes;
  ], 
  [ have_int32_t_int_64_t=no;
  ])
AC_MSG_RESULT(${have_int32_t_int_64_t})
if test x${have_int32_t_int_64_t} = xyes; then
    AC_DEFINE(_4ti2_HAVE_INT32_T_INT64_T, 1, [Define if int32_t and int64_t are available])
else
    AC_MSG_WARN([Unable to use the int32_t and int64_t types in C++.])
fi
AM_CONDITIONAL([HAVE_INT32_INT64], [test x${have_int32_t_int_64_t} = xyes])

AM_CONDITIONAL([HAVE_GMP_WITH_CXX], [test -n "${GMP_HAVE_CXX}"])

if test -n "${GMP_HAVE_CXX}" ; then 

  # Definition for installed header file 4ti2_config.h
  AC_DEFINE([_4ti2_HAVE_GMP], [], [Define if GMP (with C++ interface) is installed])

  # Check whether we can convert between int32_t and int64_t and mpz_class (for the API).
  save_CXXFLAGS=${CXXFLAGS}
  CXXFLAGS="${CXXFLAGS} ${GMP_CFLAGS}"
  save_LIBS="${LIBS}"
  LIBS="${LIBS} ${GMP_LIBS}"
  AC_MSG_CHECKING(whether we can convert between int32_t and mpz_class)
  AC_TRY_LINK(
    [  #include <gmpxx.h>
       #include <cstdint>
    ],
    [  int32_t x = 815;
       mpz_class y(x);
    ],
    [ HAVE_MPZ_INT32_CONVERSION=yes;
      AC_DEFINE(_4ti2_HAVE_MPZ_INT32_CONVERSION, 1, [Define if we can convert between int32_t and mpz_class]) ],
    [ HAVE_MPZ_INT32_CONVERSION=no ])
  AC_MSG_RESULT(${HAVE_MPZ_INT32_CONVERSION})
  AC_MSG_CHECKING(whether we can convert between int64_t and mpz_class)
  AC_TRY_LINK(
    [  #include <gmpxx.h>
       #include <cstdint>
    ],
    [  int64_t x = 815;
       mpz_class y(x);
    ],
    [ HAVE_MPZ_INT64_CONVERSION=yes;
      AC_DEFINE(_4ti2_HAVE_MPZ_INT64_CONVERSION, 1, [Define if we can convert between int64_t and mpz_class]) ],
    [ HAVE_MPZ_INT64_CONVERSION=no ])
  AC_MSG_RESULT(${HAVE_MPZ_INT64_CONVERSION})
  LIBS="${save_LIBS}"
  CXXFLAGS="${save_CXXFLAGS}"
fi

if test x${have_int32_t_int_64_t} = xyes; then 
   GROEBNER_DEFAULT_EXECUTABLE=4ti2int64
elif test -n "${GMP_HAVE_CXX}" ; then
   GROEBNER_DEFAULT_EXECUTABLE=4ti2gmp
else
   AC_MSG_ERROR([Neither int32_t, int64_t, nor GMP integers are available.
4ti2 cannot be built.  Consider upgrading the compiler or
use the correct CXXFLAGS that enables these types.])
fi
AC_SUBST(GROEBNER_DEFAULT_EXECUTABLE)

## Check whether to build high-level-language interfaces using SWIG
AC_ARG_ENABLE(swig, AS_HELP_STRING([--enable-swig], [Enable building high-level-language interfaces using SWIG]),
		    [enable_swig=${enableval}], [enable_swig=no])
if test "$enable_swig" = yes; then
  # Call configure separately in this subdirectory.
  AC_CONFIG_SUBDIRS(swig)
  # If we use SWIG, we definitely want shared libraries.
  AC_ENABLE_SHARED
fi
AM_CONDITIONAL([ENABLE_SWIG], [ test x${enable_swig} == xyes ])

## Set some doxygen features
DX_HTML_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(4ti2, doxygen.cfg)

AC_OUTPUT
