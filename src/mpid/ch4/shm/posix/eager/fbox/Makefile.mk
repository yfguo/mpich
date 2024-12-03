##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

if BUILD_CH4_SHM_POSIX_EAGER_FBOX

noinst_HEADERS += src/mpid/ch4/shm/posix/eager/fbox/fbox_send.h \
                  src/mpid/ch4/shm/posix/eager/fbox/fbox_recv.h \
                  src/mpid/ch4/shm/posix/eager/fbox/posix_eager_inline.h

mpi_core_sources += src/mpid/ch4/shm/posix/eager/fbox/func_table.c \
                    src/mpid/ch4/shm/posix/eager/fbox/fbox_init.c

endif
