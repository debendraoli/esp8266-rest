# ESP8266 REST API


This program allows you to control switches to control your ESP 8266 using rest api.


I have used ths program to control my home appliances using relay module.


ESP 8266 is a fully Ardunio compatible hardware.

The reason I have used this hardware is that it this has built-in wifi module, like an Ardunio UNO.


# Installation

1. Load Additional Board: http://arduino.esp8266.com/stable/package_esp8266com_index.json

2. Install esp8266 from board manager.


# Features

1. Rest Api to control GPIO PIN
2. Auth
3. Get GPIO Status

# Modules Used

1. ESP 8266
2. 8 Channel Relay module
3. 20 female-female jumper wires.

## Additional Libraries Used

1. ArduinoJson


# Endpoints

This webserver exposes two endpoints.


1. Get Status

   Method: `GET`
   
   Endpoint: `/`

Exampe Response:

    {
        "1": true,
        "2": false,
        "3": true,
        "4": true,
        "5": true,
        "6": true,
        "7": false,
        "8": true
    }



2. Set Status

   Method: `POST`

   Endpoint: `/switch` 

Example:

    {
        "1": true,
        "2": false,
        "3": true,
        "4": true,
        "5": true,
        "6": true,
        "7": false,
        "8": true
    }



# Support

Feel free to raise issue for bugs.

Ping me at <debendraoli@gmail.com> for support.
