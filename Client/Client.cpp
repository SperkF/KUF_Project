// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"

#include <iostream>

//TODO: extract defines in common file and link fro mall source files that use them..
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


enum class menuOptions_t
{
    START_MENU,
    SEND_VALUE,
    VALUE_SENT
};

using namespace std;

static void contextMenu(int asterixIndex);
static void buildStartMenu(void);
static void buildSendValueMenu(void);
static void buildSentValueMenu(void);


int main(int argc, char *argv[])
{
  
  cout << "Hello World!" << endl;;
  //TODO: change Port number
  unsigned short serverPort = 8000;
  const char *servername = nullptr;

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

  bool res = myComm.Connect(servername, serverPort);

  std::cout << "client started for server " << servername << " at port " << serverPort << " result " << res << std::endl;
  if (res)
  {
    //char inputstr[100];
      std::string inputstring;
    char sndMessage[3];
    menuOptions_t menuContext;
    menuContext = menuOptions_t::START_MENU;
    //TODO: change User Interaction
    int asterixPos = 0;
    do {
        /*
        switch (menuContext)
        {
        case menuOptions_t::START_MENU:
            buildStartMenu();
            break;
        case menuOptions_t::SEND_VALUE:
            break;
        case menuOptions_t::VALUE_SENT:
            break;
        default:
            break;
        }
        */
      std::cout << "To perform Server Handshake press 1 + enter" << std::endl;
      std::cin >> inputstring;
      //memset(inputstr, 0, sizeof(inputstr));
      //std::cin.getline(inputstr, sizeof(inputstr));
      
      if (inputstring.compare("up") == 0)
      {
          if (asterixPos > 0)
          {
              asterixPos--;
          }
      }
      else if (inputstring.compare("down") == 0)
      {
          if (asterixPos < 3)
          {
              asterixPos++;
          }
      }
      
      contextMenu(asterixPos);
      /*
      if (inputstring[0] == '1') {
          sndMessage[0] = CMD_CS_HANDSHAKE_REQUEST;
          sndMessage[1] = '\0';
          myComm.WriteToPartner(&sndMessage[0], sizeof(sndMessage));
          std::cout << "cmd sent to server" << std::endl;
      }
      */
      /*
      std::cout << "sending >>" << inputstr << std::endl;

      // send command
      const char buf = 0x01;
      myComm.WriteToPartner(&buf, 1);
      myComm.WriteToPartner(inputstr, strlen(inputstr) + 1);
      */
      // wait some time - msecs
      //WARNING: in case this delay is to short, the communication wont work properly
      OS_Sleep(1000);

      // check for answers
      while (myComm.IsMessagePending())
       myComm.ProcessMessage();//takes message and calls DataReceived CallBack

    } while (inputstring.compare("ENDE"));// strcmp(inputstring, "ENDE"));
  }
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
static void buildStartMenu(void) {
    //should take place behind the scenes in final version
    std::cout << "To perform Server Handshake press 1 + enter" << std::endl;
    std::cout << "To request spot information press 2 <index> + enter" << std::endl;
}



/**
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
static void buildSentValueMenu(void) {

}


static void contextMenu(int asterixIndex) {
    int menuItemNumber = 4;
    std::string contextMenu[4] = { "HALLO", "GOOD BYE", "GOOSE", "DUCK" };
    for (int i = 0; i < menuItemNumber; i++)
    {
        if (i == asterixIndex)
        {
            std::cout << " (*) " << contextMenu[i] << std::endl;
        }
        else {
            std::cout << "     " << contextMenu[i] << std::endl;
        }
    }
}