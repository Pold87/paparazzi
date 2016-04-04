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
 * treXton localization
 */

#include "modules/trexton/trexton.h"
#include "subsystems/gps/gps_trexton.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../computer_vision/lib/v4l/v4l2.h"
#include "../computer_vision/lib/vision/image.h"
#include "../computer_vision/lib/encoding/jpeg.h"
#include "../computer_vision/lib/encoding/rtp.h"


/* The video device */
#ifndef TREXTON_DEVICE
#define TREXTON_DEVICE /dev/video2      ///< The video device
#endif


/* The video device size (width, height) */
#ifndef TREXTON_DEVICE_SIZE
#define TREXTON_DEVICE_SIZE 320,240     ///< The video device size (width, height)
#endif


/* The video device buffers (the amount of V4L2 buffers) */
#ifndef TREXTON_DEVICE_BUFFERS
#define TREXTON_DEVICE_BUFFERS 15       ///< The video device buffers (the amount of V4L2 buffers)
#endif


static struct v4l2_device *trexton_dev;          ///< The trexton camera V4L2 device

void trexton_init() {
  gps_impl_init();


  /* Initialize the video device */
  trexton_dev = v4l2_init(STRINGIFY(TREXTON_DEVICE), 
			TREXTON_DEVICE_SIZE, 
			TREXTON_DEVICE_BUFFERS, 
			V4L2_PIX_FMT_UYVY);
  if (trexton_dev == NULL) {
    printf("[treXton_module] Could not initialize the video device\n");
  }
}

void trexton_periodic() {
  parse_gps_trexton();


  // Start the streaming on the V4L2 device
  if (!v4l2_start_capture(trexton_dev)) {
    printf("[treXton_module] Could not start capture of the camera\n");

  }


#if TREXTON_DEBUG
  // Create a new JPEG image
  struct image_t img_jpeg;
  image_create(&img_jpeg, 
	       trexton_dev->w, 
	       trexton_dev->h, 
	       IMAGE_JPEG);
#endif


  /* Main loop of the optical flow calculation */
  while (TRUE) {
    // Try to fetch an image
    struct image_t img;
    v4l2_image_get(trexton_dev, &img);


#if TREXTON_DEBUG
    jpeg_encode_image(&img, &img_jpeg, 70, FALSE);
    rtp_frame_send(
      &VIEWVIDEO_DEV,           // UDP device
      &img_jpeg,
      0,                        // Format 422
      70, // Jpeg-Quality
      0,                        // DRI Header
      0                         // 90kHz time increment
    );
#endif

    // Free the image
    v4l2_image_free(trexton_dev, &img);

  }

#if OPTICFLOW_DEBUG
  image_free(&img_jpeg);
#endif
}
