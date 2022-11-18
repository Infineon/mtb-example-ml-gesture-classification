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
import datetime
from logging import basicConfig, StreamHandler, DEBUG, WARNING, getLogger
import sys

"""
The logging information to make a log directory and to save all log files generated
"""
CUR_DIR = os.path.dirname(os.path.abspath(__file__))
EXEC_TIME = "Magic_wand_CNN_" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
LOG_DIR = os.path.join(CUR_DIR, f"logs/{EXEC_TIME}")
os.makedirs(LOG_DIR, exist_ok=True)  # Create log directory
formatter = "%(levelname)s: %(asctime)s: %(filename)s: %(funcName)s: %(message)s"
basicConfig(filename=f"{LOG_DIR}/{EXEC_TIME}.log", level=DEBUG, format=formatter)
mpl_logger = getLogger("matplotlib")
mpl_logger.setLevel(WARNING)
getLogger().addHandler(StreamHandler(sys.stdout))
logger = getLogger(__name__)
logger.setLevel(DEBUG)
logger.debug(f"{LOG_DIR}/{EXEC_TIME}.log")