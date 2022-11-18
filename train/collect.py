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
import click
import serial
import threading
import queue

# Hardcode the baudrate to match with the firmware
BAUDRATE = 115200

# Get number of arguments
num_args = len(sys.argv)

# Check if the number of arguments match 
if (num_args != 4):
    print("")
    print("Please use the following syntax")
    print(">>> python collect.py <COM port> <gesture name> <person name>")
    quit()

# Open the serial port to communicate with the kit
ser = serial.Serial()
ser.baudrate = BAUDRATE
ser.port = sys.argv[1]
ser.open()

if ser.is_open:
    print("Listenning to serial port " + sys.argv[1] + " at " + str(BAUDRATE) + " bps")
else:
    print("Error opening serial port")
    exit

print("")
print("-------- Data Collector --------")

gesture_name = sys.argv[2]
person_name  = sys.argv[3]

print("Gesture name: ", gesture_name)
print("Person name : ", person_name)
filename = "output_" + gesture_name + "_" + person_name + ".txt"
abspath = os.path.abspath("") 

# Check if gesture_data/ folder exists, if not created one
path = os.path.join(abspath, "gesture_data")
folder_exists = os.path.exists(path)
if not folder_exists:
    os.mkdir(path)

# Check if gesture_data/<Gesture> folder exists, if not create one
path = os.path.join(abspath, "gesture_data", gesture_name)
folder_exists = os.path.exists(path);
if not folder_exists:
    os.mkdir(path)

# Check if gesture_data/<Gesture>/<filename> exists, if not create one
path = os.path.join(abspath, "gesture_data", gesture_name, filename)
file_exists = os.path.exists(path)
print("")
if file_exists:
    print("Opening current file: " + filename)
    fileHandler = open(path, "a")
else:
    print("Creating new file: " + filename)
    fileHandler = open(path, "w")

# Print instruction message
print("")
print("Prepare to do the gesture '" + sys.argv[2] + "' movement")
print("Press 'c' to start capturing and 's' to stop and exit")

char = 0
sflag = 0
char_queue = queue.Queue()

# Create a console task to place characters to a queue
def console(char_queue):
    while True:
        char_queue.put(click.getchar())

input_thread = threading.Thread(target=console, args=(char_queue,))
input_thread.daemon = True
input_thread.start()

# While loop to check if any command from the terminal was received and 
# if any message from the serial communication was received
while char != 's' and sflag < 100:
    try:
        char = char_queue.get(False)
    except:
        char = 0

    if char == 'c':
        ser.flushInput()
        sflag = 1

    if sflag > 0:
        line = ser.readline().strip()
        strline = line.decode('ascii') + "\n"
        if strline.find("-,-,-,-,-,-") != -1:
            click.echo('.')
            sflag = sflag + 1

        if sflag > 1:
            fileHandler.write(strline)
            
    
ser.close()
fileHandler.close()