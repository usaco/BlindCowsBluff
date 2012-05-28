#!/bin/bash

botname=$*
botfile=`echo $botname | sed "s/[^a-zA-Z0-9]//g"`
botdir=bot-$botfile

cd `dirname $0`
rootdir=`pwd`

mkdir $botdir

sed "s/BOTNAME/$botfile/g" client/Makefile.template > $botdir/Makefile
sed "s/BOTNAME/$botname/g" client/bot-template.c > $botdir/$botfile.c

cd $botdir
ln -s ../client/bcb-client.c
ln -s ../client/bcb-client.h
