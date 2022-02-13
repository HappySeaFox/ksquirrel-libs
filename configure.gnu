#!/bin/sh

#par="--exec-prefix=/usr --prefix=/usr --libdir=/usr/lib/ksquirrel"
par="--prefix=/usr"

echo "*** Doing configure $* $par ..."

./configure $* $par
