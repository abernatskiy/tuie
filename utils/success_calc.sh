#!/bin/bash

DIR=$1

SCRDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CHECKERAPP="${SCRDIR}/alliedness_checker.py"

LFW=20

WINS=0
ATTEMPTS=0
IFS=$'\n'

rm wins.log

for subdir in $DIR/*; do
	if [ -d "$subdir" ] && [ `"$CHECKERAPP" ${subdir} | sed -e 's/.*opposing - \([0123456789]*\),.*/\1/'` -eq 0 ]; then
		ATTEMPTS=$((1+$ATTEMPTS))
		if [ `tail -$LFW ${subdir}/summary.log | awk '{print $5}' | grep 0 | wc -l` -eq $LFW ]; then
			WINS=$((1+$WINS))
			echo 1 >> wins.log
		else
			echo 0 >> wins.log
		fi
	fi
done

FRAC=`calc ${WINS}/$ATTEMPTS`
echo Attempts $ATTEMPTS, wins $WINS, fraction $FRAC
#echo $ATTEMPTS $WINS $FRAC
#echo Wintimes:
#"${SCRDIR}/ssc.py" wins.log
