#include "texton_settings.h"
#include "texton_helpers.h"
#include "trexton_regression.h"
#include "image_conversions.h"

#include <stdio.h>

#include "/home/pold/Documents/Internship/particle_filter/src/particle_filter.h"

#include "lib/v4l/v4l2.h"
#include "lib/vision/image.h"
#include "lib/encoding/jpeg.h"
#include "lib/encoding/rtp.h"
#include "udp_socket.h"

#include "opticflow_module.h"
#include "opticflow/edge_flow.h"
#include "opticflow/opticflow_calculator.h"

#include "floatfann.h"

/* Histogram paths and settings */ 
static char histogram_filename[] = "mat_train_hists.csv";
static char histogram_filename_testset[] = "mat_test_hists_str8.csv";
static char position_filename[] =  "board_train_pos.csv";
static char test_position_filename[] =  "predictions_cross.csv";
static struct measurement all_positions[NUM_HISTOGRAMS];
static struct measurement all_test_positions[NUM_TEST_HISTOGRAMS];

#if USE_COLOR
  #define SIZE_HIST NUM_COLOR_BINS * COLOR_CHANNELS
#else
  #define SIZE_HIST NUM_TEXTONS * CHANNELS
#endif

static int regression_histograms[NUM_HISTOGRAMS][SIZE_HIST];
static double regression_histograms_color[NUM_HISTOGRAMS][SIZE_HIST];
static int histograms_testset[NUM_TEST_HISTOGRAMS][SIZE_HIST];

static int current_test_histogram = 0;

static double ws_x[NUM_TEXTONS * CHANNELS];
static double ws_y[NUM_TEXTONS * CHANNELS];

static int use_variance = 0;

/* Create  particles */
struct particle particles[N];

/* The main opticflow variables */
struct opticflow_t opticflow;                      ///< Opticflow calculations
static struct opticflow_result_t opticflow_result; ///< The opticflow result
static struct opticflow_state_t opticflow_state;   ///< State of the drone to communicate with the opticflow
static pthread_mutex_t opticflow_mutex;            ///< Mutex lock fo thread safety
static bool_t opticflow_got_result; ///< When we have an optical flow calculation

static int image_num = 0;

#define USE_FLOW true

void trexton_init() {

  /* Get textons -- that is the clustering centers */
  read_textons_from_csv(textons, texton_filename);

  /* Remove predictions file */
  remove("particle_filter_preds.csv");
  remove("edgeflow_diff.csv");

  // Set the opticflow state to 0
  opticflow_state.phi = 0;
  opticflow_state.theta = 0;
  opticflow_state.agl = 0;

  // Initialize the opticflow calculation
  opticflow_calc_init(&opticflow, TREXTON_DEVICE_SIZE);

  opticflow_got_result = FALSE;

  #if PREDICT
    /* Read histograms */
    /* read_histograms_from_csv(regression_histograms, histogram_filename, SIZE_HIST); */
    read_color_histograms_from_csv(regression_histograms_color, histogram_filename, SIZE_HIST);

  int i, j;
  for (i = 0; i < NUM_HISTOGRAMS; i++) {
    for (j = 0; j < NUM_COLOR_BINS * COLOR_CHANNELS; j++) {
      printf("%f ", regression_histograms_color[i][j]);
    }
    printf("\n");
  }


  
  read_positions_from_csv(all_positions, position_filename);
    /* read_predictions_from_csv(all_test_positions, test_position_filename, NUM_TEST_HISTOGRAMS, 0); */
    /* read_weights_from_csv(ws_x, "ws_x.csv"); */
    /* read_weights_from_csv(ws_y, "ws_y.csv"); */

  #endif

    /* read_test_histograms_from_csv(histograms_testset, histogram_filename_testset); */

   /* Write header for predictions file*/
    remove("predictions.csv");
    FILE *fp_predictions;
    fp_predictions = fopen("predictions.csv", "a");
    fprintf(fp_predictions, "id,x,y,dist\n");
    fclose(fp_predictions);

    /* Initialize particles*/
    init_particles(particles);

    /* Debugging read poitions from csv */
    /* int i; */
    /* for (i = 0; i < 10; i++) { */
    /*   printf("is is %d pos x is %f, pos y is %f\n", i, all_positions[i].x, all_positions[i].y); */
    /* } */


}

/* Main function for the texton framework. It s called with a
   frequency of 30 Hz*/
void trexton_periodic() {

  #if MEASURE_TIME
    /* clock_t start = clock(); */;
  static struct timeval t0, t1, tot;
  long elapsed;
  gettimeofday(&tot, 0);
  #endif


  /* Calculate the texton histogram -- that is the frequency of
    characteristic image patches -- for this image */

  struct image_t img;

  #if false
    /* Get the image from the caera */
  v4l2_image_get(trexton_dev, &img);

  #if USE_CONVERSIONS
  
    /* Create RGB image */
    struct image_t rgb_img, opp_img, std_img;
    image_create(&rgb_img, 320, 240, IMAGE_RGB);
    image_create(&opp_img, 320, 240, IMAGE_OPPONENT);
    image_create(&std_img, 320, 240, IMAGE_STD);
    YUV422toRGB(&img, &rgb_img);
    double means[8];
    RGBtoOpponent(&rgb_img, &opp_img, means);
    image_grayscale_standardize(&opp_img, &std_img, means);
    image_grayscale_standardize(&img, &std_img, means);
    printf("Means are %f, %f, %f %f\n", means[0], means[1], means[2], means[3]);

    uint8_t *rgb_buf = (uint8_t *)rgb_img.buf;
    uint8_t *opp_buf = (uint8_t *)opp_img.buf;
    double *std_buf = (double *)std_img.buf;

    printf("RGB: %d %d\n", rgb_buf[0], rgb_buf[1]);
    printf("Opponent: %d %d %d %d\n", opp_buf[0], opp_buf[1], opp_buf[2], opp_buf[3]);
    printf("STD: %f %f %f %f\n", std_buf[0], std_buf[1], std_buf[2], std_buf[3]);
  #endif
    
  #else
    struct image_t rgb_img, opp_img, std_img;
    image_create(&rgb_img, 320, 240, IMAGE_RGB);
    image_create(&img, 320, 240, IMAGE_YUV422);

    char image_path[1024];
    sprintf(image_path, "/home/pold/Documents/Internship/datasets/board_str8_test/%d.png", image_num);

    printf("%s", image_path);
    fflush(stdout);

    read_png_file(image_path, &rgb_img);
    RGBtoYUV422(&rgb_img, &img);
    
  #endif

  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec) * 1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed first part: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif


  int texton_histogram[NUM_TEXTONS * NUM_TEXTONS] = {0};
  get_texton_histogram(&img, texton_histogram, textons);

  double color_hist[COLOR_CHANNELS * NUM_COLOR_BINS] = {0};
  get_color_histogram(&img, color_hist, NUM_COLOR_BINS);

  
  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec) * 1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed get histogram: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif


  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed second part: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif


 #if SAVE_HISTOGRAM
    save_histogram(texton_histogram, HISTOGRAM_PATH);
 #elif PREDICT

  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed after get histogram: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif


  int i;
  printf("COLOR\n");
  for (i = 0; i < COLOR_CHANNELS * NUM_COLOR_BINS; i++) {
     printf("%f ", color_hist[i]);
  }
  printf("\n");
  
   /* TreXton prediction */
  struct measurement pos, pos_knn;
    /* For textons */
  /* pos = predict_position(texton_histogram, NUM_TEXTONS * CHANNELS); */
  
    /* For colors */
    pos = predict_position(color_hist, COLOR_CHANNELS * NUM_COLOR_BINS);
  
  /* For textons */
  /* pos = predict_fann(texton_histogram, NUM_TEXTONS * CHANNELS); */

  /* For colors */
  /* pos = predict_fann(texton_histogram, NUM_COLOR_BINS * COLOR_CHANNELS); */
   /* pos = linear_regression_prediction(texton_histogram); */
   printf("\nPOSITION IS x:%f y:%f\n", pos.x, pos.y);
  fflush(stdout);   

  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed predict position: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif
   //pos = all_test_positions[current_test_histogram];
   /* save_image(&img, "afterpredict.csv"); */

   /* Optical flow prediction */
   /* TODO */

   /* Particle filter update */
   struct measurement flow;

   #if USE_FLOW
    // Copy the state
    pthread_mutex_lock(&opticflow_mutex);
    struct opticflow_state_t temp_state;
    memcpy(&temp_state, &opticflow_state, sizeof(struct opticflow_state_t));
    pthread_mutex_unlock(&opticflow_mutex);
    // Do the optical flow calculation
    struct opticflow_result_t temp_result;

    edgeflow_calc_frame(&opticflow, &temp_state, &img, &temp_result);
    /* opticflow_calc_frame(&opticflow, &temp_state, &img, &temp_result); */
    printf("\n edgeflow result: x:%d y:%d\n", temp_result.flow_x, temp_result.flow_y);
// Copy the result if finished
    pthread_mutex_lock(&opticflow_mutex);
    memcpy(&opticflow_result, &temp_result, sizeof(struct opticflow_result_t));
    opticflow_got_result = TRUE;
    pthread_mutex_unlock(&opticflow_mutex);

    /* if (image_num == 10) { */
    /*   save_image(&std_img, "mainpic.csv");  */
    /* } */

    /* flow.y =  2.5 * ((double) opticflow_result.flow_x) / 1000; */
    /* flow.x =  - 2.5 * ((double) opticflow_result.flow_y) / 1000; */

    flow.y =  2.5 * ((double) opticflow_result.flow_x);
    flow.x =  2.5 * ((double) opticflow_result.flow_y);

    /* flow.y =  - 1.5 * ((double) opticflow_result.flow_x); */
    /* flow.x = -  1.5 * ((double) opticflow_result.flow_y); */
   
    printf("flow is %f", flow.x);
    particle_filter(particles, &pos, &flow, use_variance, 1);
    opticflow_got_result = FALSE;
   #else
     particle_filter(particles, &pos, &flow, use_variance, 0);
   #endif

  #if MEASURE_TIME
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
    printf("Elapsed flow part: %ld ms\n", elapsed / 1000);
  gettimeofday(&t0, 0);
  #endif

   struct particle p_forward = weighted_average(particles, N);
   /* printf("\nRaw: %f,%f\n", pos.x, pos.y); */
   printf("Particle filter: %f,%f\n", p_forward.x, p_forward.y);


   FILE *fp_predictions;
   FILE *fp_particle_filter;
   FILE *fp_edge;
   fp_predictions = fopen("predictions.csv", "a");
   fp_particle_filter = fopen("particle_filter_preds.csv", "a");
   fp_edge = fopen("edgeflow_diff.csv", "a");
   fprintf(fp_edge, "%f,%f\n", flow.x, flow.y);
   fprintf(fp_particle_filter, "%f,%f\n", p_forward.x, p_forward.y);
   //fprintf(fp_predictions, "%d,%f,%f,%f\n", current_test_histogram, pos.x, pos.y, pos.dist);
   fclose(fp_predictions);
   fclose(fp_particle_filter);
   fclose(fp_edge);
 #endif

   current_test_histogram++;

  #if !EVALUATE

   #if USE_CONVERSIONS
    /* Free the image */
    image_free(&rgb_img);
    image_free(&opp_img);
    image_free(&std_img);
   #endif
  #if false
  v4l2_image_free(trexton_dev, &img);
  #else
  image_free(&img);
  image_free(&rgb_img);
  #endif

  #endif

  image_num = image_num + 1;

  #if MEASURE_TIME
    /* clock_t end = clock(); */
    /* float seconds = (float) (end - start) / CLOCKS_PER_SEC; */
    /* printf("%.10f\n", seconds); */
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec-tot.tv_sec)*1000000 + t1.tv_usec-tot.tv_usec;

    printf("TOTAL ELAPSED (entire function) %ld ms\n", elapsed / 1000);
  #endif


}

/**
 * Predict the x, y position of the UAV using the texton histogram.
 *
 * @param texton_hist The texton histogram
 *
 * @return The x, y, position of the MAV, computed by means of the input histogram
 */
struct measurement predict_position(double hist[], int hist_size) {

  int h = 0; /* Histogram iterator variable */

  struct measurement measurements[NUM_HISTOGRAMS];
  double dist;

  /* Compare current texton histogram to all saved histograms for
     a certain class */
  for (h = 0; h < NUM_HISTOGRAMS; h++) {
    /* dist = euclidean_dist_int(hist, regression_histograms[h], hist_size); */
    #if USE_COLOR
      dist = chi_square_dist_double(hist, regression_histograms_color[h], hist_size);
    #else
      dist = euclidean_dist_int(hist, regression_histograms[h], hist_size);
    #endif
    
    /* printf("dist is %d %f\n", h, dist); */
    /* printf("all pos is %f\n", all_positions[h].x); */
    
    struct measurement z;
    z.x = all_positions[h].x;
    z.y = all_positions[h].y;
    z.hist_num = h;
    z.dist = dist;
    measurements[h] = z;

    /* printf("H is %d, dist is %f\n", h, dist); */
    
  }

    /* Sort distances */
  qsort(measurements, sizeof(measurements) / sizeof(*measurements), sizeof(*measurements), measurement_comp);
  
  /* Return average over first positions for accurate regression: */

  int k = 3, l;
  struct  measurement mean_pos;
  mean_pos.x = 0;
  mean_pos.y = 0;
  mean_pos.dist = 0;
  for (l = 0; l < k; l++) {
    printf("\nmeasurement: x: %f, y: %f dist: %f num_histogram: %d\n", measurements[l].x, measurements[l].y, measurements[l].dist, measurements[l].hist_num);
    printf("\n\nnum_histogram: %d\n\n", measurements[l].hist_num);
      fflush(stdout);
      mean_pos.x += measurements[l].x / k;
      mean_pos.y += measurements[l].y / k;
      mean_pos.dist += measurements[l].dist / k;
    }

  return mean_pos;
}


/**
 * Predict the x, y position of the UAV using the texton histogram.
 *
 * @param texton_hist The texton histogram
 *
 * @return The x, y, position of the MAV, computed by means of the input histogram
 */
struct measurement linear_regression_prediction(int texton_hist[]) {

  double total_x = 0, total_y = 0;
  
  int w;
  for (w = 0; w < NUM_TEXTONS * CHANNELS; w++) {
    total_x += ws_x[w] * texton_hist[w];
    total_y += ws_y[w] * texton_hist[w];
  }
  
  struct measurement pos;
  pos.x = total_x;
  pos.y = total_y;
  pos.dist = 0;

  return pos;
}

struct measurement predict_fann(int hist[], int size_hist) {

  fann_type *calc_out;
  fann_type input[size_hist];

  struct fann *ann;
  ann = fann_create_from_file("treXton_float.net");

  int i;
  /* Copy histogram to fann style histogram; possible TODO: do it directly when extracting the histogram */
  for (i = 0; i < size_hist; i++) {
       input[i] = hist[i];
    }

    calc_out = fann_run(ann, input);

    struct  measurement pos;
    pos.x = calc_out[0] * 1500;
    pos.y = calc_out[1] * 1500;
    pos.dist = 0;

return pos;

}


int main()
{

  trexton_init();

  int i ;
  for (i = 0; i < NUM_TEST_HISTOGRAMS; i++) {
    trexton_periodic();
  }

  return 0;
}
