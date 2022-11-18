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

import matplotlib.pyplot as plt
from tensorflow.keras.callbacks import ModelCheckpoint, EarlyStopping
import numpy as np
import seaborn as sns
import os


def create_callback(model, path_chpt, patience=30, metric="accuracy", verbose=10, epochs=None):
    """

    :param model: model being trained
    :param path_chpt: path to save checkpoint
    :param patience: what patience to use
    :param metric: what scoring metric to use
    :param verbose: verbose number
    :param epochs: number of epochs
    :return: List of Callback
    """

    callbacks = [EarlyStopping(monitor="val_loss", min_delta=0, patience=patience, verbose=1, mode="min"),
                 ModelCheckpoint(filepath=path_chpt, save_best_only=True),]

    return callbacks


def plot_learning_history(fit, metric="accuracy", path="history.png"):
    """

    :param fit: history object
    :param metric: metric to calculate
    :param path: path to log file
    :return: plot of learning history
    """

    fig, (axL, axR) = plt.subplots(ncols=2, figsize=(10, 4))
    axL.plot(fit.history["loss"], label="feature_data_train")
    axL.plot(fit.history["val_loss"], label="validation")
    axL.set_title("Loss")
    axL.set_xlabel("epoch")
    axL.set_ylabel("loss")
    axL.legend(loc="upper right")

    axR.plot(fit.history[metric], label="feature_data_train")
    axR.plot(fit.history[f"val_{metric}"], label="validation")
    axR.set_title(metric.capitalize())
    axR.set_xlabel("epoch")
    axR.set_ylabel(metric)
    axR.legend(loc="lower right")

    fig.savefig(path)
    plt.close()


def plot_confusion_matrix(conf_matrix, labels=None, path="confusion_matrix.png"):
    """

    :param conf_matrix: confusion matrix to plot
    :param labels: labels data
    :param path: path to save confusion matrix
    :return: a confusion matrix saved to log folder
    """

    ax = plt.subplot()

    sns.heatmap(
            conf_matrix,
            annot=True,
            cmap="Blues",
            square=True,
            vmin=0,
            vmax=1.0,
            xticklabels=labels,
            yticklabels=labels,
            ax=ax
    )

    ax.set_xlabel("Predicted label")
    ax.set_ylabel("True label")
    ax.set_title(f"Normalized confusion matrix")

    plt.tight_layout()
    plt.savefig(path)
    plt.close()


def plot_summary(model, path):
    """

    @param model: Model to plot layers
    @param path: Path to plot model summary
    """
    if not os.path.isfile(path):
        with open(path, 'w') as f:
            model.summary(print_fn=lambda x: f.write(x + '\n'))