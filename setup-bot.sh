#!/bin/bash

botname=$1
cd `dirname $0`
rootdir=`pwd`

mkdir "bot-$botname"

sed "s/BOTNAME/$botname/g" example-bots/Makefile.template > bot-$botname/Makefile
cp example-bots/bcb-client.c bot-$botname/
cp example-bots/bcb-client.h bot-$botname/
sed "s/BOTNAME/$botname/g" example-bots/bot-template.c > bot-$botname/$botname.c
