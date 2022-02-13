#!/bin/sh

par="--exec-prefix=/usr --prefix=/usr --libdir=/usr/lib/ksquirrel-libs"

echo "*** Doing configure $* $par ..."

./configure $* $par
