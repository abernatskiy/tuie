#!/bin/bash

MODE=4 # common TAU
DIR=$1

SCRDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CHECKERAPP="${SCRDIR}/alliedness_checker.py"

MODETIME=0
ALLTIME=0
IFS=$'\n'

for subdir in $DIR/*; do
	if [ -d "$subdir" ] && [ `"$CHECKERAPP" ${subdir} | sed -e 's/.*opposing - \([0123456789]*\),.*/\1/'` -eq 0 ]; then
		MODETIME=$((`cat "${subdir}/score_type.log" | awk '{print $10}' | grep -c 4`+$MODETIME))
		ALLTIME=$((`cat "${subdir}/score_type.log" | wc -l`+$ALLTIME))
	fi
done

FRAC=`calc ${MODETIME}/$ALLTIME`
echo "Out of $ALLTIME valid iterations $MODETIME ($FRAC) were spent in mode $MODE"
#echo $FRAC
