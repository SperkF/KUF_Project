Current Implementation:
Commands:
-> Client to Server
  -) Handshake [CMD] [ARG]
  -) Set Feature [CMD] [spotIndex] [featureIndex] [new_value]
  -) Get Feature [CMD] [spotIndex] [featureIndex]
-> Server to Client
  ..server echos sent command as first byte of message
  ..second byte of reply is errorcode (success or failure)
  ..every byte after that holds information in refference to command sent by the client
 
 Memory:
 -> Server learns about available spots (their DMX channle number and the number of available features) through a textfile called
    "DMX_CONFIG.txt" that resides in the same folder as the executable (DEBUG folder)
 -> Server has data structure that saves the values of all the features (only gets eradicated when the server shuts down)
 

What still needs to be done:
1) implementiere die Ansteuerung der Leuchten per DMX ->insdie ServerCallback.cpp
2) rewrite RFC document
(~not really neccessary: 3) make User Interface more beautifull)


Commands:
# Handshake
Client to Server
    [0x01] [0x01] (Request Handshake + turn all lights off)
    [0x01] [0x02] (Request Handshake + turn all lights on)
    [0x01] [0x03] (Request Handshake + leave lights in current state)
Server Reply
    [0x01] [0xBF] (echod command + sent error code that all went well)
    [0x01] [0xEA] (echod command + sent error code that command is unknown)
    [0x01] [0xEB] (echod command + sent error code that argument is unknown)
    [0x01] [0xFF] (echod command + sent all went wrong erro code ->is sent when something went wrong, but none of the
                   two above errocodes applys)
# Set feature value
Client to Server
    [0x03] [spotIndex] [featureIndex] [new_value] (Request to set new feature + provides server with neccessary info)
Server Reply
    [0x03] [0xBF] (echod command + sent error code that all went well)
    [0x03] [0xEA] (echod command + sent error code that command is unknown)
    [0x03] [0xEB] (echod command + sent error code that argument is unknown)
    [0x03] [0xFF] (echod command + sent all went wrong erro code ->is sent when something went wrong, but none of the
                   two above errocodes applys)
                   
# Get feature value
Client to Server
    [0x02] [spotIndex] [featureIndex] (Request to set new feature + provides server with neccessary info)
Server Reply
    [0x02] [0xBF] [spotIndex] [featureIndex] [requested_value] (echod command + sent error code that all went well + provides client with neccessary info)
    [0x02] [0xEA] (echod command + sent error code that command is unknown)
    [0x02] [0xEB] (echod command + sent error code that argument is unknown)
    [0x02] [0xFF] (echod command + sent all went wrong erro code ->is sent when something went wrong, but none of the
                   two above errocodes applys)
