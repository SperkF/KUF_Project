#pragma once

#include <Communication.h>



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

#define ERR_MESSAGE_LEN 50
extern char errMessage[ERR_MESSAGE_LEN];

enum class initState_e {
	LIGHT_LEAVE_AS_IS,
	LIGHTS_ALL_ON,
	LIGHTS_ALL_OFF
};
extern initState_e lightState;

enum class conState_e {
	CONNECTED,
	DISCONNECTED
};
extern conState_e connectionState;

typedef struct {
	uint8_t spotIndex;
	uint8_t featureCount;
	uint8_t *featureArray;
} spotStruct2_t;
// file static varaibles
extern spotStruct2_t* DMX_Config;
extern uint8_t DMX_Conifg_len;


class CallbackHandler : public CommCallbacks
{
public:
  // got connection from host / port
  virtual void NewConnectionCB(const char *hostname);

  virtual void ConnectionLost();

  virtual void DataReceived(const char *data, unsigned len);

};

