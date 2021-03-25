#!/bin/bash

OUTPUT="public"
DATE=`date +"%d/%m/%Y %H:%m"`
MESSAGE="UPD: Rebuilding site: $DATE"

set -e

set -a
[ -f .env ] && . .env
set +a

if [ ! -d $OUTPUT ]
then
	echo "The $OUTPUT directory was not found. Please make sure that submodule is initalized."
	echo "git submodule update --init --recursive"
	exit 1;
fi

cd $OUTPUT
if [ "`git status -s`" ]
then
	echo "The $OUTPUT directory is dirty. Cleaning automatically."
	git reset --hard HEAD
	git clean -f
	git clean -fd
	if [ "`git status -s`" ]
	then
		echo "Could not clean properly. Please clean manually."
		exit 1;
	else
		echo "Clean successful."
	fi
fi

if [ "`git branch --show-current`" != "main" ]
then
	echo "$OUTPUT not set to main branch. Trying to set"
	git checkout main
	if [ "`git branch --show-current`" != "main" ]
	then
		echo "Could not change branch."
		exit 1;
	fi

fi

git reset --hard origin/clean

rsync -r ../build/html/ .

git add .
git commit -m "$MESSAGE"
git push origin main

