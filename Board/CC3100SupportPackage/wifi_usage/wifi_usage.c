/*
 * Based on code provided in UT.6.02x Embedded Systems - Shape the World
 * by Jonathan Valvano and Ramesh Yerraballi
 * June 21, 2019
*/

#include <wifi_usage.h>

char Recvbuff[MAX_RECV_BUFF_SIZE];
char SendBuff[MAX_SEND_BUFF_SIZE];


void parseServerResponse(char* parsedResponse, char* keyword){
    char *pt = 0;
    char *endpt = 0;
    char parsedRecvBuff[MAX_RECV_BUFF_SIZE];

    pt = strstr(Recvbuff, keyword);
    endpt = strstr(Recvbuff, "</span>");
    *endpt = '\0';

    int i = 0;
    if(pt != 0){
        pt += strlen(keyword);
        while(pt < endpt){
            parsedRecvBuff[i] = *pt;
            pt++; i++;
        }
        strcpy(parsedResponse, parsedRecvBuff);
        parsedResponse[i] = '\0';
    } else {
        strcpy(parsedResponse, Recvbuff);
        printf("Could not parse response, received buffer:\n%s", parsedResponse);
        restartWIFI();
    }
}

void restartWIFI(){
    disconnectFromAP();
    sl_Stop(SL_STOP_TIMEOUT);
    if(connectionType == KEEP_CONNECTION){
        sl_Start(0, 0, 0);
        establishConnectionWithAP();
    }
}

int32_t sendRequestToServer(char* request){

    int32_t retVal;
    int32_t ASize = 0;
    SlSockAddrIn_t Addr;
    memset(Recvbuff, 0, MAX_RECV_BUFF_SIZE);

    if (connectionType == CLOSE_CONNECTION){
        sl_Start(0, 0, 0);
        establishConnectionWithAP();
    }

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(PORT);
    Addr.sin_addr.s_addr = sl_Htonl(DestinationIP); // IP to big endian
    ASize = sizeof(SlSockAddrIn_t);
    SockID = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, 0);

    if (SockID >= 0) {
        retVal = sl_Connect(SockID, (SlSockAddr_t *)&Addr, ASize);

        if ((SockID >= 0) && (retVal >= 0)) {
            strcpy(SendBuff, request);
            sl_Send(SockID, SendBuff, strlen(SendBuff), 0);   // Send the HTTP GET/POST
            sl_Recv(SockID, Recvbuff, MAX_RECV_BUFF_SIZE, 0); // Receive response
            sl_Close(SockID);
        }
        else{
            restartWIFI();
            return 0;
        }
    }
    else {
        restartWIFI();
        return 0;
    }

    if(connectionType == CLOSE_CONNECTION){
        disconnectFromAP();
        sl_Stop(SL_STOP_TIMEOUT);
    }

    return 1;
}


/*
    Supplementary Functions
*/

/*
    \brief This function configure the SimpleLink device in its default state. It:
           - Sets the mode to STATION
           - Configures connection policy to Auto and AutoSmartConfig
           - Deletes all the stored profiles
           - Enables DHCP
           - Disables Scan policy
           - Sets Tx power to maximum
           - Sets power policy to normal
           - Unregisters mDNS services
           - Remove all filters

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/

int32_t configureSimpleLinkToDefaultState(void)
{
    SlVersionFull ver = {0};
    _WlanRxFilterOperationCommandBuff_t RxFilterIdMask = {0};

    _u8 val = 1;
    _u8 configOpt = 0;
    _u8 configLen = 0;
    _u8 power = 0;

    int32_t retVal = -1;
    int32_t mode = -1;

    mode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in station-mode */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while (!IS_IP_ACQUIRED(g_Status))
            {
                _SlNonOsMainLoopTask();
            }
        }

        /* Switch to STA role and restart */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(retVal);

        /* Check if the device is in station again */
        if (ROLE_STA != retVal)
        {
            /* We don't want to proceed if the device is not coming up in station-mode */
            ASSERT_ON_ERROR(-0x7D0);
        }
    }

    /* Get the device's version-information */
    configOpt = SL_DEVICE_GENERAL_VERSION;
    configLen = sizeof(ver);
    retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (_u8 *)(&ver));
    ASSERT_ON_ERROR(retVal);

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove all profiles */
    retVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retVal);

    /*
     * Device in station-mode. Disconnect previous connection if any
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
    retVal = sl_WlanDisconnect();
    if (0 == retVal)
    {
        /* Wait */
        while (IS_CONNECTED(g_Status))
        {
            _SlNonOsMainLoopTask();
        }
    }

    /* Enable DHCP client*/
    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &val);
    ASSERT_ON_ERROR(retVal);

    /* Disable scan */
    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN, configOpt, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
    power = 0;
    retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *)&power);
    ASSERT_ON_ERROR(retVal);

    /* Set PM policy to normal */
    retVal = sl_WlanPolicySet(SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Unregister mDNS services */
    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove  all 64 filters (8*8) */
    pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                                sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(retVal);

    //  retVal = initializeAppVariables();
    //  ASSERT_ON_ERROR(retVal);

    return retVal; /* Success */
}


///*!
//    \brief Connecting to a WLAN Access point
//
//    This function connects to the required AP (SSID_NAME).
//    The function will return once we are connected and have acquired IP address
//
//    \param[in]  None
//
//    \return     0 on success, negative error-code on error
//
//    \note
//
//    \warning    If the WLAN connection fails or we don't acquire an IP address,
//                We will be stuck in this function forever.
//*/
int32_t establishConnectionWithAP(void)
{
    SlSecParams_t secParams = {0};
    uint32_t retVal = 0;

    secParams.Key = PASSKEY;
    secParams.KeyLen = PASSKEY_LEN;
    secParams.Type = SEC_TYPE;

    retVal = sl_WlanConnect(SSID_NAME, pal_Strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(retVal);

    /* Wait */
    while ((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status)))
    {
        _SlNonOsMainLoopTask();
    }

    return SUCCESS;
}


///*!
//    \brief Disconnecting from a WLAN Access point
//
//    This function disconnects from the connected AP
//
//    \param[in]      None
//
//    \return         none
//
//    \note
//
//    \warning        If the WLAN disconnection fails, we will be stuck in this function forever.
//*/
int32_t disconnectFromAP(void)
{
    uint32_t retVal = -1;

    /*
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
    retVal = sl_WlanDisconnect();
    if (0 == retVal)
    {
        /* Wait */
        while (IS_CONNECTED(g_Status))
        {
            _SlNonOsMainLoopTask();
        }
    }

    return SUCCESS;
}


/* EVENT HANDLERS */

/*
    \brief This function handles WLAN events
    \param[in]      pWlanEvent is the event passed to the handler
    \return         None
    \note
    \warning
 */
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
    {
        /* [WLAN EVENT] NULL Pointer Error */
        return;
    }

    switch(pWlanEvent->Event)
    {
    case SL_WLAN_CONNECT_EVENT:
    {
        SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

        /*
         * Information about the connected AP (like name, MAC etc) will be
         * available in 'slWlanConnectAsyncResponse_t' - Applications
         * can use it if required
         *
         * slWlanConnectAsyncResponse_t *pEventData = NULL;
         * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
         *
         */
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
        slWlanConnectAsyncResponse_t*  pEventData = NULL;

        CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

        pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

        /* If the user has initiated 'Disconnect' request, 'reason_code' is
         * SL_USER_INITIATED_DISCONNECTION */
        if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
        {
            /* Device disconnected from the AP on application's request */
        }
        else
        {
            /* Device disconnected from the AP on an ERROR..!! */
        }
    }
    break;

    default:
    {
        /* [WLAN EVENT] Unexpected event */
    }
    break;
    }
}

/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication
    \param[in]      pNetAppEvent is the event passed to the handler
    \return         None
    \note
    \warning
 */
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
    {
        /* [NETAPP EVENT] NULL Pointer Error */
        return;
    }

    switch(pNetAppEvent->Event)
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
    {
        SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
        // TODO: THIS IS WHERE I GET THE MF IP ADDRRESS

        SlIpV4AcquiredAsync_t *pEventData = NULL;
        pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
        localIP =  pEventData->ip;

        /*
         * Information about the connection (like IP, gateway address etc)
         * will be available in 'SlIpV4AcquiredAsync_t'
         * Applications can use it if required
         *
         * SlIpV4AcquiredAsync_t *pEventData = NULL;
         * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
         *
         */
    }
    break;

    default:
    {
        /* [NETAPP EVENT] Unexpected event */
    }
    break;
    }
}

/*!
    \brief This function handles callback for the HTTP server events
    \param[in]      pHttpEvent - Contains the relevant event information
    \param[in]      pHttpResponse - Should be filled by the user with the
                    relevant response information
    \return         None
    \note
    \warning
 */
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
    /* [HTTP EVENT] Unexpected event */
}

/*!
    \brief This function handles general error events indication
    \param[in]      pDevEvent is the event passed to the handler
    \return         None
 */
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
    /* [GENERAL EVENT] */
}

/*!
    \brief This function handles socket events indication
    \param[in]      pSock is the event passed to the handler
    \return         None
 */
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
    {
        /* [SOCK EVENT] NULL Pointer Error */
        return;
    }

}
