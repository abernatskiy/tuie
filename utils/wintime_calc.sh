#!/bin/bash

DIR=$1

SCRDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CHECKERAPP="${SCRDIR}/alliedness_checker.py"
WINTIMEAPP="${SCRDIR}/wintime.py"

WINTIME=0.0
ATTEMPTS=0
IFS=$'\n'

mv wintime.log wintime.log.`date +%s` > /dev/null 2>&1
mv gens.log gens.log.`date +%s` > /dev/null 2>&1

for subdir in $DIR/*; do
	if [ -d "$subdir" ] && [ `"$CHECKERAPP" ${subdir} | sed -e 's/.*opposing - \([0123456789]*\),.*/\1/'` -eq 0 ]; then
		GENERATIONS=`tail -1 "${subdir}/summary.log" | awk '{print $1}'`
		if [ $GENERATIONS -lt 800 ]; then
			echo "crash detected for $subdir"
		fi
		echo $GENERATIONS >> gens.log

		ATTEMPTS=$((1+$ATTEMPTS))
		CURWINTIME=`"$WINTIMEAPP" "${subdir}/summary.log"`
		echo $CURWINTIME >> wintime.log
#		echo $CURWINTIME
		WINTIME=`calc "print $WINTIME + $CURWINTIME"`
	fi
done

#WINTIME=`calc ${WINTIME}/$ATTEMPTS`
#echo Attempts $ATTEMPTS, wins $WINS, fraction $FRAC
#echo $ATTEMPTS $WINTIME

echo Wintime:
"${SCRDIR}/ssc.py" wintime.log
echo Generations:
"${SCRDIR}/ssc.py" gens.log | head -1
