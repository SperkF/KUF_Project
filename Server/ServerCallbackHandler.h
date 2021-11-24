#pragma once

#include <Communication.h>

#include "DmxDllAccess.h"

class ServerCallbackHandler : public CommCallbacks
{
public:
  ServerCallbackHandler();
  virtual ~ServerCallbackHandler();

  void SetCommunicationLayer(Communication *comm) { myComm = comm; }

  // got connection from host / port
  virtual void NewConnectionCB(const char *hostname);

  virtual void ConnectionLost();

  virtual void DataReceived(const char *data, unsigned len);

private:
  Communication *myComm;
  DmxDllAccess myDll;
};

