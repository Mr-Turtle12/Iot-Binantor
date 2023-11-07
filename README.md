# Iot-Binantor
This repository is for my IoT device using ESP32 for university.

Has three folder: 
 - BinESP32 (This script connects to the Ibeacon and will send it's location to the pal esp)
 - PalESP32 (This script takes in if it's bin day and check what's location the bins are in, and display a sad face and makes nose if in the wrong place)
 - WebscrappingBinDayAPI (This should be run on a computer and will run the webscrapping API for the Pal to find out what day it is)
 

To run:

- Place the bin and pal scripts on each of the ESP32 devices. Make sure that the pal ESP has both the LED and mic. Using Arduino
- You will need to have a BLE iBeacon to attach to the bin itself.
- When both are turned on, you will need to connect them to your Wi-Fi. They should appear on your Wi-Fi network, and then it will prompt you to sign in. Follow the instructions and they should connect to your Wi-Fi without any issues. They will remember your Wi-Fi credentials as well.
-Next, you will need to set both the store location and collection location on the bin. You can do this by visiting "http://esp32config". Ensure that the ESP32 (bin) is in its designated location. If you move that ESP, you will need to recalibrate it.


