#include "CallbackHandler.h"
#include <iostream>

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
  std::cout << "got new data >>" << data << "<< len " << len << std::endl;
}
