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
# Remove pad function
# Add format_support_func_vis and format_vis functions
# Basic changes to script for support of gesture data

"""Load data from the specified paths and format them for training."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import numpy as np
import tensorflow as tf
import os


class DataLoader(object):
    """Loads data and prepares for training."""

    def __init__(self, train_data_path, valid_data_path, test_data_path,
                 seq_length):
        self.dim = 6
        self.classes = 4
        self.label_name = 'gesture'
        self.data_name = 'accel_xyz'
        self.training_labels = []
        self.valid_labels = []
        self.testing_labels = []
        self.seq_length = seq_length
        self.train_data, self.train_label, self.train_len = self.get_data_file(
            train_data_path, "train")
        self.valid_data, self.valid_label, self.valid_len = self.get_data_file(
            valid_data_path, "valid")
        self.test_data, self.test_label, self.test_len = self.get_data_file(
            test_data_path, "test")

    def get_data_file(self, data_path, data_type):
        """
        Get train, valid and test data from files.

        @param data_path: The path of the data to read
        @param data_type: The type of data, train, test or validation
        @return:
        """

        data = []
        label = []
        with open(data_path, "r") as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                dic = json.loads(line)
                data.append(dic[self.data_name])
                label.append(dic[self.label_name])

        length = len(label)
        print('[INFO]', data_type + ' data length: ' + str(length))
        return data, label, length

    def format_support_func(self, length, data, label):
        """
        @param length: The length of the passed data
        @param data: The data to turn into a tf dataset
        @param label: The corresponding data labels
        @return: the length of the dataset, and the TF dataset of the given data
        """

        xx = np.zeros((1, self.seq_length, self.dim))
        features = np.empty([0, self.seq_length, self.dim])
        labels = np.empty([0, self.classes])

        path = 'gesture_data/'
        classes = os.listdir(path)
        for x in range(4):
            if classes[x] == 'negative':
                del classes[x]
                classes.append("negative")
                break

        label2id_square = {classes[0]: 0, classes[1]: 1, classes[2]: 2, classes[3]: 3}

        gesture_labels = label2id_square

        nb_seq = 0
        for idx, (tmp_data, label_id) in enumerate(zip(data, label)):
            data_np = np.array(tmp_data)
            sz_x = data_np.shape
            data_len = sz_x[0]

            if self.dim != sz_x[1]:
                raise Exception("Number of features do not match")

            for i in range(0, data_len - self.seq_length + 1):
                xx[0, :, :] = data_np[i:i + self.seq_length, :]
                features = np.append(features, xx, axis=0)
                labels = np.append(labels, gesture_labels[label_id])
                nb_seq += 1

        length = nb_seq

        # Turn into tf.data.Dataset
        dataset = tf.data.Dataset.from_tensor_slices((features, labels.astype("int32")))
        return length, dataset

    def format_support_func_vis(self, data, label):
        """
        Support function for format for visualisations
        @param data: The data to turn into a tf dataset
        @param label: The corresponding data labels
        @return: the length of the dataset, and the TF dataset of the given data
        """
        xx = np.zeros((1, self.seq_length, self.dim))
        features = np.empty([0, self.seq_length, self.dim])
        labels = np.empty([0, self.classes])

        path = 'gesture_data/'
        classes = os.listdir(path)

        label2id_square = {classes[0]: 0, classes[1]: 1, classes[2]: 2, classes[3]: 3}

        gesture_labels = label2id_square

        nb_seq = 0
        for idx, (tmp_data, label_id) in enumerate(zip(data, label)):
            data_np = np.array(tmp_data)
            sz_x = data_np.shape
            data_len = sz_x[0]

            if self.dim != sz_x[1]:
                raise Exception("Number of features do not match")

            # This loop is for just size 128
            for i in range(0, data_len - self.seq_length + 1):
                xx[0, :, :] = data_np[i:i + self.seq_length, :]
                features = np.append(features, xx, axis=0)
                labels = np.append(labels, gesture_labels[label_id])
                nb_seq += 1

        return features, labels

    def format(self):
        """
        Format data and get the dataset for the model training
        """
        self.train_len, self.train_data = self.format_support_func(self.train_len, self.train_data, self.train_label)
        self.valid_len, self.valid_data = self.format_support_func(self.valid_len, self.valid_data, self.valid_label)
        self.test_len, self.test_data = self.format_support_func(self.test_len, self.test_data, self.test_label)

    def format_vis(self):
        """
        Format data for visualisations
        """
        self.train_data, self.training_labels = self.format_support_func_vis(self.train_data, self.train_label)
        self.valid_data, self.valid_labels = self.format_support_func_vis(self.valid_data, self.valid_label)
        self.test_data, self.testing_labels = self.format_support_func_vis(self.test_data, self.test_label)
