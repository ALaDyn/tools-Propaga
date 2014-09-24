#! /bin/bash
# Copyright 2010, 2011, 2012, 2013, 2014 Stefano Sinigardi
# The program is distributed under the terms of the GNU General Public License 
# This file is part of "Propaga_dataproc".
# Propaga_dataproc is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# Propaga_dataproc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with Propaga_dataproc.  If not, see <http://www.gnu.org/licenses/>.
#===================
rm xyz.emitt.dat
STDIN_FILE=stdin_SCAN
declare -a STDIN
STDIN=(`head -2 ${STDIN_FILE} | tail -1`)
#pay attention to have put the full maximum number with 7 chars
#in the stdin file, ex. 0005200 and not just 5200
#nb: DO not read that file with ANSI C because a 0 at the beginning means octal!
#
STEP_WIDTH=${STDIN[0]}
declare -a TUTTI
TUTTI=(`ls -1 *.???????.ppg`)
for file_input in ${TUTTI[*]}
do
./dataproc $file_input xyz.emitt.dat -3 ${STEP_WIDTH}
done
