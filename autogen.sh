#! /bin/sh

mkdir build-aux \
&& aclocal \
&& automake --add-missing \
&& autoconf
