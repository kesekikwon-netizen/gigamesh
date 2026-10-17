#!/usr/bin/python

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


# Compute two feature vectors first, e.g.
#   gigamesh-featurevectors -f mesh/keilschrift_schnipsel.ply -o keilschrift_schnipsel_run01
#   gigamesh-featurevectors -f mesh/keilschrift_schnipsel.ply -o keilschrift_schnipsel_run02
# and compare with
#   ./compare_ftvecs.py keilschrift_schnipsel_run01_r1.00_n4_v256.volume.mat keilschrift_schnipsel_run02_r1.00_n4_v256.volume.mat 01_02

import argparse
import csv
import numpy as np

# Parse command line
parser = argparse.ArgumentParser( description='Compare two (mat) files with feature vectors from GigaMesh' )
parser.add_argument( 'filename1', type=str, help='1st filename' )
parser.add_argument( 'filename2', type=str, help='2nd filename' )
parser.add_argument( 'output_suffix', type=str, help='suffix for storing the differences' )
args = parser.parse_args()
fname_first = args.filename1
fname_secnd = args.filename2

# The number of scales i.e. elements of the feature vector is 16 by default
number_of_elements_in_feature_vec = 16

# Load feature vectors (1)
print( "Loading: " + fname_first )
data_first = np.recfromcsv( fname_first, delimiter=' ', filling_values=np.nan, case_sensitive=True, deletechars='', replace_space=' ' )

# Load feature vectors (2)
print( "Loading: " + fname_secnd )
data_secnd = np.recfromcsv( fname_secnd, delimiter=' ', filling_values=np.nan, case_sensitive=True, deletechars='', replace_space=' ' )

# Check for equal numbers of vectors
if len( data_first ) == len( data_secnd ):
	print "Both files contain the same number of feature vectors: " + str( len( data_first ) )
else:
	print "The numbers of feature vectors is NOT equal: " + str( len( data_first ) ) + " != " + str( len( data_second ) )
	quit()

# Compute differences between both files
diff_array = np.zeros(( len( data_first ), number_of_elements_in_feature_vec+1 ) )
diff_array_flip = np.zeros(( len( data_first ), number_of_elements_in_feature_vec+1 ) )
for line in range( 0, len( data_first ) ):
	line_content_first = data_first[line]
	line_content_secnd = data_secnd[line]
#	diff_str =      "Same: "
#	diff_str_flip = "Flip: "
#	diff_array[line][0] = line_content_first[0]
	for cellnr in range( 1, len( line_content_first ) ):
		diff = line_content_first[cellnr] - line_content_secnd[cellnr]
		diff_flip = line_content_first[cellnr] - line_content_secnd[len( line_content_first )-cellnr]
		diff_array[line][cellnr] = diff
		diff_array_flip[line][cellnr] = diff_flip
#		diff_str = diff_str + str( diff ) + " "
#		diff_str_flip = diff_str_flip + str( diff_flip ) + " "
#	print diff_str
#	print diff_str_flip
#	print "-------------------------"

difference_sum = np.sum( diff_array[:,1:] );
if difference_sum == 0.0:
	print "Both files are equal."
	quit()
print "Sum of differences same:    " + str( difference_sum ) + " average diff per vector: " + str( difference_sum / len( data_first ) )

difference_sum_flip = np.sum( diff_array_flip[:,1:] );
if difference_sum_flip == 0.0:
	print "Both files are equal, but one has flipped vectors."
	quit()
print "Sum of differences flipped: " + str( difference_sum_flip ) + " average diff per vector: " + str( difference_sum / len( data_first ) )

# Show the differences as mean, std, min and max
print "--- MEAN ----------"
print np.mean( diff_array[:,1:], 0 )
print "--- STD -----------"
print np.std( diff_array[:,1:], 0 )
print "--- MIN -----------"
print np.amin( diff_array[:,1:], 0 )
print "--- MAX -----------"
print np.amax( diff_array[:,1:], 0 )

# Write new file containing the differences of the feature vectors
filename_differences = "feature_vector_diff_" + args.output_suffix + ".mat"
with open( filename_differences, "w" ) as my_csv:   # writing the file as my_csv
    csvWriter = csv.writer(my_csv,delimiter=' ')        # using the csv module to write the file
    csvWriter.writerows( diff_array  )                  # write every row in the matrix
print "Differences of both files stored as '" + filename_differences +"'"
