################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
#
################################################################################
# \copyright
# Copyright 2018-2024, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
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
################################################################################


################################################################################
# Basic Configuration
################################################################################

# Type of ModusToolbox Makefile Options include:
#
# COMBINED    -- Top Level Makefile usually for single standalone application
# APPLICATION -- Top Level Makefile usually for multi project application
# PROJECT     -- Project Makefile under Application
#
MTB_TYPE=COMBINED

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make library-manager' from command line), which will also update Eclipse IDE launch
# configurations.
TARGET=CY8CKIT-062S2-43012

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=mtb-example-ml-gesture-classification

# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC provided with ModusToolbox software
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
#
# If CONFIG is manually edited, ensure to update or regenerate launch configurations
# for your IDE.
CONFIG=Debug

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

# Neural Network Configuration. Options include
#
# float    -- floating point for the input data and weights
# int8x8   -- 8-bit fixed-point for the input data and weights
# int16x8  -- 16-bit fixed-point for the input data and 8-bit for weights
# int16x16 -- 16-bit fixed-point for the input data and weights
NN_TYPE=float

# Model Name to be loaded to the firmware
NN_MODEL_NAME=MAGIC_WAND

# Folder name containing the model and regression data
NN_MODEL_FOLDER=./mtb_ml_gen

# Choose the inference engine
# tflm      -- TensorFlow Lite for Microcontrollers inference engine with interpreter
# tflm_less -- TensorFlow Lite for Microcontrollers inference engine interpreter-less
# ifx       -- Infineon ModusToolbox ML inference engine
NN_INFERENCE_ENGINE=tflm

# Shield used to gather IMU data
#
# CY_028_TFT_SHIELD    -- Using the 028-TFT shield
# CY_028_SENSE_SHIELD_v1  -- Using the 028-SENSE shield rev**
# CY_028_SENSE_SHIELD_v2  -- Using the 028-SENSE shield rev*B or later
SHIELD_DATA_COLLECTION=CY_028_TFT_SHIELD

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS=FREERTOS

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

NN_MODEL_FOLDER=mtb_ml_gen

# Select only the regression and model files that belong to the desired
# settings. 
MODEL_PREFIX=$(subst $\",,$(NN_MODEL_NAME))
CY_IGNORE+=$(NN_MODEL_FOLDER)
# Add the model file based on the inference and data types
SOURCES+=$(wildcard $(NN_MODEL_FOLDER)/mtb_ml_models/$(MODEL_PREFIX)_$(NN_INFERENCE_ENGINE)_model_$(NN_TYPE).c*)

# Ignore any other model and regression data files
CY_IGNORE+=$(LIST_IGNORE_MODELS)
CY_IGNORE+=$(LIST_IGNORE_REGDATA)

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=$(NN_MODEL_FOLDER)/mtb_ml_models source

# Add additional defines to the build process (without a leading -D).
DEFINES=MODEL_NAME=$(NN_MODEL_NAME)

# Add additional define to select the inference engine
ifeq (tflm, $(NN_INFERENCE_ENGINE))
COMPONENTS+=ML_TFLM_INTERPRETER IFX_CMSIS_NN
DEFINES+=TF_LITE_STATIC_MEMORY
endif

ifeq (tflm_less, $(NN_INFERENCE_ENGINE))
COMPONENTS+=ML_TFLM_INTERPRETER_LESS IFX_CMSIS_NN
DEFINES+=TF_LITE_STATIC_MEMORY TF_LITE_MICRO_USE_OFFLINE_OP_USER_DATA
endif

ifeq (ifx, $(NN_INFERENCE_ENGINE))
COMPONENTS+=ML_IFX IFX_CMSIS_NN
endif


# Depending which Neural Network Type, add a specific DEFINE and COMPONENT
ifeq (float, $(NN_TYPE))
COMPONENTS+=ML_FLOAT32
endif
ifeq (int16x16, $(NN_TYPE))
COMPONENTS+=ML_INT16x16
endif
ifeq (int16x8, $(NN_TYPE))
COMPONENTS+=ML_INT16x8
endif
ifeq (int8x8, $(NN_TYPE))
COMPONENTS+=ML_INT8x8
endif

# Depending which shield is used for data collection, add specific DEFINE
ifeq (CY_028_TFT_SHIELD, $(SHIELD_DATA_COLLECTION))
DEFINES+=CY_BMI_160_IMU_I2C=1
endif
ifeq (CY_028_SENSE_SHIELD_v1, $(SHIELD_DATA_COLLECTION))
DEFINES+=CY_BMX_160_IMU_SPI=1
DEFINES+=CY_IMU_SPI=1
endif
ifeq (CY_028_SENSE_SHIELD_v2, $(SHIELD_DATA_COLLECTION))
DEFINES+=CY_BMI_160_IMU_SPI=1
DEFINES+=CY_IMU_SPI=1
endif

# Check if IAR is used with TFLM. If yes, trigger an error
ifeq ($(NN_INFERENCE_ENGINE), $(filter $(NN_INFERENCE_ENGINE),tflm tflm_less))
   ifeq ($(TOOLCHAIN), IAR)
      $(error Only GCC_ARM and ARM toolchains are supported for TFLM inference engine)
   endif
endif

ifeq (CY_028_SENSE_SHIELD_v1, $(SHIELD_DATA_COLLECTION))
DEFINES+=BMI160_CHIP_ID=UINT8_C\(0xD8\)
endif

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=hardfp

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
LDFLAGS=

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom post-build commands to run.
PREBUILD=$(SEARCH_sensor-orientation-bmx160)/bmx160_fix.bash "libs/BMI160_driver/bmi160_defs.h"


################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory. The variable name depends on the
# toolchain used for the build. Refer to the ModusToolbox user guide to get the correct
# variable name for the toolchain used in your build.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# software provided compiler by default).
CY_COMPILER_GCC_ARM_DIR=


# Locate ModusToolbox helper tools folders in default installation
# locations for Windows, Linux, and macOS.
CY_WIN_HOME=$(subst \,/,$(USERPROFILE))
CY_TOOLS_PATHS ?= $(wildcard \
    $(CY_WIN_HOME)/ModusToolbox/tools_* \
    $(HOME)/ModusToolbox/tools_* \
    /Applications/ModusToolbox/tools_*)

# If you install ModusToolbox software in a custom location, add the path to its
# "tools_X.Y" folder (where X and Y are the version number of the tools
# folder). Make sure you use forward slashes.
CY_TOOLS_PATHS+=

# Default to the newest installed tools folder, or the users override (if it's
# found).
CY_TOOLS_DIR=$(lastword $(sort $(wildcard $(CY_TOOLS_PATHS))))

ifeq ($(CY_TOOLS_DIR),)
$(error Unable to find any of the available CY_TOOLS_PATHS -- $(CY_TOOLS_PATHS). On Windows, use forward slashes.)
endif

$(info Tools Directory: $(CY_TOOLS_DIR))

include $(CY_TOOLS_DIR)/make/start.mk
