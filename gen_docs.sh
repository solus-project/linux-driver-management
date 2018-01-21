#!/bin/bash

# Credit to swupd developers: https://github.com/clearlinux/swupd-client

MANPAGES="man/mkmodaliases.1 man/linux-driver-management.1 man/ldm-session-init.1"

for MANPAGE in ${MANPAGES}; do \
    ronn --roff < ${MANPAGE}.md > ${MANPAGE}; \
    ronn --html < ${MANPAGE}.md > ${MANPAGE}.html; \
done
