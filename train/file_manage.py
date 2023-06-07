# (c) 2022, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor 
# Corporation.  All rights reserved.
#
# This software, including source code, documentation and related materials
# ("Software") is owned by Cypress Semiconductor Corporation or one of its
# affiliates ("Cypress") and is protected by and subject to worldwide patent 
# protection (United States and foreign), United States copyright laws and 
# international treaty provisions.  Therefore, you may use this Software only
# as provided in the license agreement accompanying the software package from
# which you obtained this Software ("EULA").
#
# If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
# non-transferable license to copy, modify, and compile the Software source
# code solely for use in connection with Cypress's integrated circuit products.
# Any reproduction, modification, translation, compilation, or representation
# of this Software except as specified above is prohibited without the express 
# written permission of Cypress.
#
# Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
# reserves the right to make changes to the Software without notice. Cypress
# does not assume any liability arising out of the application or use of the
# Software or any product or circuit described in the Software. Cypress does
# not authorize its products for use in any products where a malfunction or
# failure of the Cypress product may reasonably be expected to result in
# significant property damage, injury or death ("High Risk Product"). By
# including Cypress's product in a High Risk Product, the manufacturer of such 
# system or application assumes all risk of such use and in doing so agrees to
# indemnify Cypress against all liability.

import os
import shutil
import tensorflow as tf

all_folders = []
path = 'logs\\'
folders = os.listdir(path)

# Get all folders in the log file
for x in range(len(folders)):
    all_folders.append(path + folders[x])

# Get the path to the current parent directory
save = os.path.abspath(os.path.join(os.getcwd(), os.pardir))

# Create a path to the most recent generated model
latest = save + '\\train\\' + max(all_folders, key=os.path.getctime) + '\\Magic_wand_model.h5'
latest_pruned = save + '\\train\\' + max(all_folders, key=os.path.getctime) + '\\Magic_wand_model_pruned.h5'

# Create a path to the save location for tflite and h5 formats
save_tflite = save + '\\pretrained_models\\' + 'User_generated_model.tflite'
save_keras = save + '\\pretrained_models\\' + 'User_generated_model.h5'
save_pruned = save + '\\pretrained_models\\' + 'User_generated_model_pruned.h5'

# Copy the model from logs to pretrained_models folder
shutil.copyfile(latest, save_keras)
shutil.copyfile(latest_pruned, save_pruned)

# Generate a tflite model based on keras model
model = tf.keras.models.load_model(save_keras)
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
open(save_tflite, "wb").write(tflite_model)