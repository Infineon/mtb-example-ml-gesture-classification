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
# Minor cleanup for usecase/reduce code
# Remove startup code if main


"""
Mix and split data.

Mix different people's data together and randomly split them into train,
validation and test. These data would be saved separately under "/data".
It will generate new files with the following structure:
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import random

# Read data
def read_data(path):
    data = []
    with open(path, "r") as f:
        lines = f.readlines()
        for idx, line in enumerate(lines):
            dic = json.loads(line)
            data.append(dic)

    return data


def split_data(data, train_ratio, valid_ratio, gestures):
    """
    Splits data into train, validation and test according to ratio.

    @param data: The complete data that we want to split
    @param train_ratio: The ratio of training data we want
    @param valid_ratio: The ratio of validation data
    @param gestures: The gestures that the model will be trained for
    @return: The training, validation and test dat that will be used to train the model
    """
    train_data = []
    valid_data = []
    test_data = []

    num_dic = {gestures[0]: 0, gestures[1]: 0, gestures[2]: 0, gestures[3]: 0}

    for idx, item in enumerate(data):
        for i in num_dic:
            if item["gesture"] == i:
                num_dic[i] += 1
    print('[INFO] Data Class Breakdown ', num_dic)
    train_num_dic = {}
    valid_num_dic = {}
    for i in num_dic:
        train_num_dic[i] = int(train_ratio * num_dic[i])
        valid_num_dic[i] = int(valid_ratio * num_dic[i])
    random.seed(30)
    random.shuffle(data)
    for idx, item in enumerate(data):
        for i in num_dic:
            if item["gesture"] == i:
                if train_num_dic[i] > 0:
                    train_data.append(item)
                    train_num_dic[i] -= 1
                elif valid_num_dic[i] > 0:
                    valid_data.append(item)
                    valid_num_dic[i] -= 1
                else:
                    test_data.append(item)
    print("[INFO] Training data length: " + str(len(train_data)))
    print("[INFO] Validation data length: " + str(len(valid_data)))
    print("[INFO] Testing data length: " + str(len(test_data)))
    return train_data, valid_data, test_data

