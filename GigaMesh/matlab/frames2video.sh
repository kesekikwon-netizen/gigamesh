#!/bin/bash
ffmpeg -i spherical_filtermask_applied_frame_%05d.png -s 720x576 -b 6000k spherical_filtermask_transformation_applied.mpg