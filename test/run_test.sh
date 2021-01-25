#!/bin/bash

printf_new() {
 str=$1
 num=$2
 v=$(printf "%-${num}s" "$str")
 printf "${v// / }"
}

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
COMMAND=$1
CHRLEN=${#COMMAND}
NBSP=`echo 32-${CHRLEN}|bc`

printf "Run $1"
printf_new " " $NBSP

$1 $2 $3 $4 $5 $6 $7 $8 $9 1>$1.log 2>&1

if [ $? -ne 0 ]
then
	printf "${RED}FAIL${NC}\n"
else
	printf "${GREEN}SUCCESS${NC}\n"
fi
