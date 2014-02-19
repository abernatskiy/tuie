#!/usr/bin/python2

import os
import sys
import re

path_to_tuie = re.sub('utils/pair_viewer.py', '', os.path.abspath(sys.argv[0]))
path_to_pair0 = path_to_tuie + 'SavedFiles/pair0.dat_0'
#output = ' > /dev/null 2>&1'
#output = ' > /tmp/out 2>&1'
output = ''


os.system('mv \"' + path_to_pair0 + '\" \"' + path_to_pair0 + '.bc\"' + output)
os.system('cp \"' + sys.argv[1] + '\" \"' + path_to_pair0 + '\"')
os.system('cd \"' + path_to_tuie + '\"')
os.system('\"' + path_to_tuie + 'runclient\"' + output + '; rm \"' + path_to_pair0 + '\" \"' + path_to_tuie + 'SavedFiles/pref\*.dat\" \"' + path_to_tuie + 'SavedFiles/surrogate*.log\"' + output)
