#!bin/bash

FILE_NAME=$1
ROW_COUNT=$2

ROW_STRING=""

if ! [ -z "$FILE_NAME" ]; then


	if (( "$ROW_COUNT" < 10000000 )); then

		if (( $ROW_COUNT > 1 )); then

			for rownumber in $(seq 1 $ROW_COUNT ); do
				ROW_STRING=""
				for coloumnnumber in $(seq 1 4); do
					RANDOM_VALUE=$(bc <<< "scale=3; $(( ( RANDOM % 2000 ) - 1000 ))/1000" | sed -r 's/^(-?)\./\10./')
					ROW_STRING="${ROW_STRING} ${RANDOM_VALUE}"
				done
				echo $ROW_STRING >> $FILE_NAME
			done
		else
			echo "Error: Need positive integer less than 10000000 as second argument"
		fi

	else
		echo "Error: Need positive integer less than 10000000 as second argument"
	fi
else
	echo "Error: Need filename as first  argument";
fi
