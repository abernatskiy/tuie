#!/bin/bash

MODE=4 # common TAU
DIR=$1

CHECKERAPP="${HOME}/josh/apps/utils/alliedness_checker.py"

MODETIME=0
ALLTIME=0
IFS=$'\n'

for subdir in $DIR/*; do
	if [ -d "$subdir" ] && [ `"$CHECKERAPP" ${subdir} | sed -e 's/.*opposing - \([0123456789]*\),.*/\1/'` -eq 0 ]; then
		MODETIME=$((`grep -c 4$ "${subdir}/score_type.log"`+$MODETIME))
		ALLTIME=$((`cat "${subdir}/score_type.log" | wc -l`+$ALLTIME))
	fi
done

FRAC=`calc ${MODETIME}/$ALLTIME`
echo "Out of $ALLTIME valid iterations $MODETIME ($FRAC) were spent in mode $MODE"
echo $FRAC
