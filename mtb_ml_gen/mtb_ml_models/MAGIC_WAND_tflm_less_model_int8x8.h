// This file is generated. Do not edit.
// Generated on: 27.09.2022 21:35:10

#ifndef MAGIC_WAND_GEN_H
#define MAGIC_WAND_GEN_H

#include "tensorflow/lite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define MAGIC_WAND_MODEL_CONST_DATA_SIZE 8326
#define MAGIC_WAND_MODEL_INIT_DATA_SIZE 1053
#define MAGIC_WAND_MODEL_UNINIT_DATA_SIZE 15660


// Sets up the model with init and prepare steps.
TfLiteStatus MAGIC_WAND_init();
// Returns the input tensor with the given index.
TfLiteTensor *MAGIC_WAND_input(int index);
// Returns the output tensor with the given index.
TfLiteTensor *MAGIC_WAND_output(int index);
// Runs inference for the model.
TfLiteStatus MAGIC_WAND_invoke();

// Returns the number of input tensors.
size_t MAGIC_WAND_inputs();

// Returns the number of output tensors.
size_t MAGIC_WAND_outputs();

// Return the buffer pointer of input tensor
void *MAGIC_WAND_input_ptr(int index);

// Return the buffer size of input tensor
size_t MAGIC_WAND_input_size(int index);

// Return the dimention size of input tensor
int MAGIC_WAND_input_dims_len(int index);

// Return the dimention buffer pointer of input tensor
int *MAGIC_WAND_input_dims(int index);

// Return the buffer pointer of output tensor
void *MAGIC_WAND_output_ptr(int index);

// Return the buffer size of output tensor
size_t MAGIC_WAND_output_size(int index);

// Return the dimention size of output tensor
int MAGIC_WAND_output_dims_len(int index);

// Return the dimention buffer pointer of output tensor
int *MAGIC_WAND_output_dims(int index);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif
