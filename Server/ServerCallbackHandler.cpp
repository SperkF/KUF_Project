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

typedef struct {
    uint8_t spotIndex;
    uint8_t featureCount;
    uint8_t* featureArray;
} spotStruct2_t;
// file static varaibles
spotStruct2_t* DMX_Config = NULL;
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
#define CMD_CS_GET_FEATURE_VALUE 0x02
#define CMD_CS_SET_FEATURE_VALUE 0x03
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
              DMX_Config = (spotStruct2_t*)calloc(DMX_Conifg_len, sizeof(spotStruct2_t));
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
                  (DMX_Config + itemCount - offset2)->featureArray = (uint8_t*)calloc(numInt, sizeof(uint8_t));
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

ServerCallbackHandler::~ServerCallbackHandler()
{
  myDll.ShutDown();
  /*free allocated memory*/
  if (DMX_Config != NULL)
  {
      for (int i = 0; i < DMX_Conifg_len && (DMX_Config + i)->featureArray != NULL; i++)
      {
          free((DMX_Config + i)->featureArray);
          (DMX_Config + i)->featureArray = NULL;
      }
      free(DMX_Config);
      DMX_Config = NULL;
  }
}


void ServerCallbackHandler::NewConnectionCB(const char *hostname)
{
    std::cout << "got new connection from " << hostname << std::endl;
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
    uint8_t requestedFeatureValue = 0;
    bool argumentCheck = false;

    switch (data[0]) {
    case CMD_CS_HANDSHAKE_REQUEST:
        //processHanshakeRequest(retMessage, &retMessageLen);
        std::cout << "Handshake requested" << std::endl;

        //allocate proper memory -> struct + Header + Delimiter
        retMessageLen = (DMX_Conifg_len * 2) + 3 + STRING_DELIMITER_SIZE;
        retMessage = (char*)calloc(retMessageLen, sizeof(char));

        //HEADER
        retMessage[0] = data[0]; //echo command
        retMessage[1] = ERR_SC_BONA_FIDE;
        retMessage[2] = DMX_Conifg_len;

        //DATA
        for (uint8_t i = 0; i < DMX_Conifg_len; i++) {
            *(retMessage + (3 + i * 2)) = DMX_Config[i].spotIndex;
            *(retMessage + (3 + i * 2 + 1)) = DMX_Config[i].featureCount;
        }

        //Needed for lower layer functions ->require string delimiter
        //highest array index = arrayLen-1
        retMessage[retMessageLen - 1] = '\0';

        for (int i = 0; i < DMX_Conifg_len; i++)
        {
            switch (data[1])
            {
            case ARG_CS_TURN_ALL_LIGHT_ON:
                memset((DMX_Config + i)->featureArray, 255, (DMX_Config + i)->featureCount);
                break;
            case ARG_CS_TURN_ALL_LIGHT_OFF:
                memset((DMX_Config + i)->featureArray, 0, (DMX_Config + i)->featureCount);
                break;
            case ARG_CS_LEAVE_LIGHT_IN_CUR_STATE:
                break;
            }
        }
        break;


    case CMD_CS_GET_FEATURE_VALUE:
        sentSpotIndex = data[1];
        sentFeatureIndex = data[2];

        cout << "client requested value of feature: " << to_string(sentFeatureIndex) << "of spot: " << to_string(sentSpotIndex) << endl;
        //check to see if spot and feature are available
        for (int i = 0; i < DMX_Conifg_len; i++)
        {
            //find matching spot, and check if the requested feature is within range of spots features
            if ((DMX_Config + i)->spotIndex == sentSpotIndex && sentFeatureIndex <= (DMX_Config + i)->featureCount)
            {
                requestedFeatureValue = (DMX_Config + i)->featureArray[sentFeatureIndex-1];
                cout << "requested feature value" << to_string(requestedFeatureValue) << "was sent" << endl;
                argumentCheck = true;
                break;
            }
        }
        if (argumentCheck == true)
        {
            cout << "Feature value found" << endl;
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
            retMessageLen = 5 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = data[0]; //echo command
            retMessage[1] = ERR_SC_BONA_FIDE;
            retMessage[2] = sentSpotIndex; //echo spot index
            retMessage[3] = sentFeatureIndex; //echo feature index
            retMessage[4] = requestedFeatureValue;
            retMessage[retMessageLen - 1] = '\0';
        }
        else {
            //allocate proper memory
            retMessageLen = 2 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = data[0]; //echo command
            retMessage[1] = ERR_SC_ARG_ERROR;
            retMessage[retMessageLen - 1] = '\0';
        }
        break;

    case CMD_CS_SET_FEATURE_VALUE:
        //processetFeatureRequest(retMessage, &retMessageLen, data);
        sentSpotIndex = data[1];
        sentFeatureIndex = data[2];
        sentFeatureValue = data[3];

        cout << "client requested to set feature: " << to_string(sentFeatureIndex) << "of spot: " << to_string(sentSpotIndex) << "to value: " << to_string(sentFeatureValue) << endl;
        //check to see if spot and feature are available
        for (int i = 0; i < DMX_Conifg_len; i++)
        {
            //find matching spot, and check if the requested feature is within range of spots features
            if ((DMX_Config + i)->spotIndex == sentSpotIndex && sentFeatureIndex <= (DMX_Config + i)->featureCount)
            {
                (DMX_Config + i)->featureArray[sentFeatureIndex-1] = sentFeatureValue;
                cout << "Feature can be set to value: " << to_string((DMX_Config + i)->featureArray[sentFeatureIndex-1]) << endl;
                argumentCheck = true;
                break;
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
            retMessageLen = 2 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = data[0]; //echo command
            retMessage[1] = ERR_SC_BONA_FIDE;
            retMessage[retMessageLen - 1] = '\0';
        }
        else {
            //allocate proper memory
            retMessageLen = 2 + STRING_DELIMITER_SIZE;
            retMessage = (char*)calloc(retMessageLen, sizeof(char));
            retMessage[0] = data[0]; //echo command
            retMessage[1] = ERR_SC_ARG_ERROR;
            retMessage[retMessageLen - 1] = '\0';
        }
        break;
    default://Error reply ->comand unknown
        //allocate proper memory
        retMessageLen = 1 + STRING_DELIMITER_SIZE;
        retMessage = (char*)calloc(retMessageLen, sizeof(char));
        retMessage[0] = data[0]; //echo command
        retMessage[1] = ERR_SC_CMD_UNKNOWN;
        retMessage[retMessageLen - 1] = '\0';
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