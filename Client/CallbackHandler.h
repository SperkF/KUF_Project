#pragma once

#include <Communication.h>

class CallbackHandler : public CommCallbacks
{
public:
  // got connection from host / port
  virtual void NewConnectionCB(const char *hostname);

  virtual void ConnectionLost();

  virtual void DataReceived(const char *data, unsigned len);

};

