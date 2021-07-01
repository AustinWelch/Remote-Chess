#include "cc3100_usage.h"


/****** GLOBAL VARIABLES ******/
_u8 g_Status = 0;
_i32 retVal = 0;
static volatile uint32_t localIP;
uint32_t receivedAlready = 0;
uint32_t transmitedAlready = 0;
_i16          SockIDRx = 0;
_i16          SockIDTx = 0;
/****** GLOBAL VARIABLES ******/


/****************************************** STATIC FUNCTIONS *********************************************/
/*!
    \brief Opening a UDP server side socket and receiving data

    This function opens a UDP socket in Listen mode and waits for incoming
    UDP packets from the connected client.

    \param[in]      port number on which the server will be listening on

    \return         0 on success, Negative value on Error.

    \note

    \warning
 */
static inline _i32 BsdUdpServer(_u16 Port, _u8 *data, _u16 BUF_SIZE)
{
    SlFdSet_t readfds;

    SlSockAddrIn_t  Addr;
    SlSockAddrIn_t  LocalAddr;
    _u16          AddrSize = 0;
    //    _i16          SockID = 0;
    _i16          Status = 0;
    _u16          LoopCount = 0;
    _u16          recvSize = 0;
    _u8           *temp = data;

    LocalAddr.sin_family = SL_AF_INET;
    LocalAddr.sin_port = sl_Htons((_u16)Port);
    LocalAddr.sin_addr.s_addr = 0;

    AddrSize = sizeof(SlSockAddrIn_t);

    // Per documentation , minimum - 10ms
    struct SlTimeval_t timeVal;
    timeVal.tv_sec =  0;                  // Seconds
    timeVal.tv_usec = 50000;             // Microseconds. 10000 microseconds resolution

    /* Open initial socket */
    if(receivedAlready == 0)
    {
        receivedAlready = 1;

        SockIDRx = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);

        sl_SetSockOpt(SockIDRx,SL_SOL_SOCKET,SL_SO_RCVTIMEO, (_u8 *)&timeVal, sizeof(timeVal));    // Enable receive timeout

        SlSockNonblocking_t enableOption;
        enableOption.NonblockingEnabled = 0;
        sl_SetSockOpt(SockIDRx,SL_SOL_SOCKET,SL_SO_NONBLOCKING, (_u8 *)&enableOption,sizeof(enableOption)); // Enable/disable nonblocking mode

        if( SockIDRx < 0 )
        {
            ASSERT_ON_ERROR(SockIDRx);
        }

        Status = sl_Bind(SockIDRx, (SlSockAddr_t *)&LocalAddr, AddrSize);
        if( Status < 0 )
        {
            Status = sl_Close(SockIDRx);
            ASSERT_ON_ERROR(Status);
        }
    }

    SL_FD_ZERO(&readfds);
    SL_FD_SET(SockIDRx, &readfds);

    Status = sl_Select( SockIDRx + 1, &readfds, NULL, NULL, &timeVal ) ;

    if (Status == NULL)
        return NOTHING_RECEIVED;

    if ( SL_FD_ISSET( SockIDRx, &readfds ) )
    {
        while (LoopCount < NO_OF_PACKETS)
        {
            recvSize = BUF_SIZE;
            temp = data;
            do
            {

                // Returns number of bytes received
                Status = sl_RecvFrom(SockIDRx, temp, recvSize, 0,(SlSockAddr_t *)&Addr, (SlSocklen_t*)&AddrSize );

                //                if(Status < 0)
                //                {
                //                    sl_Close(SockIDRx);
                //                    ASSERT_ON_ERROR(Status);
                //                }

                recvSize -= Status;
                temp += 1;
            }while(recvSize > 0);

            LoopCount++;
        }
    }

    //    Status = sl_Close(SockID);
    //    ASSERT_ON_ERROR(Status);

    return SUCCESS;
}

/*!
    \brief Opening a UDP client side socket and sending data

    This function opens a UDP socket and tries to send data to a UDP server
    IP_ADDR waiting on port PORT_NUM.
    Then the function will send 1000 UDP packets to the server.

    \param[in]      port number on which the server will be listening on

    \return         0 on success, -1 on Error.

    \note

    \warning
 */
static inline _i32 BsdUdpClient(_u16 Port, _u8 *data, _u32 IP, _u16 BUF_SIZE)
{
    SlSockAddrIn_t  Addr;
    _u16            AddrSize = 0;
    _i16            Status = 0;
    _u32            LoopCount = 0;

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons((_u16)Port);
    Addr.sin_addr.s_addr = sl_Htonl(IP);

    AddrSize = sizeof(SlSockAddrIn_t);

    /* Open initial socket */
    if(transmitedAlready == 0)
    {
        transmitedAlready = 1;
        SockIDTx = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
        if( SockIDTx < 0 )
        {
            ASSERT_ON_ERROR(SockIDTx);
        }
    }

    while (LoopCount < NO_OF_PACKETS)
    {
        Status = sl_SendTo(SockIDTx, data, BUF_SIZE, 0, (SlSockAddr_t *)&Addr, AddrSize);

        //        if( Status <= 0 )
        //        {
        //            Status = sl_Close(SockIDTx);
        //            ASSERT_ON_ERROR(BSD_UDP_CLIENT_FAILED);
        //        }

        LoopCount++;
    }
    return SUCCESS;
}


/*!
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
static _i32 configureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    _u8           val = 1;
    _u8           configOpt = 0;
    _u8           configLen = 0;
    _u8           power = 0;

    _i32          retVal = -1;
    _i32          mode = -1;

    mode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in station-mode */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while(!IS_IP_ACQUIRED(g_Status)) { _SlNonOsMainLoopTask(); }
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
            ASSERT_ON_ERROR(DEVICE_NOT_IN_STATION_MODE);
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
    if(0 == retVal)
    {
        /* Wait */
        while(IS_CONNECTED(g_Status)) { _SlNonOsMainLoopTask(); }
    }

    /* Enable DHCP client*/
    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);
    ASSERT_ON_ERROR(retVal);

    //    // Disable packet aggregation
    //    uint8_t RxAggrEnable = 0;
    //    sl_NetCfgSet(SL_SET_HOST_RX_AGGR, 0, sizeof(RxAggrEnable), (_u8 *) &RxAggrEnable);


    /* Disable scan */
    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Set Tx power level for station mode
          Number between 0-15, as dB offset from max power - 0 will set maximum power */
    power = 0;
    retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *)&power);
    ASSERT_ON_ERROR(retVal);

    /* Set PM policy to normal */
    retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

    ASSERT_ON_ERROR(retVal);

    /* Unregister mDNS services */
    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove  all 64 filters (8*8) */
    pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                                sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retVal);


    //    retVal = sl_Stop(SL_STOP_TIMEOUT);
    //    ASSERT_ON_ERROR(retVal);

    g_Status = 0;
    return SUCCESS;
}


/*!
    \brief Connecting to a WLAN Access point

    This function connects to the required AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]  None

    \return     0 on success, negative error-code on error

    \note

    \warning    If the WLAN connection fails or we don't acquire an IP address,
                We will be stuck in this function forever.
 */
static _i32 establishConnectionWithAP()
{
    SlSecParams_t secParams = {0};
    _i32 retVal = 0;

    secParams.Key = PASSKEY;
    secParams.KeyLen = pal_Strlen(PASSKEY);
    secParams.Type = SEC_TYPE;

    retVal = sl_WlanConnect(SSID_NAME, pal_Strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(retVal);

    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status))) { _SlNonOsMainLoopTask(); }

    return SUCCESS;
}

/****************************************** STATIC FUNCTIONS *********************************************/


/****************************************** PUBLIC FUNCTIONS *********************************************/

/*
 * This function initializes the CC3100
 * Enters CC3100 into default state
 * Configured as a station with its own static IP defined in cc3100_usage.h
 * Connects to an access point - params defined in sl_common.h
 *
 * Takes in BUF SIZE for packet - size of player struct
 */
void initCC3100(playerType playerRole)
{
//    asm("   CPSIE   I ");
    _i32 retVal = -1;
    g_Status = 0;
    retVal = configureSimpleLinkToDefaultState();
    if(retVal < 0)
    {
        /* Failed to configure the device in its default state */
        LOOP_FOREVER();
    }

    /* Device is configured in default state */

    /*
     * Assumption is that the device is configured in station mode already
     * and it is in its default state
     */
    /* Initializing the CC3100 device */
    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) ||
            (ROLE_STA != retVal) )
    {
        /* Failed to start device */
        LOOP_FOREVER();
    }

    /* Device started as STATION */

    if(playerRole == Client)
    {
        /* Configuring device in DHCP mode */

        _u8 val = 1;

        /* After calling this API device will be configured for DHCP mode */
        retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);
        if(retVal < 0)
            LOOP_FOREVER();

        /* Restart the device */
        retVal = sl_Stop(SL_STOP_TIMEOUT);
        if(retVal < 0)
            LOOP_FOREVER();

        retVal = sl_Start(0, 0, 0);
        if ((retVal < 0) ||
                (ROLE_STA != retVal) )
        {
            /* Failed to start the device */
            LOOP_FOREVER();
        }
    }
    else
    {
        /* Configure static IP address */
        SlNetCfgIpV4Args_t ipV4;
        ipV4.ipV4 = CONFIG_IP;
        ipV4.ipV4Mask = AP_MASK;
        ipV4.ipV4Gateway = AP_GATEWAY;
        ipV4.ipV4DnsServer = AP_DNS;

        /* Configuring device to connect using static IP */

        /* After calling this API device will be configure for static IP address.*/
        retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_STATIC_ENABLE,1,
                              sizeof(SlNetCfgIpV4Args_t), (_u8 *)&ipV4);
        if(retVal < 0)
            LOOP_FOREVER();
    }

    /* Connecting to WLAN AP - Set with static parameters defined at the top
       After this call we will be connected and have IP address */
    retVal = establishConnectionWithAP();
    if(retVal < 0)
    {
        /* Failed to establish connection w/ an AP */
        LOOP_FOREVER();
    }

    /* Connection established w/ AP and IP is acquired */

//    asm("   CPSID   I ");
}


/*
 * Function sends out an array of bytes, defined by the BUF_SIZE in cc3100_usage.h
 */
void SendData(_u8 *data, _u32 IP, _u16 BUF_SIZE)
{
    /* Sending data to UDP server */
    retVal = BsdUdpClient(PORT_NUM, data, IP, BUF_SIZE);

    //    if(retVal < 0)
    //        /* Failed to send data to UDP server */
    //    else
    //        /* Successfully sent data to UDP server */
}

/*
 * Function reads an array of bytes, defined by the BUF_SIZE in cc3100_usage.h
 */
_i32 ReceiveData(_u8 *data, _u16 BUF_SIZE)
{
    /* Receiving data from UDP server */
    retVal = BsdUdpServer(PORT_NUM, data, BUF_SIZE);
    //    if(retVal < 0)
    //        /* Failed to read data from the UDP client */
    //    else
    //        /* Successfully received data from UDP client */
    return retVal;
}


_u32 getLocalIP()
{
    return localIP;
}

/****************************************** PUBLIC FUNCTIONS *********************************************/





/************************************ ASYNCHRONOUS EVENT HANDLERS ****************************************/
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

    //    switch( pSock->Event )
    //    {
    //        case SL_SOCKET_TX_FAILED_EVENT:
    //            /*
    //             * TX Failed
    //             *
    //             * Information about the socket descriptor and status will be
    //             * available in 'SlSockEventData_t' - Applications can use it if
    //             * required
    //             *
    //             * SlSockEventData_u *pEventData = NULL;
    //             * pEventData = & pSock->socketAsyncEvent;
    //             */
    //            switch( pSock->socketAsyncEvent.SockTxFailData.status )
    //            {
    //                case SL_ECLOSE:
    //                    CLI_Write(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\n\r");
    //                    break;
    //                default:
    //                    CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
    //                    break;
    //            }
    //            break;
    //
    //        default:
    //            CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
    //            break;
    //    }
}
/************************************ ASYNCHRONOUS EVENT HANDLERS ****************************************/


