#!/bin/bash
#
# GigaMesh - The GigaMesh Software Framework is a modular software for display,
# editing and visualization of 3D-data typically acquired with structured light or
# structure from motion.
# Copyright (C) 2009-2020 Hubert Mara
#
# This file is part of GigaMesh.
#
# GigaMesh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GigaMesh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
#
# Strips the DPI value from the filename and encodes it into the file.
if [ ! -d converted ]; then
	mkdir converted
fi
for FILEVIEW in *_*DPI.png; do 
	SETDPI=`echo $FILEVIEW | sed 's/.*_\([0-9]*\)DPI.*/\1/'`
	FILENAMENEW=`echo $FILEVIEW | sed 's/\(.*\)_[0-9]*DPI\(.*\)/\1\2/'`
	SETPPCM=`echo "scale=2; 800*100/254" | bc`
	echo $FILEVIEW will be set to $SETPPCM px/cm $SETDPI DPI and saved as $FILENAMENEW
	COMMENT=`echo Exported from GigaMesh, original filename: $FILEVIEW`
	convert $FILEVIEW -units PixelsPerCentimeter -density $SETPPCM -set Comment "$COMMENT" $FILENAMENEW
	if [ $? ]; then
		mv $FILEVIEW converted
		echo Conversion done - moved to converted/$FILEVIEW.
	else
		echo Conversion failed.
	fi
done
