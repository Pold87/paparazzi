#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <particle_filter.h>

#include <Ivy/ivy.h>
#include <Ivy/ivyglibloop.h>


static void on_position_estimate(IvyClientPtr app, void *user_data, int argc, char *argv[]){
    /* printf("id:%s x:%s y: %s:\n", argv[0], argv[1], argv[2]); */
  visualize_simple(atof(argv[1]), atof(argv[2]));
}


int main ( int argc, char** argv) {
  printf("Starting VPS");
  init_visualize();
  GMainLoop *ml =  g_main_loop_new(NULL, FALSE);

  IvyInit ("VPS visualizer", "VPS Visualizer READY", NULL, NULL, NULL, NULL);
  IvyBindMsg(on_position_estimate, NULL, "^(\\S*) GPS_INT (\\S*) (\\S*) (\\S*) .*");

  IvyStart("127.255.255.255");

  g_main_loop_run(ml);

  return 0;
}
