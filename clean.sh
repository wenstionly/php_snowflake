#!/bin/bash
[ -f Makefile ] && make distclean
rm -rf *.o .libs autom4te.cache build *.lo tools/isecret .deps Makefile.global acinclude.m4 aclocal.m4
rm -rf config.guess config.h* config.h.in config.nice config.sub
rm -rf Makefile Makefile.fragments Makefile.objects config.log config.status configure configure.ac configure.in
rm -rf install-sh libtool ltmain.sh missing mkinstalldirs run-tests.php
