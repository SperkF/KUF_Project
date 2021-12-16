// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"

#include <iostream>
#include <cstdlib> //for system() functions

char errMessage[ERR_MESSAGE_LEN];
spotStruct2_t* DMX_Config = NULL;
uint8_t DMX_Conifg_len = 0;
initState_e lightState = initState_e::LIGHT_LEAVE_AS_IS;
conState_e connectionState = conState_e::DISCONNECTED;

enum class menuOptions_t
{
	HANDSHAKE_MENU,
	SPOTS_MENU,
	FEATURES_MENU,
};



using namespace std;

static void contextMenu(int asterixIndex);
static void buildStartMenu(void);
static void buildSendValueMenu(void);
static void buildSentValueMenu(void);

static char* handShakeMenu(menuOptions_t* menuContext, int* messageLen);
static spotStruct2_t* spotsMenu(menuOptions_t* menuContext);
static char* featuresMenu(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen);

int main(int argc, char* argv[])
{

	cout << "Hello World!" << endl;;
	//TODO: change Port number
	unsigned short serverPort = 8000;
	const char* servername = nullptr;

	// expect client to connect to as argument
	// if no arg start as server
	if (argc > 1)
	{
		servername = argv[1];
	}
	else
	{
		cerr << "need server name to connect to" << endl;
		return -1;
	}

	std::shared_ptr<CommCallbacks> myCallbackHandler(new CallbackHandler());
	Communication myComm(myCallbackHandler);

	



	int messageLen = 0;
	char* sndMessage = NULL;
	spotStruct2_t* selectedSpot = NULL;
	initState_e lightState = initState_e::LIGHT_LEAVE_AS_IS;

	//attemp to connect to server ->server must be running before client
	bool res = myComm.Connect(servername, serverPort);
	std::cout << "client started for server " << servername << " at port " << serverPort << " result " << res << std::endl;
	bool tryToConnect = true;
	char selectedOption = '1';
	if (res == true)
	{
		while (tryToConnect == true)
		{
			if (res == false) {
				std::cout << "please enter number corresponding to menu option" << endl;

				//display menuOptions
				std::cout << "\n\n\n" << endl;

				std::cout << "Lost Connection" << endl;
				std::cout << "(1) Try to reconnect" << endl;
				std::cout << "(2) terminate client" << endl;

				//go up some line on the terminal to wait for user input
				for (int i = 0; i < 3 + 3; i++)
				{
					cout << "\x1b[A";
				}
				std::cout << "INPUT please: ";
				selectedOption = fgetc(stdin);
				fflush(stdin);

				if (selectedOption == '1')
				{
					system("cls");
					res = myComm.Connect(servername, serverPort);
					if (res != true)
					{
						OS_Sleep(5000);
					}
					OS_Sleep(1000);
				}
				else if (selectedOption == '2')
				{
					tryToConnect = false;
				}
			}
			else {
				std::string inputstring;
				menuOptions_t menuContext;
				menuContext = menuOptions_t::HANDSHAKE_MENU;
				do {
					fflush(stdin);
					system("cls");
					switch (menuContext)
					{
					case menuOptions_t::HANDSHAKE_MENU:
						sndMessage = handShakeMenu(&menuContext, &messageLen);
						break;
					case menuOptions_t::SPOTS_MENU:
						selectedSpot = spotsMenu(&menuContext);
						break;
					case menuOptions_t::FEATURES_MENU:
						if (selectedSpot != NULL)
						{
							sndMessage = featuresMenu(&menuContext, selectedSpot, &messageLen);
						}
						break;
					default:
						break;
					}
					if (sndMessage != NULL)
					{
						myComm.WriteToPartner(&sndMessage[0], sizeof(messageLen));
						free(sndMessage);
						sndMessage = NULL;
					}

					// wait some time - msecs
					//WARNING: in case this delay is to short, the communication wont work properly
					OS_Sleep(1000);

					// check for answers
					while (myComm.IsMessagePending())
						myComm.ProcessMessage();//takes message and calls DataReceived CallBack

				} while (connectionState == conState_e::CONNECTED && res == true);
				//myComm.Disconnect(); //kill thread?
				res = false;
			}
			myComm.Disconnect(); //kill thread?
			system("cls");
			OS_Sleep(300);
		}
	}

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
	OS_Sleep(1000);

	myComm.Disconnect();

	return 0;
}


//instead of menu option Disconnect -> in case user enters "END"..disconnect 
/*
* MENU 1 ->Start Menu
* Following actions are possible:
* (1) Perform Handshake with server *(star when user pressed '1')
* (2) Disconnect from server
* enter respective ASCII-character + <enter> for selection
*/
#define HANDSHAKE_MENU_ITEM_NO 6
static char* handShakeMenu(menuOptions_t* menuContext, int* messageLen) {
	//should take place behind the scenes in final version
	std::cout << "please enter number corresponding to menu option" << endl;
	std::cout << "to go one menu step back enter 0" << std::endl;

	std::string contextMenu[HANDSHAKE_MENU_ITEM_NO] = {
		"perform Handshake (leave lights as is)",
		"perform Handshake (turn all lights on)",
		"perform Handshake (turn all lights off)",
		"disconnect from server (leave lights as is)",
		"disconnect from server (turn all lights on)",
		"disconnect from server (turn all lights off)" };
	//display menuOptions
	cout << "\n\n\n" << endl;
	for (uint8_t i = 0; i < HANDSHAKE_MENU_ITEM_NO; i++)
	{
		cout << " (" << to_string((uint8_t)(i + 1)) << ") " << contextMenu[i] << endl;
	}
	//go up some line on the terminal to wait for user input
	for (int i = 0; i < HANDSHAKE_MENU_ITEM_NO + 3; i++)
	{
		cout << "\x1b[A";
	}
	cout << "INPUT please: ";
	char selectedOption = '0';
	selectedOption = fgetc(stdin);
	char* retMessage = NULL;
	switch (selectedOption)
	{
	case '0':
		break;
	case '1':
		*menuContext = menuOptions_t::SPOTS_MENU;
		lightState = initState_e::LIGHT_LEAVE_AS_IS;
		*messageLen = 2 + STRING_DELIMITER_SIZE;
		retMessage = (char*)calloc(*messageLen, sizeof(char));
		retMessage[0] = CMD_CS_HANDSHAKE_REQUEST;
		retMessage[1] = ARG_CS_LEAVE_LIGHT_IN_CUR_STATE;
		retMessage[*messageLen - 1] = '\0';
		break;
	case '2':
		*menuContext = menuOptions_t::SPOTS_MENU;
		lightState = initState_e::LIGHTS_ALL_ON;
		*messageLen = 2 + STRING_DELIMITER_SIZE;
		retMessage = (char*)calloc(*messageLen, sizeof(char));
		retMessage[0] = CMD_CS_HANDSHAKE_REQUEST;
		retMessage[1] = ARG_CS_TURN_ALL_LIGHT_ON;
		retMessage[*messageLen - 1] = '\0';
		break;
	case '3':
		*menuContext = menuOptions_t::SPOTS_MENU;
		lightState = initState_e::LIGHTS_ALL_OFF;
		*messageLen = 2 + STRING_DELIMITER_SIZE;
		retMessage = (char*)calloc(*messageLen, sizeof(char));
		retMessage[0] = CMD_CS_HANDSHAKE_REQUEST;
		retMessage[1] = ARG_CS_TURN_ALL_LIGHT_OFF;
		retMessage[*messageLen - 1] = '\0';
		break;
	default:
		cout << "entered Menu option out of scope.." << endl;
		OS_Sleep(1000);
		break;
	}
	return &retMessage[0];
}




static spotStruct2_t* spotsMenu(menuOptions_t* menuContext) {
	//should take place behind the scenes in final version
	std::cout << "please enter number corresponding to menu option" << endl;
	std::cout << "to go one menu step back enter 0" << std::endl;

	//display menuOptions
	cout << "\n\n\n" << endl;
	for (uint8_t i = 0; i < DMX_Conifg_len; i++)
	{
		cout << " (" << to_string((uint8_t)(i + 1)) << ") " << "spot with index: " << to_string((uint8_t)(DMX_Config + i)->spotIndex) << " has: " << to_string((uint8_t)(DMX_Config + i)->featureCount) << " features available" << endl;
	}
	//go up some line on the terminal to wait for user input
	for (int i = 0; i < (DMX_Conifg_len)+3; i++)
	{
		cout << "\x1b[A";
	}
	cout << "INPUT please: ";
	int selectedOption = 0;
	cin >> selectedOption;
	if (selectedOption == 0)
	{
		*menuContext = menuOptions_t::HANDSHAKE_MENU;
	}
	else if (selectedOption > 0 && selectedOption <= DMX_Conifg_len)
	{
		*menuContext = menuOptions_t::FEATURES_MENU;
		return &DMX_Config[selectedOption - 1];

		/*selectedSpot->featureCount = DMX_Config[selectedOption - 1].featureCount;
		selectedSpot->spotIndex = DMX_Config[selectedOption - 1].spotIndex;
		std::cout << "read config: " << to_string((uint8_t)selectedSpot->spotIndex) << std::endl;
		std::cout << "read config: " << to_string((uint8_t)selectedSpot->featureCount) << std::endl;
		OS_Sleep(3000);
		*/
	}
	else
	{
		//TODO: wrong input
	}
}

static char* setfeature(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen);
static char* getfeature(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen);

static char* featuresMenu(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen)
{
	char* retMessage = NULL;
	//should take place behind the scenes in final version
	cout << "SPOT with index: " << to_string(selectedSpot->spotIndex) << " currently in scope" << endl;
	std::cout << "please enter number corresponding to menu option" << endl;
	std::cout << "to go one menu step back enter 0" << std::endl;

	//display menuOptions
	cout << "\n" << endl;

	std::cout << "(1) set feature" << endl;
	std::cout << "(2) get feature" << std::endl;

	cout << "\n\n\n" << endl;
	for (uint8_t i = 0; i < selectedSpot->featureCount - 1; i++)
	{
		cout << " (" << to_string((uint8_t)(i + 1)) << ") " << "feature available; value: " << to_string((uint8_t)(selectedSpot->featureArray[i])) << endl;
		//TODO save writen value on client side
	}
	//go up some line on the terminal to wait for user input
	for (int i = 0; i < (selectedSpot->featureCount - 1) + 3; i++)
	{
		cout << "\x1b[A";
	}

	cout << "INPUT please: ";
	char selectedOption = '0';
	selectedOption = fgetc(stdin);
	switch (selectedOption)
	{
	case '0':
		*menuContext = menuOptions_t::SPOTS_MENU;
		break;
	case '1':
		retMessage = setfeature(menuContext, selectedSpot, messageLen);
		break;
	case '2':
		retMessage = getfeature(menuContext, selectedSpot, messageLen);
		break;
	}

	return retMessage;
}

static char* setfeature(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen)
{
	system("cls");
	char* retMessage = NULL;
	//should take place behind the scenes in final version
	std::cout << "please enter number corresponding to menu option" << endl;
	std::cout << "to go one menu step back enter 0" << std::endl;

	//display menuOptions
	cout << "\n\n\n" << endl;
	for (uint8_t i = 0; i < selectedSpot->featureCount - 1; i++)
	{
		cout << " (" << to_string((uint8_t)(i + 1)) << ") " << "feature available; value: " << to_string((uint8_t)(selectedSpot->featureArray[i])) << endl;
		//TODO save writen value on client side
	}
	//go up some line on the terminal to wait for user input
	for (int i = 0; i < (selectedSpot->featureCount - 1) + 3; i++)
	{
		cout << "\x1b[A";
	}
	cout << "set feature: ";
	int featureIndex = 0;
	cin >> featureIndex;

	if (featureIndex == 0)
	{
		*menuContext = menuOptions_t::SPOTS_MENU;
	}
	else if (featureIndex > 0 && featureIndex < selectedSpot->featureCount - 1)
	{
		cout << "to value: ";
		int featureValue = 0;
		cin >> featureValue;
		//selectedSpot = (DMX_Config + selectedOption - 1);
		*messageLen = 4 + STRING_DELIMITER_SIZE;
		retMessage = (char*)calloc(*messageLen, sizeof(char));
		retMessage[0] = CMD_CS_SET_FEATURE_VALUE;
		retMessage[1] = selectedSpot->spotIndex;
		retMessage[2] = featureIndex;
		retMessage[3] = (uint8_t)featureValue;
		retMessage[*messageLen - 1] = '\0';
		*(selectedSpot->featureArray + featureIndex) = featureValue;
	}

	else
	{
		//TODO: wrong input
	}
	return retMessage;
}

static char* getfeature(menuOptions_t* menuContext, spotStruct2_t* selectedSpot, int* messageLen)
{
	system("cls");
	char* retMessage = NULL;
	//should take place behind the scenes in final version
	std::cout << "please enter number corresponding to menu option" << endl;
	std::cout << "to go one menu step back enter 0" << std::endl;

	//display menuOptions
	cout << "\n\n\n" << endl;
	for (uint8_t i = 0; i < selectedSpot->featureCount - 1; i++)
	{
		cout << " (" << to_string((uint8_t)(i + 1)) << ") " << "feature available; value: " << to_string((uint8_t)(selectedSpot->featureArray[i])) << endl;
		//TODO save writen value on client side
	}
	//go up some line on the terminal to wait for user input
	for (int i = 0; i < (selectedSpot->featureCount - 1) + 3; i++)
	{
		cout << "\x1b[A";
	}
	cout << "get feature: ";
	int featureIndex = 0;
	cin >> featureIndex;

	if (featureIndex == 0)
	{
		*menuContext = menuOptions_t::SPOTS_MENU;
	}
	else if (featureIndex > 0 && featureIndex < selectedSpot->featureCount - 1)
	{
		//selectedSpot = (DMX_Config + selectedOption - 1);
		*messageLen = 3 + STRING_DELIMITER_SIZE;
		retMessage = (char*)calloc(*messageLen, sizeof(char));
		retMessage[0] = CMD_CS_GET_FEATURE_VALUE;
		retMessage[1] = selectedSpot->spotIndex;
		retMessage[2] = featureIndex;
		retMessage[*messageLen - 1] = '\0';
	}

	else
	{
		//TODO: wrong input
	}
	return retMessage;
}


/**;
* MENU 2 ->After Handshake<-
* Available Spots:
* Spot 1
*   Fature 1
*   Feature 2
*   ...
* Spot 2
* ...
* Set Feature value:
* Set Spot (wait for user input), Feature (wait for user input) to value of: (wait for user input)
* -> after the input, inform user of succesfull/unsuccsefull write operation
*/
static void buildSendValueMenu(void) {

}


/**
* MENU 3 ->feature was writen
* Writing to Featue was SUCCSEFULL/UNSUCCSEFULL
* possible actions:
* (1) retry
* (2) return to Handshake Menu
* (3) from server
*
*/
static void echoInput(void) {

}

#define MENU_ITEM_NUMBER 5
static void contextMenu(int asterixIndex) {
	std::string contextMenu[MENU_ITEM_NUMBER] = { \
		"perform Handshake (leave lights as is)",
		"perform Handshake (turn all lights on)",
		"perform Handshake (turn all lights off)",
		"show available spots",
		"access specific spot" };
	cout << "\n\n\n" << endl;
	for (int i = 0; i < MENU_ITEM_NUMBER; i++)
	{
		if (i == asterixIndex)
		{
			std::cout << " (*) " << contextMenu[i] << std::endl;
		}
		else {
			std::cout << "     " << contextMenu[i] << std::endl;
		}
	}
	for (int i = 0; i < MENU_ITEM_NUMBER + 3; i++)
	{
		cout << "\x1b[A";
	}
}

static void SpotsMenu(int asterixIndex)
{
	string inputstring;

	//echo input
	char help;
	help = fgetc(stdin);
	fputc(help, stdout);
	//read whole line ->happens when user presses enter
	cin >> inputstring;

	for (int i = 0; i < DMX_Conifg_len; i++)
	{
		cout << "spot: " << (DMX_Config + i)->spotIndex << endl;
	}
}

static void moveAsterix(int* asterix, string myString)
{

}

static void accessSpot(void) {
	int spotIndex;
	cout << "enter index of spot you want to access: ";
	scanf_s("%d", &spotIndex);
}

static void commandLog(string command) {

}

static void showSpots(void) {

}

static void showFeaturesofSpot(int spotIndex) {

}