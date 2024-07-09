##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

if BUILD_CH4_SHM_POSIX_EAGER_IQUEUE

noinst_HEADERS += src/mpid/ch4/shm/posix/eager/quicq/quicq_send.h \
                  src/mpid/ch4/shm/posix/eager/quicq/quicq_recv.h \
                  src/mpid/ch4/shm/posix/eager/quicq/posix_eager_inline.h

mpi_core_sources += src/mpid/ch4/shm/posix/eager/quicq/func_table.c \
                    src/mpid/ch4/shm/posix/eager/quicq/quicq_init.c

endif
