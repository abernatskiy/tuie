#!/bin/bash

DIR=$1

SCRDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

"$SCRDIR"/wintime_calc.sh "$DIR"
"$SCRDIR"/avg_quality_calc.sh "$DIR"
"$SCRDIR"/mode_time_calc.sh "$DIR"
"$SCRDIR"/success_calc.sh "$DIR"

