#include "CallbackHandler.h"
#include <iostream>
#include <string>
#include <sstream>



using std::cout; using std::cin;
using std::endl; using std::string;
using std::to_string;

using namespace std;



void CallbackHandler::NewConnectionCB(const char *hostname)
{
	connectionState = conState_e::CONNECTED;
  std::cout << "got new connection from " << hostname << std::endl;
}

void CallbackHandler::ConnectionLost()
{
	system("cls");
	std::cout << "connection lost" << std::endl;
	OS_Sleep(3000);
	connectionState = conState_e::DISCONNECTED;
}

void CallbackHandler::DataReceived(const char* data, unsigned len)
{
	// here we rely on the fact that the data is a string! -> '\0' terminated
	//std::cout << "got new data >>" << data << "<< len " << len << std::endl;
	std::cout << "DATA LEN: " << len << std::endl;
	std::cout << "reply-command from server " << to_string((uint8_t)data[0]) << std::endl;
	std::cout << "argument from server " << to_string((uint8_t)data[1]) << std::endl;

	//check reply error code and display message if something went wrong
	switch ((uint8_t)data[1]) {
	case ERR_SC_BONA_FIDE:
		strncpy_s(errMessage, ERR_MESSAGE_LEN, "Command executed succesfully", strlen("Command executed succesfully"));
		break;
	case ERR_SC_CMD_UNKNOWN:
		strncpy_s(errMessage, ERR_MESSAGE_LEN, "Error Command Unkonwn", strlen("Error Command Unkonwn"));
		break;
	case ERR_SC_ARG_ERROR:
		strncpy_s(errMessage, ERR_MESSAGE_LEN, "Error Argument unusable", strlen("Error Argument unusable"));
		break;
	case ERR_SC_SUPERGAU:
		strncpy_s(errMessage, ERR_MESSAGE_LEN, "Error Supergau", strlen("Error Supergau"));
		break;
	}


	//only work with received data if the server ansered with "all good" error reply
	int headerLen = 0;
	if ((uint8_t)data[1] == ERR_SC_BONA_FIDE)
	{
		//server echos last sent command as first byte of reply message
		switch ((uint8_t)data[0])
		{
		case CMD_CS_HANDSHAKE_REQUEST:
			headerLen = 3; //echo cmd + err code + data len
			std::cout << "server answered Handshake" << std::endl;
			DMX_Conifg_len = (len - headerLen) / 2;
			if (DMX_Config == NULL) //allocate only once!
			{
				DMX_Config = (spotStruct2_t*)calloc(DMX_Conifg_len, sizeof(spotStruct2_t));
				for (int i = headerLen, j = 0; i < len - 1; j++)
				{
					(DMX_Config + j)->spotIndex = data[i];
					i++;
					(DMX_Config + j)->featureCount = data[i];
					(DMX_Config + j)->featureArray = (uint8_t*)calloc((DMX_Config + j)->featureCount, sizeof(uint8_t));
					switch (lightState)
					{
					case initState_e::LIGHTS_ALL_OFF:
						memset((DMX_Config + j)->featureArray, 0, (DMX_Config + j)->featureCount);
						break;
					case initState_e::LIGHTS_ALL_ON:
						memset((DMX_Config + j)->featureArray, 255, (DMX_Config + j)->featureCount);
						break;
					case initState_e::LIGHT_LEAVE_AS_IS:
						break;
					}
					i++;
				}
			}
			else {
				for (int i = 0; i < DMX_Conifg_len; i++)
				{
					switch (lightState)
					{
					case initState_e::LIGHTS_ALL_OFF:
						memset((DMX_Config + i)->featureArray, 0, (DMX_Config + i)->featureCount);
						break;
					case initState_e::LIGHTS_ALL_ON:
						memset((DMX_Config + i)->featureArray, 255, (DMX_Config + i)->featureCount);
						break;
					case initState_e::LIGHT_LEAVE_AS_IS:
						break;
					}
				}

			}


#if FS_DEBUG
			for (uint8_t j = 0; j < DMX_Conifg_len; j++)
			{
				std::cout << "read config: " << to_string((uint8_t)(DMX_Config + j)->spotIndex) << std::endl;
				std::cout << "read config: " << to_string((uint8_t)(DMX_Config + j)->featureCount) << std::endl;
			}
#endif
			std::cout << "data len: " << to_string((uint8_t)len) << std::endl;
			break;
		case CMD_CS_SET_FEATURE_VALUE:
			std::cout << "server acknoledged set feature request" << std::endl;
			break;
		case CMD_CS_GET_FEATURE_VALUE:
			//find spot and feature whos value was requested
			for (int i = 0; i < DMX_Conifg_len; i++)
			{
				//find matching spot, and check if the requested feature is within range of spots features
				if ((DMX_Config + i)->spotIndex == data[2] && data[3] <= (DMX_Config + i)->featureCount)
				{
					(DMX_Config + i)->featureArray[(uint8_t)data[3]-1] = data[4];
					break;
				}
			}
			break;
		default:
			std::cout << "ERROR: default-case inside Client: CallbackHandler.cpp swtich() was hit" << std::endl;
			break;

		}
	}
}
