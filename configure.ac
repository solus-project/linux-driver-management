AC_INIT([linux-driver-management], 0.0, [ikey@solus-project.com], [linux-driver-management], [https://github.com/solus-project/linux-driver-management/issues])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz foreign subdir-objects])
AC_PROG_CC
AC_PROG_CC_STDC
LT_PREREQ(2.2)
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr/local)
AM_SILENT_RULES([yes])
LT_INIT([disable-static])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile])

m4_define([pci_required_version], [0.13.4])

PKG_CHECK_MODULES(CORE,
        [
         pciaccess >= pci_required_version
        ]
)

AC_OUTPUT

AC_MSG_RESULT([
        linux-driver-management $VERSION
        ========

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        sysconfdir:             ${sysconfdir}
        exec_prefix:            ${exec_prefix}
        bindir:                 ${bindir}
        datarootdir:            ${datarootdir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
