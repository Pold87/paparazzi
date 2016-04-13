echo "Release script"

ftp -inv 192.168.42.1 <<END_SCRIPT
put /home/pold/paparazzi/sw/airborne/modules/computer_vision/textons.csv internal_000/
END_SCRIPT
