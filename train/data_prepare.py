# Copyright 2019 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

# Portions (c) 2022, Cypress Semiconductor Corporation (an Infineon company) or 
# an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
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

# CYPRESS CHANGES TO TENSORFLOW CONTENT:
# Created a class around data prepare
# Added overlap and data segmentation
# Add arguments for script options


"""Prepare data for training of the ML model.

For the gesture model:
    Read data from from the generated folders and save them
    in "/data/complete_data" in python dict format.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import csv
import json
import os
import pandas as pd
from scipy.signal import butter, lfilter
import numpy as np
import argparse
import pickle

from file_encodings import NumpyArrayEncoder
from data_split import read_data, split_data
from tsaug import TimeWarp, Crop, Quantize, Drift, Reverse

class DataPreparation(NumpyArrayEncoder):
    def __init__(self):
        super().__init__()
        self.label_name = 'gesture'
        self.data_name = 'accel_xyz'

    def prepare_original_data_overlap(self, classes, name, data, file_to_read, overlap):
        """
        Read collected data from files.

        @param classes: The classes for the current data set
        @param name: The name of the data
        @param data: The complete dataset
        @param file_to_read: The specific file that we want to read from a directory
        @param overlap: The overlap rate of data 0 - 0.99
        """

        datax = []
        with open(file_to_read, "r") as f:
            print('[INFO] File being read: ', file_to_read)
            lines = csv.reader(f)
            data_new = {}

            data_new[self.label_name] = classes

            data_new[self.data_name] = []
            data_new["name"] = name
            for idx, line in enumerate(lines):
                if len(line) == 6:
                    if line[2] == "-" and data_new[self.data_name]:
                        datax.append(data_new)
                        data_new = {}

                        data_new[self.label_name] = classes

                        data_new[self.data_name] = []
                        data_new["name"] = name
                    elif line[2] != "-":
                        data_new[self.data_name].append([float(i) for i in line[0:6]])

            datax.append(data_new)
            data_updating = datax

            full_arr = self.overlap_features(data_updating, overlap)

            for idx1, line in enumerate(full_arr):

                data_new_1 = {}
                data_new_1[self.label_name] = classes

                data_new_1[self.data_name] = []
                data_new_1["name"] = name
                for x in line:
                    data_new_1[self.data_name].append(x)

                data.append(data_new_1)

    def segment_signal(self, signal, overlap_rate, window_size=128, res_type="array"):
        """
        Sample sensor signals in fixed-width sliding windows and 50% overlap (128 readings/window)

        :param signal: raw signal
        :param window_size: window size of sliding window to segment raw signals.
        :param overlap_rate: overlap rate of sliding window to segment raw signals.
        :param res_type: type of return value; 'array' or 'dataframe'
        :return: list of segmented signal
        """

        signal_seg = []

        for start_idx in range(0, len(signal) - window_size, int(window_size * overlap_rate)):
            seg = signal.iloc[start_idx: start_idx + window_size].reset_index(drop=True)
            if res_type == "array":
                seg = seg.values
            signal_seg.append(seg)

        if res_type == "array":
            signal_seg = np.array(signal_seg)

        return signal_seg

    def overlap_features(self, data_updating, overlap):
        """
        Overlap features at a rate of 0 to 0.9

        @param data_updating: Data to stretch based on overlap rate
        @param overlap: The rate as a decimal point that we want to overlap 0 to 0.99
        @return: A list of segmented overlapped signals
        """

        full_arr = np.array([])

        for idx1, line in enumerate(data_updating):

            x = line[self.data_name]
            a = np.array(x)
            if len(full_arr) == 0:
                full_arr = a
            else:
                full_arr = np.vstack((full_arr, a))

        df = pd.DataFrame(full_arr, columns=['accx', 'accy', 'accz', 'gyrox', 'gyroy', 'gyroz'])

        segmented_signal = self.segment_signal(df, overlap)

        return segmented_signal

    def write_data(self, data_to_write, path):
        """
        Writing data to file

        @param data_to_write: Data that we want to write to file in json format
        @param path: Path to where to write file

        """
        with open(path, "w") as f:
            for idx, item in enumerate(data_to_write):
                dic = json.dumps(item, cls=NumpyArrayEncoder,  ensure_ascii=False)
                f.write(dic)
                f.write("\n")

    def write_complete_data(self, final_data):
        """
        Write complete data to file

        @param final_data: Complete dataset as a file in json format
        """

        # Write all gesture data to a file named data
        if not os.path.exists("./data"):
            os.makedirs("./data")
        self.write_data(final_data, "./data/complete_data")

    def save_pickle(self, final_data, data_name):
        """
        Save data as a pickle file

        @param final_data: Complete dataset as a file in json format
        @param data_name: The name of the dataset
        """

        f = open(f'./data/{data_name}.pkl', 'wb')
        pickle.dump(final_data, f)
        f.close()

    def split_data(self, classes):
        """
        Split data randomly
        @param classes: the different classes/gestures that will be trained
        """
        print('[INFO] Splitting data')
        data = read_data("./data/complete_data")
        train_data, valid_data, test_data = split_data(data, 0.6, 0.2, classes)
        self.write_data(train_data, "./data/train")
        self.write_data(valid_data, "./data/valid")
        self.write_data(test_data, "./data/test")
        print('[INFO] Splitting data complete')

    def get_dataset(self, args):
        """
        Retrieve square dataset

        @param args: Arguments to determine if the dataset is saved as a pickle file, and how to split data
        """
        folders, names, classes = self.get_files()

        # Read all data in and perform data overlap
        final_data = []
        for idx1, folder in enumerate(folders):
            for idx2, name in enumerate(names[idx1]):
                activity = folder.split('/')
                self.prepare_original_data_overlap(classes[idx1], name, final_data,
                                                   "%s/%s" % (folder, name), args.overlap)

        print('[INFO] All files read successfully')
        print("[INFO] Length of data set is: " + str(len(final_data)))
        print("[INFO] Writing dataset to data directory")
        self.write_complete_data(final_data)
        print("[INFO] Writing data complete")

        # Split data into train, validation, and test
        self.split_data(classes)


    def get_files(self):
        """
        Pulls in all training data from gesture_data folder
        limited to 4 gestures
        @return: folders - folder paths to all the training txt files
                 names - names of all the training txt files
                 classes - names of the classes that will train the model with
        """
        path = 'gesture_data/'
        folders = []
        names = []
        classes = os.listdir(path)

        # Gets all the classes and move the negative class to the back of the list
        for x in range(4):
            if classes[x] == 'negative':
                del classes[x]
                classes.append("negative")
                break

        # Gets all the folder paths
        for x in range(4):
            folders.append(path + classes[x])

        # Gets all the txt training file names
        for x in range(4):
            path = folders[x]
            names.insert(x, os.listdir(path))

        return folders, names, classes


def main(arguments):
    """
    Driver function to prepare the square dataset
    @param arguments: Arguments to determine the dataset to use
    """
    prep = DataPreparation()
    prep.get_dataset(args)

    print('[INFO] Dataset prepared and ready for training')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--overlap", "-o", default=0.2, type=float, help='Overlap of data rate as a decimal')
    args = parser.parse_args()

    main(args)
