#include "lib/vision/image.h"
#include "readpng.h"
#include "texton_settings.h"
#include "texton_helpers.h"
#include "image_conversions.h"

/* The output file for labeling*/
static char histogram_output_filename[] = "mat_train_hists.csv";
static char texton_out_filename[] = "mat_train_hists_texton.csv";
/*static char histogram_output_filename[] = "chrome2_hists.csv";*/

/* The folder that contains the images */
static char image_folder[] =  "/home/pold/Documents/Internship/datasets/board_train/";
/* static char image_folder[] =  "/home/pold/paparazzi/comp/"; */


void label_folder(char image_folder[], int num_imgs) {
  
  FILE *fp = fopen(histogram_output_filename, "a");
  FILE *fp_texton = fopen(texton_out_filename, "a");
  
  int i;
  for (i = 0; i < num_imgs; i++) {
    printf("Image num: %d\n", i);
    fflush(stdout);
    char image_path[2048];
    sprintf(image_path, "%s%d.png", image_folder, i);

    struct image_t rgb_img, opp_img, std_img, yuv_img;
    
    double means[8];
    image_create(&rgb_img, 320, 240, IMAGE_RGB);
    image_create(&opp_img, 320, 240, IMAGE_OPPONENT);
    image_create(&std_img, 320, 240, IMAGE_STD);
    image_create(&yuv_img, 320, 240, IMAGE_YUV422);
    
    printf("Type of creation is %d\n", opp_img.type);
    printf("Image path: %s", image_path);
    read_png_file(image_path, &rgb_img);
    RGBtoOpponent(&rgb_img, &opp_img, means);
    /* Convert RGB to YUV */
    RGBtoYUV422(&rgb_img, &yuv_img);
    
    double color_hist[COLOR_CHANNELS * NUM_COLOR_BINS] = {0.0};
    get_color_histogram(&yuv_img, color_hist, NUM_COLOR_BINS);

    int u;
    for (u = 0; u < 3 * NUM_COLOR_BINS; u++) {
      printf("c: %f ", color_hist[u]);
    }
    printf("\n");

    image_grayscale_standardize(&opp_img, &std_img, means);
    printf("Type of after conversion is %d\n", opp_img.type);
    
    int texton_histogram[NUM_TEXTONS * CHANNELS] = {0};
    get_texton_histogram(&yuv_img, texton_histogram, textons);

    /* int m, n; */
    /* for (m = 0; m < NUM_TEXTONS; m++) { */
    /*   for (n = 0; n < TOTAL_PATCH_SIZE; n++) { */
    /* 	printf("%f ", textons[m][n]);	 */
    /*   } */
    /* } */

    /* Append this histogram to the output file */
    /* save_histogram(texton_histogram, fp, NUM_TEXTONS * CHANNELS); */
    save_histogram_double(color_hist, fp, NUM_COLOR_BINS * COLOR_CHANNELS);
    save_histogram_int(texton_histogram, fp_texton, CHANNELS * NUM_TEXTONS);
    

   #if TEXTON_STANDARIZE
    
    image_free(&opp_img);
    image_free(&rgb_img);
    image_free(&std_img);
   #endif
    
  }

  fclose(fp);
  fclose(fp_texton);
  
}

int main(int argc, char *argv[])
{
  
  remove(histogram_output_filename);
  remove(texton_out_filename);
  
  /* Get textons -- that is the clustering centers */
  read_textons_from_csv(textons, texton_filename);


  int i, j;
  for (i = 0; i < NUM_TEXTONS * CHANNELS; i++) {
    for (j = 0; j < TOTAL_PATCH_SIZE; j++) {
      printf("%f ", textons[i][j]);
    }
    printf("\n");
  }


  label_folder(image_folder, 1900);
  /* label_folder(image_folder, 44); */
  
  return 0;
}
