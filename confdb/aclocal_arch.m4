dnl Architecture related options and checking

AC_ARG_ENABLE(sse2,
    AS_HELP_STRING([Enable SSE2 instructions. Default is on]),,enable_sse2=yes)

AC_ARG_ENABLE(avx,
    AS_HELP_STRING([Enable AVX instructions. Default is off]),,enable_avx=no)

AC_ARG_ENABLE(avx512f,
    AS_HELP_STRING([Enable AVX512F instructions. Default is off]),,enable_avx512f=no)

if test "X$enable_sse2" = "Xyes"; then
    AC_CACHE_CHECK([whether -msse2 is supported], pac_cv_found_sse2,
                   [PAC_C_CHECK_COMPILER_OPTION([-msse2],pac_cv_found_sse2=yes,pac_cv_found_sse2=no)],
                   pac_cv_found_sse2=no,pac_cv_found_sse2=yes)
    if test "$pac_cv_found_sse2" = "yes" ; then
        PAC_APPEND_FLAG([-msse2],[CFLAGS])
        AC_DEFINE(HAVE_SSE2,1,[Define if SSE2 instructions are enabled])
    else
        enable_sse2=no
    fi
fi

if test "X$enable_avx" = "Xyes"; then
    AC_CACHE_CHECK([whether -mavx is supported], pac_cv_found_avx,
                   [PAC_C_CHECK_COMPILER_OPTION([-mavx],pac_cv_found_avx=yes,pac_cv_found_avx=no)],
                   pac_cv_found_avx=no,pac_cv_found_avx=yes)
    if test "$pac_cv_found_avx" = "yes" ; then
        PAC_APPEND_FLAG([-mavx],[CFLAGS])
        AC_DEFINE(HAVE_AVX,1,[Define if AVX instructions are enabled])
    else
        enable_avx=no
    fi
fi

if test "X$enable_avx512f" = "Xyes"; then
    AC_CACHE_CHECK([whether -mavx512f is supported], pac_cv_found_avx512f,
                   [PAC_C_CHECK_COMPILER_OPTION([-mavx512f],pac_cv_found_avx512f=yes,pac_cv_found_avx512f=no)],
                   pac_cv_found_avx512f=no,pac_cv_found_avx512f=yes)
    if test "$pac_cv_found_avx512f" = "yes" ; then
        PAC_APPEND_FLAG([-mavx512f],[CFLAGS])
        AC_DEFINE(HAVE_AVX512F,1,[Define if AVX512F instructions are enabled])
    else
        enable_avx512f=no
    fi
fi
