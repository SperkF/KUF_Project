// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"

#include <iostream>
using namespace std;

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
    char inputstr[100];
    //TODO: change User Interaction
    do {
      memset(inputstr, 0, sizeof(inputstr));
      std::cin.getline(inputstr, sizeof(inputstr));

      std::cout << "sending >>" << inputstr << std::endl;

      // send command
      myComm.WriteToPartner(inputstr, strlen(inputstr) + 1);

      // wait some time - msecs
      OS_Sleep(100);

      // check for answers
      while (myComm.IsMessagePending())
        myComm.ProcessMessage();//takes message and calls DataReceived CallBack

    } while (strcmp(inputstr, "ENDE"));
  }
  myComm.Disconnect();

  return 0;
}



