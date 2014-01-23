#!/bin/bash

DIR=$1

SCRDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CHECKERAPP="${SCRDIR}/alliedness_checker.py"
WINTIMEAPP="${SCRDIR}/wintime.py"

WINTIME=0.0
ATTEMPTS=0
IFS=$'\n'

for subdir in $DIR/*; do
	if [ -d "$subdir" ] && [ `"$CHECKERAPP" ${subdir} | sed -e 's/.*opposing - \([0123456789]*\),.*/\1/'` -eq 0 ]; then
		ATTEMPTS=$((1+$ATTEMPTS))
		CURWINTIME=`"$WINTIMEAPP" "${subdir}/summary.log"`
		WINTIME=`calc "print $WINTIME + $CURWINTIME"`
	fi
done

WINTIME=`calc ${WINTIME}/$ATTEMPTS`
#echo Attempts $ATTEMPTS, wins $WINS, fraction $FRAC
echo $ATTEMPTS $WINTIME
