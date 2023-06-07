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

# Import sys module
import sys
import os
#import click
import serial
import threading
import queue


path = 'gesture_data/'
classes = os.listdir(path)
dash = []
space = []

# Gets all the classes and move the negative class to the back of the list
for x in range(4):
    if classes[x] == 'negative':
        del classes[x]
        classes.append("negative")
        break

for x in range(4):
    total = ''
    for y in range(16 - len(classes[x])):
        total += ' '
    dash.append(total)

with open('gesture_names.h', 'w') as f:
    f.write("/******************************************************************************\n"
            "* (c) 2022, Cypress Semiconductor Corporation (an Infineon company)\n" 
            "* or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.\n"
            "* \n"
            "* This software, including source code, documentation and related materials\n"
            "* (\"Software\") is owned by Cypress Semiconductor Corporation or one of its\n"
            "* affiliates (\"Cypress\") and is protected by and subject to worldwide patent\n"
            "* protection (United States and foreign), United States copyright laws and\n"
            "* international treaty provisions.  Therefore, you may use this Software only\n"
            "* as provided in the license agreement accompanying the software package from\n"
            "* which you obtained this Software (\"EULA\").\n"
            "* \n"
            "* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,\n"
            "* non-transferable license to copy, modify, and compile the Software source\n"
            "* code solely for use in connection with Cypress's integrated circuit products.\n"
            "* Any reproduction, modification, translation, compilation, or representation\n"
            "* of this Software except as specified above is prohibited without the express\n"
            "* written permission of Cypress.\n"
            "* \n"
            "* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,\n"
            "* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED\n"
            "* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress\n"
            "* reserves the right to make changes to the Software without notice. Cypress\n"
            "* does not assume any liability arising out of the application or use of the\n"
            "* Software or any product or circuit described in the Software. Cypress does\n"
            "* not authorize its products for use in any products where a malfunction or\n"
            "* failure of the Cypress product may reasonably be expected to result in\n"
            "* significant property damage, injury or death (\"High Risk Product\"). By\n"
            "* including Cypress's product in a High Risk Product, the manufacturer of such\n"
            "* system or application assumes all risk of such use and in doing so agrees to\n"
            "* indemnify Cypress against all liability.\n"
            "*******************************************************************************/\n\n")
    f.write("/******************************************************************************\n"
            "* File Name:   control.c\n"
            "*\n"
            "* Description: This file contains the names of gestures used when\n"
            "*              generating a model using the provided scripts\n"
            "*\n"
            "* Related Document: README.md\n"
            "*\n"
            "*******************************************************************************/\n\n")
    f.write("const char gesture_one[] = \"" + classes[0] + "\";\n")
    f.write("const char gesture_two[] = \"" + classes[1] + "\";\n")
    f.write("const char gesture_three[] = \"" + classes[2] + "\";\n")
    f.write("const char gesture_four[] = \"" + classes[3] + "\";\n\n")

    f.write("const char dash_ges_one[] = \"" + dash[0] + "\";\n")
    f.write("const char dash_ges_two[] = \"" + dash[1] + "\";\n")
    f.write("const char dash_ges_three[] = \"" + dash[2] + "\";\n")
    f.write("const char dash_ges_four[] = \"" + dash[3] + "\";\n")
