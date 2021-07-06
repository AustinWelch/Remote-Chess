/*
 * Based on code provided in UT.6.02x Embedded Systems - Shape the World
 * by Jonathan Valvano and Ramesh Yerraballi
 * June 21, 2019
*/

#include "simplelink.h"
#include "sl_common.h"
#include <stdbool.h>


/* Application specific status/error codes */
#define SL_STOP_TIMEOUT 0xFF

typedef enum
{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0, // Choosing this number to avoid overlap with host-driver's error codes
    HTTP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    HTTP_RECV_ERROR = HTTP_SEND_ERROR - 1,
    HTTP_INVALID_RESPONSE = HTTP_RECV_ERROR - 1,

    STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

/*
 * GLOBAL VARIABLES -- Start
 */

_u32 g_Status;

/*
 * GLOBAL VARIABLES -- End
 */


#define MAX_RECV_BUFF_SIZE 1024
#define MAX_SEND_BUFF_SIZE 512
#define SUCCESS 0

/*
 * GLOBAL VARIABLES -- Start
 */

char Recvbuff[MAX_RECV_BUFF_SIZE];
char SendBuff[MAX_SEND_BUFF_SIZE];
unsigned long DestinationIP;
int SockID;
static volatile uint32_t localIP;

extern _i32 establishConnectionWithAP(void);
extern _i32 disconnectFromAP(void);
extern _i32 configureSimpleLinkToDefaultState(void);
extern _i32 sendRequestToServer(char* request);
extern void parseServerResponse(char* parsedResponse, char* keyword);
