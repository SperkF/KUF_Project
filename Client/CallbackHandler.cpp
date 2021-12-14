#include "CallbackHandler.h"
#include <iostream>
#include <string>
#include <sstream>



#define FS_DEBUG 1


//CLIENT TO SERVER Commands
#define CMD_HANDSHAKE_REQUEST 0x01
#define CMD_SPOTINFO_REQUEST 0x02

//SERVER TO CLIENT Commands
#define CMD_HANDSHAKE_REPLY 0xF1

//ARGUMETNS AND ERRORCODES
#define ARG_BONA_FIDE 0xBF


using std::cout; using std::cin;
using std::endl; using std::string;
using std::to_string;

using namespace std;

void errorPrinter(uint8_t errCode)
{
	std::cout << "received errorMessage: " << errCode << std::endl;
} 

void CallbackHandler::NewConnectionCB(const char *hostname)
{
  std::cout << "got new connection from " << hostname << std::endl;
}

void CallbackHandler::ConnectionLost()
{
  std::cout << "connection lost" << std::endl;
}

void CallbackHandler::DataReceived(const char *data, unsigned len)
{
  // here we rely on the fact that the data is a string! -> '\0' terminated
  //std::cout << "got new data >>" << data << "<< len " << len << std::endl;
	std::cout << "DATA LEN: " << len << std::endl;
	std::cout << "reply-command from server " << to_string((uint8_t)data[0]) << std::endl;
	std::cout << "argument from server " << to_string((uint8_t)data[1]) << std::endl;

  switch ((uint8_t)data[0])
  {
  case CMD_HANDSHAKE_REPLY:
	  std::cout << "server answered Handshake" << std::endl;
	  std::cout << "highest spot index: " << to_string((uint8_t)data[1]) << std::endl;
	  break;
  case 0xF2:
	  std::cout << "server answered Request Spot Inforamtion" << std::endl;
	  std::cout << "highest feature index: " << data[1] << std::endl;
	  break;
  case 0xF3:
	  if (data[1] != 0xBF)
	  {
		  errorPrinter(data[1]);
	  }
	  else {
		  std::cout << "server answered Request Feature Inforamtion" << std::endl;
		  std::cout << "requested Feature No.: " << to_string((uint8_t)data[1]) << " is available" << std::endl;
	  }
	  break;
  case 0xF4: 
	  if (data[1] != 0xBF)
	  {
		  errorPrinter(data[1]);
	  }
	  else{
		std::cout << "Feature set succefully" << std::endl;
	  }
	  break;
  case 0xF5:
	  if (data[1] != 0xBF)
	  {
		  errorPrinter(data[1]);
	  }
	  else {
		  std::cout << "server acknoledged Disconnect enquiry" << std::endl;
	  }
		  break;
  default:
	  std::cout << "ERROR: default-case inside Client: CallbackHandler.cpp swtich() was hit" << std::endl;
	  break;

  }
}
