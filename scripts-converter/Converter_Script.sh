#! /bin/tcsh
# Copyright 2010, 2011, 2012, 2013, 2014 Stefano Sinigardi
# The program is distributed under the terms of the GNU General Public License 
# This file is part of "Propaga_converter".
# Propaga_converter is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# Propaga_converter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with Propaga_converter.  If not, see <http://www.gnu.org/licenses/>.
set TUTTI=(`ls -1 15giugno.*`)
foreach file_input($TUTTI)
converter 1 1 2 $file_input
end
