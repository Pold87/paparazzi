all: trexton_regression ann


ann:
	gcc -o ann ann.c /home/pold/paparazzi/sw/airborne/modules/fann/src/floatfann.c -I/home/pold/paparazzi/sw/airborne/modules/fann/src/include -lm


trexton_regression:
	gcc -ansi -g -pthread -I. -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib -I/home/pold/paparazzi/sw/airborne/arch/linux -I/home/pold/paparazzi/sw/include -I ../particle_filter/src -Iopticflow/ -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib/v4l -o trexton_regression trexton_regression.c lib/v4l/v4l2.c image_conversions.c  lib/encoding/jpeg.c lib/encoding/rtp.c readcsv.c texton_helpers.c texton_helpers.h libcsv.c opticflow/edge_flow.c opticflow/math/*.c opticflow/opticflow_calculator.c opticflow/linear_flow_fit.c opticflow/size_divergence.c -Ilib/vision lib/vision/*.c ../particle_filter/src/particle_filter.c ../particle_filter/src/random_number_gen.c -lm readpng.c readpng.h lib/vision/image.h  -std=c99 -Wno-implicit-function-declaration -L/home/pold/Downloads/libpng -I/usr/local/include/libpng16 -L/usr/local/lib -I/home/pold/paparazzi/sw/airborne/ /home/pold/paparazzi/sw/airborne/arch/linux/udp_socket.c -I/home/pold/paparazzi/sw/airborne/arch/linux/ -I/home/pold/paparazzi/var/include/ -I/home/pold/paparazzi/sw/airborne/modules/gps /home/pold/paparazzi/sw/airborne/modules/gps/*.c /home/pold/paparazzi/sw/airborne/modules/fann/src/floatfann.c -I/home/pold/paparazzi/sw/airborne/modules/fann/src/include -lpng16


trexton_regression_prof:
	gcc -ansi -pg -pthread -I. -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib -I/home/pold/paparazzi/sw/airborne/arch/linux -I/home/pold/paparazzi/sw/include -I ../particle_filter/src -Iopticflow/ -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib/v4l -o trexton_regression_prof trexton_regression_prof.c lib/v4l/v4l2.c image_conversions.c  readcsv.c texton_helpers.c texton_helpers.h libcsv.c opticflow/edge_flow.c opticflow/math/*.c opticflow/opticflow_calculator.c opticflow/linear_flow_fit.c opticflow/size_divergence.c -Ilib/vision lib/vision/*.c ../particle_filter/src/particle_filter.c ../particle_filter/src/random_number_gen.c -lm readpng.c readpng.h lib/vision/image.h  -std=c99 -Wno-implicit-function-declaration -L/home/pold/Downloads/libpng -I/usr/local/include/libpng16 -L/usr/local/lib -I/home/pold/paparazzi/sw/airborne/  -I/home/pold/paparazzi/sw/airborne/arch/linux/ -I/home/pold/paparazzi/var/include/ -lpng16


trexton_evaluate:
	gcc -ansi -pg -pthread -I. -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib -I/home/pold/paparazzi/sw/airborne/arch/linux -I/home/pold/paparazzi/sw/include -I ../particle_filter/src -Iopticflow/ -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib/v4l -o trexton_evaluate trexton_evaluate.c lib/v4l/v4l2.c image_conversions.c  readcsv.c texton_helpers.c texton_helpers.h libcsv.c opticflow/edge_flow.c opticflow/math/*.c opticflow/opticflow_calculator.c opticflow/linear_flow_fit.c opticflow/size_divergence.c -Ilib/vision lib/vision/*.c ../particle_filter/src/particle_filter.c ../particle_filter/src/random_number_gen.c -lm readpng.c readpng.h lib/vision/image.h  -std=c99 -Wno-implicit-function-declaration -L/home/pold/Downloads/libpng -I/usr/local/include/libpng16 -L/usr/local/lib -I/home/pold/paparazzi/sw/airborne/  -I/home/pold/paparazzi/sw/airborne/arch/linux/ /home/pold/paparazzi/sw/airborne/modules/fann/src/floatfann.c -I/home/pold/paparazzi/sw/airborne/modules/fann/src/include -I/home/pold/paparazzi/var/include/ -lpng16


get_histograms:
	gcc -ansi -g -pthread -I. -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib -I/home/pold/paparazzi/sw/include -I ../particle_filter/src -Iopticflow/ -I/home/pold/paparazzi/sw/airborne/modules/computer_vision/lib/v4l -o get_histograms get_histograms.c lib/v4l/v4l2.c image_conversions.c  lib/encoding/jpeg.c  readcsv.c texton_helpers.c texton_helpers.h libcsv.c opticflow/edge_flow.c opticflow/math/*.c opticflow/opticflow_calculator.c opticflow/linear_flow_fit.c opticflow/size_divergence.c -Ilib/vision lib/vision/*.c ../particle_filter/src/particle_filter.c ../particle_filter/src/random_number_gen.c -lm readpng.c readpng.h lib/vision/image.h  -std=c99 -Wno-implicit-function-declaration -L/home/pold/Downloads/libpng -I/usr/local/include/libpng16 -L/usr/local/lib -I/home/pold/paparazzi/sw/airborne/ -I/home/pold/paparazzi/sw/airborne/arch/linux/ -I/home/pold/paparazzi/var/include/ -lpng16

run:
	./trexton_regression

clean:
	rm trexton_pprz trexton_regression get_histograms ann trexton_evaluate
