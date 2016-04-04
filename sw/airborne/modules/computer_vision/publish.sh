echo "Release script"

ftp -inv 192.168.1.1 <<END_SCRIPT
put /home/pold/paparazzi/sw/airborne/modules/computer_vision/textons.csv data/video/paparazzi
END_SCRIPT
