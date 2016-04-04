/*
 * Copyright (C) Volker Strobel
 *
 * This file is part of paparazzi
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file "modules/trexton/trexton.c"
 * @author Volker Strobel
 *
 * This file implements a localization technique based on textons - i.e. small characteristic image patches.
 */

#include "trexton.h"
#include "texton_helpers.h" /* Utilities for extracting textons and saving histograms */
# include "readcsv.h"
#include <limits.h>
//#include "subsystems/gps/gps_trexton.h"


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
//#include "state.h"
//#include "subsystems/abi.h"

#include "lib/v4l/v4l2.h"
#include "lib/vision/image.h"
#include "lib/encoding/jpeg.h"
#include "lib/encoding/rtp.h"
#include "udp_socket.h"

/* TODO: see if static is necessary here */
int histograms[NUM_CLASSES][NUM_HISTOGRAMS][NUM_TEXTONS];

/* //static int targets[1000]; /\* targets for classifier (machine learning)*\/ */
static char *classes[] = {"firefox", "logitech", "linux", "camel"};
static char training_data_path[] = "training_data/";


void trexton_init() {

  //gps_impl_init();

  /* Get textons -- that is the clustering centers */
  read_textons_from_csv(*textons, texton_filename);

  #if PREDICT

      /* Read histograms */
      int i = 0;
      char histogram_filename[256];
      for (i = 0; i < NUM_CLASSES; i++) {
        /* Create filename (add .csv to class name) */
        snprintf(histogram_filename, sizeof(histogram_filename), "%s%s%s", training_data_path, classes[i], ".csv");
        read_histograms_from_csv(*histograms[i], histogram_filename);
      }

  #endif


  /* Initialize the video device */
  trexton_dev = v4l2_init(STRINGIFY(TREXTON_DEVICE),
      TREXTON_DEVICE_SIZE,
      TREXTON_DEVICE_BUFFERS,
      V4L2_PIX_FMT_UYVY);
  if (trexton_dev == NULL) {
    printf("[treXton_module] Could not initialize the video device\n");
  }

  // Start the streaming on the V4L2 device
  if (!v4l2_start_capture(trexton_dev))
    printf("[treXton_module] Could not start capture of the camera\n");


  // Open udp socket
  udp_socket_create(&video_sock,
          STRINGIFY(VIEWVIDEO_HOST),
          VIEWVIDEO_PORT_OUT,
          -1,
          VIEWVIDEO_BROADCAST);

}

/* Main function for the texton framework. It s called with a
   frequency of 30 Hz*/
void trexton_periodic() {

  #if MEASURE_TIME
    /* clock_t start = clock(); */;
  static struct timeval t0, t1;
  gettimeofday(&t0, 0);
  #endif

  //parse_gps_trexton();

  /* Get the image from the camera */
  struct image_t img;
  v4l2_image_get(trexton_dev, &img);

  #if TREXTON_DEBUG
  /* Send JPG image over RTP/UDP */

  // Create a new JPEG image
  struct image_t img_jpeg;
  image_create(&img_jpeg,
         trexton_dev->w,
         trexton_dev->h,
         IMAGE_JPEG);

  jpeg_encode_image(&img, &img_jpeg, 70, FALSE);
  rtp_frame_send(&video_sock, /* UDP device */
                 &img_jpeg,
     0, /* Format 422 */
     70, /* Jpeg-Quality */
     0,  /* DRI Header */
     0); /* 90kHz time increment */
  #endif

  /* Calculate the texton histogram -- that is the frequency of
  characteristic image patches -- for this image */
  int texton_histogram[NUM_TEXTONS] = {0};
  get_texton_histogram(&img, texton_histogram, textons);

  #if SAVE_HISTOGRAM
    save_histogram(texton_histogram, HISTOGRAM_PATH);
  #elif PREDICT
    uint8_t class;
    class = predict_class(texton_histogram);
      printf("predicted class is: %s\n\n", classes[class]);
  #endif

  /* Free the image */
  v4l2_image_free(trexton_dev, &img);

  #if TREXTON_DEBUG
    /* Free RTP stream image */
    image_free(&img_jpeg);
  #endif

  #if MEASURE_TIME
    /* clock_t end = clock(); */
    /* float seconds = (float) (end - start) / CLOCKS_PER_SEC; */
    /* printf("%.10f\n", seconds); */
    gettimeofday(&t1, 0);
    long elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;

    printf("%ld ms\n", elapsed / 1000);
  #endif

}

int marker_comp (const struct marker *elem1, const struct marker *elem2)
{
  double f = elem1->dist;
  double s =  elem2->dist;
  if (f > s) return 1;
  if (f < s) return -1;
  return 0;
}


uint8_t predict_class(int *texton_hist){

  int h = 0, class_i = 0, j = 0;

  struct marker markers[NUM_CLASSES * NUM_HISTOGRAMS];

    /* Iterate over classes (e.g. firefox, camel, ...) and save distances */
    uint8_t c = 0;
    double dist;
    double min_dists[NUM_CLASSES];

    for (c = 0; c < NUM_CLASSES; c++) {
      min_dists[c] = MAX_POSSIBLE_DIST;
      /* Compare current texton histogram to all saved histograms for
   a certain class */
      for (h = 0; h < NUM_HISTOGRAMS; h++) {
  dist = euclidean_dist_int(texton_hist, histograms[c][h], NUM_TEXTONS);

  /* Create marker */
  struct marker m;
  m.id = c;
  m.dist = dist;
  markers[j] = m;

  //printf("%s: %f\n", classes[c], dist);
  if (dist <= min_dists[c])
    min_dists[c] = dist;

  j++;
      }
    }

    /* Sort distances */
    qsort(markers, sizeof(markers) / sizeof(*markers), sizeof(*markers), marker_comp);

    printf("marker1: %d marker2: %d marker3: %d\n", markers[0].id, markers[1].id, markers[2].id);
    printf("marker1: %s %f marker2: %s %f marker3: %s %f\n", classes[markers[0].id], markers[0].dist,
     classes[markers[1].id], markers[1].dist,
     classes[markers[2].id], markers[2].dist);


    int use_only_nearest = 0;
    if (use_only_nearest) {
      double min_dist_all_classes = MAX_POSSIBLE_DIST;
      for (c = 0; c < NUM_CLASSES; c++) {

  /* Print distance for each class */
  printf("%s: %f\n", classes[c], min_dists[c]);

  if (min_dists[c] < min_dist_all_classes) {
    class_i = c;
    min_dist_all_classes = min_dists[c];
  }
      }
      printf("\n");
    } else if (knn) {

      /* Determine frequency of each neighbor */
      int k = 0;
      int nearest_markers[NUM_CLASSES] = {0};
      for (k = 0; k < knn; k++) {
  struct marker current_marker = markers[k];
  /* Increase counter for this marker */
  nearest_markers[current_marker.id]++;
      }

      /* Find arg max (that is, the neighbor with the highest
   frequency) */
      class_i = arg_max(nearest_markers, NUM_CLASSES);
    }

    return class_i;

}


int main()
{

  trexton_init();

  while(1)
    trexton_periodic();

  return 0;
}
