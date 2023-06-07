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
# Created a class around model training
# Changed model layering


"""
Driver file to train a machine learning model in Tensorflow and Keras for gesture classification
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from sklearn.metrics import precision_score, recall_score, f1_score

from utils import plot_learning_history, create_callback, plot_confusion_matrix, plot_summary
from tensorflow.keras.models import Model
from logging_config_train import *

import argparse
import datetime
from data_load import DataLoader
import numpy as np
import tensorflow as tf
import tensorflow_model_optimization as tfmot

class TrainModel(object):

    def __init__(self):
        self.logdir = "logs/" + EXEC_TIME
        self.model_name = "Magic_wand_model"
        self.seq_length = 128
        self.epochs = 10
        self.batch_size = 64

    def reshape_function(self, data, label):
        """

        @param data: The data that we reshape to 2D
        @param label: The labels related to that data
        @return: reshaped data and corresponding label
        """
        reshaped_data = tf.reshape(data, [-1, 6, 1])
        return reshaped_data, label

    def calculate_model_size(self, model):
        """

        @param model: The model, whos size will be calculated
        """
        print(model.summary())
        var_sizes = [
            np.product(list(map(int, v.shape))) * v.dtype.size
            for v in model.trainable_variables
        ]
        print("[INFO] Model size:", sum(var_sizes) / 1024, "KB")

    def build_square_model(self, seq_length):
        """
        Builds a convolutional neural network in Keras for the square data
        @param seq_length: the length of the sequence, in the default case it is 128
        @return: a keras ML model
        """
        model = tf.keras.Sequential([
            tf.keras.layers.Conv2D(16, (3, 3), padding="same", activation="relu", kernel_initializer='glorot_normal',
                                   input_shape=(seq_length, 6, 1)),
            tf.keras.layers.MaxPool2D((3, 3)),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Conv2D(32, (3, 3), padding="same", kernel_initializer='glorot_normal', activation="relu"),
            tf.keras.layers.MaxPool2D((3, 1)),
            tf.keras.layers.Dropout(0.3),
            tf.keras.layers.GlobalAveragePooling2D(),
            tf.keras.layers.Dense(16, kernel_initializer='glorot_normal', activation="relu"),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(4, activation="softmax")
        ])

        return model

    def load_data(self, train_data_path, valid_data_path, test_data_path):
        data_loader = DataLoader(
            train_data_path, valid_data_path, test_data_path, seq_length=self.seq_length)
        data_loader.format()
        return data_loader.train_len, data_loader.train_data, data_loader.valid_len, \
               data_loader.valid_data, data_loader.test_len, data_loader.test_data

    def build_net(self):
        """

        @param args: AN argument that determines what type of dataset is used
        @return: a model built specifically for the dataset assigned from the argument
        """
        print("[INFO] Building model")
        model = self.build_square_model(self.seq_length)
        print(model.summary())

        return model

    def structure_data(self, train_data, valid_data, test_data, test_len, train_len):
        """
        @param train_data: A TF dataset of training data
        @param valid_data: A TF dataset of validation data
        @param test_data: A TF dataset of testing data
        @param test_len: Length of the test dataset
        @param train_len: Length of the training dataset
        @return: Structured datasets for training, validation and test, with corresponding labels
        """
        train_data = train_data.map(self.reshape_function)
        test_data = test_data.map(self.reshape_function)
        valid_data = valid_data.map(self.reshape_function)

        test_labels = np.zeros(test_len)
        idx = 0
        for data, label in test_data:
            test_labels[idx] = label.numpy()
            idx += 1

        train_labels = np.zeros(train_len)
        idx = 0
        for data, label in train_data:
            train_labels[idx] = label.numpy()
            idx += 1

        train_data = train_data.batch(self.batch_size).repeat()
        valid_data = valid_data.batch(self.batch_size)
        test_data = test_data.batch(self.batch_size)

        return train_data, valid_data, test_data, test_labels, train_labels

    def model_performance(self, model, test_data, test_labels):
        """
        @param model: Keras model to test performance
        @param test_data: Data to test the model on
        @param test_labels: Labels to test the model on
        @return: Logs performance metrics while returning a confusion matrix for visulisation
        """
        loss, acc = model.evaluate(test_data)
        logger.debug(f'Model Test loss: {loss} / Model Test accuracy: {acc}')

        pred = np.argmax(model.predict(test_data), axis=1)
        precision = precision_score(test_labels, pred, average="macro")
        recall = recall_score(test_labels, pred, average="macro")
        f1 = f1_score(test_labels, pred, average="macro")

        logger.debug(f'Model precision: {precision}')
        logger.debug(f'Model Recall: {recall}')
        logger.debug(f'Model F1 Score: {f1}')

        confusion = tf.math.confusion_matrix(
            labels=tf.constant(test_labels),
            predictions=tf.constant(pred),
            num_classes=4)

        return confusion

    def train_net(self, model, train_len, train_data, valid_len, valid_data, test_len, test_data):
        """
        @param model: the prebuilt keras model to train
        @param train_len: Length of the training data
        @param train_data: A TF dataset for training
        @param valid_len: Length of the validation data
        @param valid_data: A TF dataset for validation
        @param test_len: Length of the test data
        @param test_data: A TF dataset for testing
        """
        print("[INFO] Start training")
        self.calculate_model_size(model)
        plot_summary(model, path=f"{self.logdir}/model.txt")
        optimizer = tf.keras.optimizers.Adam(lr=0.0001)
        model.compile(optimizer=optimizer, loss="sparse_categorical_crossentropy", metrics=["accuracy"])

        train_data, valid_data, test_data, test_labels, train_labels = self.structure_data(train_data, valid_data,
                                                                                           test_data, test_len,
                                                                                           train_len)

        callbacks = create_callback(model=model, path_chpt=f"{self.logdir}/{self.model_name}.h5", verbose=10,
                                    epochs=self.epochs)

        fit = model.fit(train_data,
                        epochs=self.epochs,
                        validation_data=valid_data,
                        steps_per_epoch=1000,
                        validation_steps=int((valid_len - 1) / self.batch_size + 1),
                        callbacks=callbacks)

        plot_learning_history(fit=fit, path=f"{self.logdir}/{self.model_name}.png")
        confusion = self.model_performance(model, test_data, test_labels)

        path = 'gesture_data/'
        classes = os.listdir(path)
        for x in range(4):
            if classes[x] == 'negative':
                del classes[x]
                classes.append("negative")
                break

        plot_confusion_matrix(confusion, labels=[classes[0], classes[1], classes[2], classes[3]],
                              path=f"{self.logdir}/confusion_matrix_square.png")
                              
        self.prune_model(model, train_data, valid_data, test_data, args.sparsity_level)

    def prune_model(self, model, train_data, valid_data, test_data, sparsity_level=50):
        print("[INFO] Model pruning started")
        sparsity_level = int(sparsity_level)/100
        epochs_pruning = 3
        pruned_model_name = "gesture_model_f32_pruned"
        self.print_model_weights_sparsity(model)
        # define pruning schedule
        prune_low_magnitude = tfmot.sparsity.keras.prune_low_magnitude
        
        # this wraps the the mode/layer with pruning functionality to ensure sparisty introduction during model retraining.
        pruning_params = {
            "pruning_schedule" : tfmot.sparsity.keras.ConstantSparsity(sparsity_level, begin_step=0, frequency=100)
        }

        model_for_pruning = prune_low_magnitude(model, **pruning_params)

        callbacks = [
            tfmot.sparsity.keras.UpdatePruningStep()
        ]

        optimizer = tf.keras.optimizers.Adam(1e-5)
        
        model_for_pruning.compile(
            optimizer=optimizer,
            loss="sparse_categorical_crossentropy",
            metrics=["accuracy"]
        )

        model_for_pruning.summary()

        model_for_pruning.fit(
            train_data,
            epochs = epochs_pruning,
            steps_per_epoch=1000,
            callbacks=callbacks,
            verbose=1
        )
        print("[INFO] Model pruning done")

        # after training we need to strip the pruning wrapper to avoid duplicated weights, leading to increase in model size.
        stripped_pruned_model = tfmot.sparsity.keras.strip_pruning(model_for_pruning)
        stripped_pruned_model.compile(
            optimizer=optimizer,
            loss="sparse_categorical_crossentropy",
            metrics=["accuracy"]
        )
        # sparisty level in model after pruning
        self.print_model_weights_sparsity(stripped_pruned_model)
        loss , acc = stripped_pruned_model.evaluate(test_data)
        print(f"Accuracy: {acc}\nLoss: {loss}")
        stripped_pruned_model.save(f"{self.logdir}/Magic_wand_model_pruned.h5")

    def print_model_weights_sparsity(self, model):
        for layer in model.layers:
            if isinstance(layer, tf.keras.layers.Wrapper):
                weights = layer.trainable_weights
            else:
                weights = layer.weights
            for weight in weights:
                if "kernel" not in weight.name or "centroid" in weight.name:
                    continue
                weight_size = weight.numpy().size
                zero_num = np.count_nonzero(weight == 0)
                print(
                    f"[INFO] {weight.name}: {zero_num/weight_size:.2%} sparsity ",
                    f"({zero_num}/{weight_size})",
                )


if __name__ == "__main__":
    """
    Driver function to train a machine learning model to classify square data
    @param args: Arguments taken from the user to determine how training works
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("-prune", "--sparsity_level", help = "Set the level of sparsity for pruning the model. Eg. 70 denotes '70%' sparse model.")
    args = parser.parse_args()

    
    train_obj = TrainModel()

    train_len, train_data, valid_len, valid_data, test_len, test_data = \
        train_obj.load_data("./data/train", "./data/valid", "./data/test")

    print("[INFO] Data load successful")

    model = train_obj.build_net()
    train_obj.train_net(model, train_len, train_data, valid_len, valid_data, test_len, test_data)

    print("[INFO] Training complete")
