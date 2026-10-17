#!/bin/bash
IMAGE_OVERLAY='image_overlay.png'
#rm -f watermarked/*
#mkdir watermarked
find . -type f -name Keilschrift\*.png -print0 | while IFS= read -r -d $'\0' IMAGE; do
#	echo $IMAGE
	IMAGE_PATH=`dirname ${IMAGE}`
	mkdir ${IMAGE_PATH}_FCGL
	IMAGE_NEW=`basename ${IMAGE} | rev | cut -d'.' -f2- | rev`
	echo ${IMAGE_PATH}_FCGL/${IMAGE_NEW}.png
	composite -gravity SouthEast -geometry +5+5 ${IMAGE_OVERLAY} ${IMAGE} ${IMAGE_PATH}_FCGL/${IMAGE_NEW}.png
done

