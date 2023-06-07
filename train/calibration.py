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
import numpy as np
from numpy import genfromtxt


if __name__ == "__main__":
    path = 'gesture_data/'

    # Create lists to store the folder names and the path to the calibration data
    folders = []
    calibration_data_path = []

    # Store the classes
    classes = os.listdir(path)

    # Move the negative or no gesture data to the end
    for x in range(4):
        if classes[x] == 'negative':
            del classes[x]
            classes.append("negative")
            break

    # Gets all the folder paths
    for x in range(4):
        folders.append(path + classes[x])

    # Grabs the first txt file in each folder
    for x in range(4):
        path = folders[x]
        file_list = os.listdir(path)
        for file in range(len(file_list)):
            calibration_data_path.append(folders[x] + '/' + os.listdir(path)[file])

    # Store data from the selected folders in a numpy array
    total_data = []
    for x in range(len(calibration_data_path)):
        data = genfromtxt(calibration_data_path[x], delimiter=',')

        # Removes all NAN variables in the numpy
        data = data[~np.isnan(data)]

        # Remove the last sample if it is not full
        if (data.size % (128*6)) != 0:
            overflow = data.size - (data.size % (128*6))
            data = np.delete(data, np.s_[overflow:])

        # Store all data in one array
        total_data = np.append(total_data, data)

    # Add random samples to ensure all nodes in the model are hit
    rand_samples = 20*128*6
    rand_data = np.random.rand(rand_samples)
    for x in range(rand_samples):
        if x % 2 != 0:
            rand_data[x] = rand_data[x] * -1

    total_data = np.append(total_data, rand_data)

    # Reshape the numpy array to match the input of the model
    total_data = total_data.reshape(-1, 128, 6, 1)

    # Create an empty 2D array that will be used by the ML tooling
    classification = np.zeros((total_data.shape[0], 1), dtype=int)

    # Save the arrays as a npz file
    np.savez('calibration.npz', total_data, classification)
