#!/bin/sh

par="--libdir=/usr/lib/ksquirrel-libs"

echo "Doing configure $* $par ..."

./configure $* $par
