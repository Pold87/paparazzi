#include "fann.h"

int main()
{
    const unsigned int num_input = 33;
    const unsigned int num_output = 2;
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 50;
    const float desired_error = (const float) 0.0005;
    const unsigned int max_epochs = 10000;
    const unsigned int epochs_between_reports = 100;
    struct fann_train_data * data = NULL;

    data = fann_read_train_from_file("dataset.data");

    struct fann *ann = fann_create_standard(num_layers,
                                            data->num_input,
                                            num_neurons_hidden,
                                            data->num_output);

    fann_set_learning_rate(ann, 0.00001f);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID);
    fann_set_activation_function_output(ann, FANN_LINEAR);
    fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);

  /* fann_set_scaling_params( */
  /*    ann, */
  /*    data, */
  /*     0,	/\* New input minimum *\/ */
  /*     100,	/\* New input maximum *\/ */
  /*     0,	/\* New output minimum *\/ */
  /*     1);	/\* New output maximum *\/ */

  /* fann_scale_train( ann, data ); */

  fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);
  fann_destroy_train( data );

    fann_save(ann, "treXton_float.net");

    fann_destroy(ann);

    return 0;
}
