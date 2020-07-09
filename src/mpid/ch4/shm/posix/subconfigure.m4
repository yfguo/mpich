[#] start of __file__
dnl MPICH_SUBCFG_BEFORE=src/mpid/common/shm
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4], [
    # always enable POSIX
    build_ch4_shm_posix=yes

    # the POSIX shmmod depends on the common shm code
    build_mpid_common_shm=yes
    ])
    AM_CONDITIONAL([BUILD_SHM_POSIX],[test "X$build_ch4_shm_posix" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_SHM_POSIX],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:shm:posix])
])dnl end AM_COND_IF(BUILD_SHM_POSIX,...)
])dnl end _BODY

[#] end of __file__
