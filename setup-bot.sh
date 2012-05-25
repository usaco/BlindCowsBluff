#!/bin/bash

botname=$*
botfile=`echo $botname | sed "s/[^a-zA-Z0-9]//g"`
botdir=bot-$botfile

cd `dirname $0`
rootdir=`pwd`

mkdir $botdir

sed "s/BOTNAME/$botfile/g" example-bots/Makefile.template > $botdir/Makefile
cp example-bots/bcb-client.c $botdir/
cp example-bots/bcb-client.h $botdir/
sed "s/BOTNAME/$botname/g" example-bots/bot-template.c > $botdir/$botfile.c
