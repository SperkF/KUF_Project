#include "ServerCallbackHandler.h"
#include <iostream>
#include <string>
#include <sstream>


//FILE I/O
#include <fstream>
using namespace std;
#define DMX_FILE "DMX_CONFIG.txt"
// special type
typedef struct {
    uint8_t spotIndex;
    uint8_t featureCount;
} spotStruct_t;
// file static varaibles
 spotStruct_t* DMX_Config = NULL;
 uint8_t DMX_Conifg_len = 0;

using std::cout; using std::cin;
using std::endl; using std::string;
using std::to_string;

using namespace std;

#define FS_DEBUG 1

// '\0' character takes up 1-byte
#define STRING_DELIMITER_SIZE 1

//CLIENT TO SERVER Commands
#define CMD_CS_HANDSHAKE_REQUEST 0x01
#define CMD_CS_SETFEATURE 0x02
#define CMD_CS_BREAK_CONNECTION 0x03
//CLient TO SERVER ARGUMENTS
#define ARG_CS_TURN_ALL_LIGHT_OFF 0x01
#define ARG_CS_TURN_ALL_LIGHT_ON 0x02
#define ARG_CS_LEAVE_LIGHT_IN_CUR_STATE 0x03

//SERVER TO CLIENT reply codes
#define ERR_SC_BONA_FIDE 0xBF
#define ERR_SC_CMD_UNKNOWN 0xEA
#define ERR_SC_ARG_ERROR 0xEB
#define ERR_SC_SUPERGAU 0xFF

ServerCallbackHandler::ServerCallbackHandler()
{
  myDll.Init();
}

ServerCallbackHandler::~ServerCallbackHandler()
{
  myDll.ShutDown();
}


void ServerCallbackHandler::NewConnectionCB(const char *hostname)
{
  std::cout << "got new connection from " << hostname << std::endl;


  /*
*   READ in DMX Server Konfig file to learn about available spots and their features
*/
/*
Structure:
//read mail...
No_of_fileLines
Spot_INXED      No_of_Features
*/
  
  string numString;
  int numInt;
  int itemCount = 0;
  int toggleFlag = 0;
  ifstream inFile;
  int offset1 = 1;
  int offset2 = 2;
  inFile.open(DMX_FILE, std::ifstream::in);
  if (inFile.is_open()) {
      cout << "Data read from Config File: " << endl;
      while (inFile >> numString) {
          numInt = stoi(numString);
          //get number of items to read ->to allocate proper memory
          if (itemCount == 0)
          {
              DMX_Conifg_len = numInt;
              DMX_Config = (spotStruct_t*)calloc(DMX_Conifg_len, sizeof(spotStruct_t));
          }
          else {
              //read spot index and availabel feature number for given spot
              if (toggleFlag == 0)
              {
                  (DMX_Config + itemCount - offset1)->spotIndex = numInt;
                  cout << to_string((uint8_t)(DMX_Config + itemCount - offset1)->spotIndex) << " || ";
                  offset1++;
                  toggleFlag++;
              }
              else {
                  (DMX_Config + itemCount - offset2)->featureCount = numInt;
                  cout << to_string((uint8_t)(DMX_Config + itemCount - offset2)->featureCount) << endl;
                  offset2++;
                  toggleFlag--;
                  
              }
          }
          itemCount++;
      }
      inFile.close();
  }
  else {
      cout << "Unable to open file" << endl;
      while (1);
      exit(1); // terminate with error
  }
}

void ServerCallbackHandler::ConnectionLost()
{
  std::cout << "connection lost" << std::endl;
}



bool processetFeatureRequest(char* retMessage, int* retMessageLen, const char* data);


void ServerCallbackHandler::DataReceived(const char *data, unsigned len)
{
    char *retMessage = NULL;
    int retMessageLen = 0;

    uint8_t sentSpotIndex = 0;
    uint8_t sentFeatureIndex = 0;
    uint8_t sentFeatureValue = 0;
    bool argumentCheck = false;

    switch (data[0]) {
    case CMD_CS_HANDSHAKE_REQUEST:
        //processHanshakeRequest(retMessage, &retMessageLen);
        std::cout << "Handshake requested" << std::endl;

        //allocate proper memory -> struct + Header + Delimiter
        retMessageLen = (DMX_Conifg_len * 2) + 2 + STRING_DELIMITER_SIZE;
        retMessage = (char*)calloc(retMessageLen, sizeof(char));

        //HEADER
        retMessage[0] = ERR_SC_BONA_FIDE;
        retMessage[1] = DMX_Conifg_len;

        //DATA
        for (uint8_t i = 0; i < DMX_Conifg_len; i++) {
            *(retMessage + (2 + i*2)) = DMX_Config[i].spotIndex;
            *(retMessage + (2 + i*2 +1)) = DMX_Config[i].featureCount;
        }

        //Needed for lower layer functions ->require string delimiter
        //highest array index = arrayLen-1
        retMessage[retMessageLen-1] = '\0';

        break;


    case CMD_CS_SETFEATURE:
        //processetFeatureRequest(retMessage, &retMessageLen, data);
        sentSpotIndex = data[1];
        sentFeatureIndex = data[2];
        sentFeatureValue = data[3];

        //check to see if spot and feature are available
        for (int i = 0; i < DMX_Conifg_len; i++)
        {
            //find matching spot, and check if the requested feature is within range of spots features
            if ((DMX_Config + i)->spotIndex == sentSpotIndex && (DMX_Config + i)->featureCount <= sentFeatureIndex)
            {
                argumentCheck = true;
            }
        }

        //if the feature is available (as accordence to the config file), set to requested value
        //if not -> reply with error code
        if (argumentCheck == true)
        {
            //set feature
            /*
            *
            *
            * TODO: write dmx code to set the value of the requested spots feature
            *
            *
            */
            //reply to client with ACK
            //allocate proper memory
            retMessageLen = 1 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = ERR_SC_BONA_FIDE;
            retMessage[retMessageLen - 1] = '\0';
        }
        else {
            //allocate proper memory
            retMessageLen = 1 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = ERR_SC_ARG_ERROR;
            retMessage[retMessageLen - 1] = '\0';
        }
        break;

    case CMD_CS_BREAK_CONNECTION:
        break;
    default://Error reply ->comand unknown
        //allocate proper memory
        retMessageLen = 1 + STRING_DELIMITER_SIZE;
        retMessage = (char*)calloc(retMessageLen, sizeof(char));
        retMessage[0] = ERR_SC_CMD_UNKNOWN;
        retMessage[retMessageLen - 1] = '\0';

        myComm->WriteToPartner((const char*)retMessage, retMessageLen - 1);
        break;
    }

#if FS_DEBUG
    for (uint8_t j = 0; j < retMessageLen; j++)
    {
        std::cout << "sever sends: " << to_string((uint8_t)retMessage[j]) << std::endl;
    }
#endif
    myComm->WriteToPartner((const char*)retMessage, retMessageLen - 1);
    //wait for a bit before freeing the memory to avoid crash (data not sent yet)
    OS_Sleep(1000);
    free(retMessage);
}




/**
* FUNCTIONS writen by FS
*/
bool processetFeatureRequest(char* retMessage, int* retMessageLen, const char* data) {
    return false;
}