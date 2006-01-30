#!/bin/sh


if [ $1 ]; then
	cd $1
fi

autoheader
aclocal -I ./
autoconf
automake --add-missing
