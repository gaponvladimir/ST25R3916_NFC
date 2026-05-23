#include <stdio.h>

#include "app_nfc.h"
#include "utils.h"
#include "rfal_nfc.h"
#include "rfal_t2t.h"

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
//#include "demo_ce.h"
#endif /* RFAL_FEATURE_LISTEN_MODE */

/** @addtogroup X-CUBE-NFC6_Applications
 *  @{
 */

/** @addtogroup PollingTagDetect
 *  @{
 */

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
/** @defgroup Private_Define
 * @{
 */
/* Definition of the state machine */
#define NFC_ST_NOTINIT               0     /*!< State:  Not initialized        */
#define NFC_ST_START_DISCOVERY       1     /*!< State:  Start Discovery        */
#define NFC_ST_DISCOVERY             2     /*!< State:  Discovery              */

#define NFC_NFCV_BLOCK_LEN           4     /*!< NFCV Block len                      */

#define NFC_NFCV_USE_SELECT_MODE     false /*!< NFCV select mode        */
#define NFC_NFCV_WRITE_TAG           true /*!< NFCV Write Single Block */

/**
  * @}
  */
/*
 ******************************************************************************
 * GLOBAL MACROS
 ******************************************************************************
 */
#define RFAL_FEATURE_NFCV true
/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

#if RFAL_FEATURE_NFC_DEP
/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static uint8_t ndefUriSTcom[] = {0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00, 0x23, 0xc1, 0x01,
                                  0x00, 0x00, 0x00, 0x1c, 0x55, 0x00, 0x68, 0x74, 0x74, 0x70, 0x3a,
                                  0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d,
                                  0x2f, 0x73, 0x74, 0x32, 0x35, 0x2D, 0x64, 0x65, 0x6D, 0x6F };
#endif /* RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
#if RFAL_SUPPORT_MODE_LISTEN_NFCA
/* NFC-A CE config */
/* 4-byte UIDs with first byte 0x08 would need random number for the subsequent 3 bytes.
 * 4-byte UIDs with first byte 0x*F are Fixed number, not unique, use for this demo
 * 7-byte UIDs need a manufacturer ID and need to assure uniqueness of the rest.*/
static uint8_t ceNFCA_NFCID[]     = {0x5F, 'S', 'T', 'M'};    /* =_STM, 5F 53 54 4D NFCID1 / UID (4 bytes) */
static uint8_t ceNFCA_SENS_RES[]  = {0x02, 0x00};             /* SENS_RES / ATQA for 4-byte UID            */
static uint8_t ceNFCA_SEL_RES     = 0x20;                     /* SEL_RES / SAK                             */
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCA */

static uint8_t ceNFCF_nfcid2[]     = {0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
  /* NFC-F CE config */
static uint8_t ceNFCF_SC[]         = {0x12, 0xFC};
static uint8_t ceNFCF_SENSF_RES[]  = {0x01,                                                   /* SENSF_RES                                */
                                  0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,             /* NFCID2                                   */
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00,             /* PAD0, PAD01, MRTIcheck, MRTIupdate, PAD2 */
                                  0x00, 0x00 };                                               /* RD                                       */
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfalNfcDiscoverParam discParam;
static uint8_t              state = NFC_ST_NOTINIT;
static bool                 multiSel;


uint8_t APDU_1[] = {0x00, 0xA4, 0x04, 0x00, 0x0A, 0xA0, 0x00, 0x00, 0x03, 0x97, 0x28, 0x4D, 0x59, 0x41, 0x29, 0x00};
uint8_t APDU_2[] = 	{0x00, 0xA4, 0x04, 0x00, 0x08, 0xA0, 0x00, 0x00, 0x01, 0x51, 0x00, 0x00, 0x00, 0x00};
uint8_t APDU_3[] = 	{0x80, 0xCA, 0x9F, 0x7F, 0x00};

uint8_t APDU_4[] = {0x00, 0xA4, 0x04, 0x00, 0x0A, 0xA0, 0x00, 0x00, 0x03, 0x97, 0x28, 0x4D, 0x59, 0x41, 0x29, 0x00};
uint8_t APDU_5[] = {0x00, 0x20, 0x00, 0x80, 0x04, 0x31, 0x32, 0x33, 0x34};
uint8_t APDU_6[] = {0x00, 0xCB, 0x3F, 0xFF, 0x0A, 0x70, 0x08, 0x84, 0x01, 0x94, 0xA5, 0x03, 0x7F, 0x49, 0x80, 0x00};

uint8_t APDU_GET_REST_DATA[] = {0x00, 0xC0, 0x00, 0x00, 0x0};

static void app_nfc_P2P( rfalNfcDevice *nfcDev );
void app_nfc_APDU( void );
static void app_nfc_Nfcv( rfalNfcvListenDevice *nfcvDev );
static void app_nfc_Nfcf( rfalNfcfListenDevice *nfcfDev );
static void app_nfc_T2t( void );
static void app_nfc_CE( rfalNfcDevice *nfcDev );
static void app_nfc_Notif( rfalNfcState st );
ReturnCode  app_nfc_TransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt );

/*!
 *****************************************************************************
 * \brief Notification
 *
 *  This function receives the event notifications from RFAL
 *****************************************************************************
 */
static void app_nfc_Notif( rfalNfcState st )
{
    uint8_t       devCnt;
    rfalNfcDevice *dev;

    if( st == RFAL_NFC_STATE_WAKEUP_MODE )
    {
        printf("Wake Up mode started \r\n");
    }
    else if( st == RFAL_NFC_STATE_POLL_TECHDETECT )
    {
        if( discParam.wakeupEnabled )
        {
            printf("Wake Up mode terminated. Polling for devices \r\n");
        }
    }
    else if( st == RFAL_NFC_STATE_POLL_SELECT )
    {
        /* Check if in case of multiple devices, selection is already attempted */
        if( (!multiSel) )
        {
            multiSel = true;
            /* Multiple devices were found, activate first of them */
            rfalNfcGetDevicesFound( &dev, &devCnt );
            rfalNfcSelect( 0 );

            printf("Multiple Tags detected: %d \r\n", devCnt);
        }
        else
        {
            rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_DISCOVERY );
        }
    }
    else if( st == RFAL_NFC_STATE_START_DISCOVERY )
    {
        /* Clear mutiple device selection flag */
        multiSel = false;
    }
}

/*!
 *****************************************************************************
 * \brief NFC-F
 *
 * Example how to exchange read and write blocks on a NFC-F tag
 *
 *****************************************************************************
 */
static void app_nfc_Nfcf( rfalNfcfListenDevice *nfcfDev )
{
#if RFAL_FEATURE_NFCF

    uint8_t                    buf[ (RFAL_NFCF_NFCID2_LEN + RFAL_NFCF_CMD_LEN + (3*RFAL_NFCF_BLOCK_LEN)) ];
    uint16_t                   rcvLen;
    rfalNfcfServ               srv = RFAL_NFCF_SERVICECODE_RDWR;
    rfalNfcfBlockListElem      bl[3];
    rfalNfcfServBlockListParam servBlock;
    //uint8_t                    wrData[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    servBlock.numServ   = 1;                            /* Only one Service to be used           */
    servBlock.servList  = &srv;                         /* Service Code: NDEF is Read/Writeable  */
    servBlock.numBlock  = 1;                            /* Only one block to be used             */
    servBlock.blockList = bl;
    bl[0].conf     = RFAL_NFCF_BLOCKLISTELEM_LEN_BIT;   /* Two-byte Block List Element           */
    bl[0].blockNum = 0x0001;                            /* Block: NDEF Data                      */

    ReturnCode err = rfalNfcfPollerCheck( nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
    //printf(" Check Block: %s Data:  %s \r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK", (err != RFAL_ERR_NONE) ? "" : hex2Str( &buf[1], RFAL_NFCF_BLOCK_LEN) );

    #if 0  /* Writing example */
        err = rfalNfcfPollerUpdate( nfcfDev->sensfRes.NFCID2, &servBlock, buf , sizeof(buf), wrData, buf, sizeof(buf) );
        //printf(" Update Block: %s Data: %s \r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK", (err != RFAL_ERR_NONE) ? "" : hex2Str( wrData, RFAL_NFCF_BLOCK_LEN) );
        err = rfalNfcfPollerCheck( nfcfDev->sensfRes.NFCID2, &servBlock, buf, sizeof(buf), &rcvLen);
        //printf(" Check Block:  %s Data: %s \r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK", (err != RFAL_ERR_NONE) ? "" : hex2Str( &buf[1], RFAL_NFCF_BLOCK_LEN) );
    #endif

#endif /* RFAL_FEATURE_NFCF */
}

/*!
 *****************************************************************************
 * \brief NFC-V Exchange
 *
 * Example how to exchange read and write blocks on a NFC-V tag
 *
 *****************************************************************************
 */
static void app_nfc_Nfcv( rfalNfcvListenDevice *nfcvDev )
{
#if RFAL_FEATURE_NFCV

    ReturnCode            err;
    uint16_t              rcvLen;
    uint8_t               blockNum = 1;
    uint8_t               rxBuf[ 1 + NFC_NFCV_BLOCK_LEN + RFAL_CRC_LEN ];                        /* Flags + Block Data + CRC */
    uint8_t               *uid;
    uint8_t               reqFlag;
#if NFC_NFCV_WRITE_TAG
    uint8_t               wrData[NFC_NFCV_BLOCK_LEN] = { 0x11, 0x22, 0x33, 0x99 };             /* Write block example */
#endif /* NFC_NFCV_WRITE_TAG */

    uid     = nfcvDev->InvRes.UID;
    reqFlag = RFAL_NFCV_REQ_FLAG_DEFAULT;

    #if NFC_NFCV_USE_SELECT_MODE
        /*
        * Activate selected state
        */
        err = rfalNfcvPollerSelect( reqFlag, nfcvDev->InvRes.UID );
        printf(" Select %s \r\n", (err != RFAL_ERR_NONE) ? "FAIL (revert to addressed mode)": "OK" );
        if( err == RFAL_ERR_NONE )
        {
            reqFlag = (RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_SELECT);
            uid     = NULL;
        }
    #endif /* NFC_NFCV_USE_SELECT_MODE */

    /*
    * Read block using Read Single Block command
    * with addressed mode (uid != NULL) or selected mode (uid == NULL)
    */
    err = rfalNfcvPollerReadSingleBlock(reqFlag, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    //printf(" Read Block: %s %s\r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK Data:", (err != RFAL_ERR_NONE) ? "" : hex2Str( &rxBuf[1], NFC_NFCV_BLOCK_LEN));

    #if NFC_NFCV_WRITE_TAG /* Writing example */
        err = rfalNfcvPollerWriteSingleBlock(reqFlag, uid, blockNum, wrData, sizeof(wrData));
        //printf(" Write Block: %s Data: %s\r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK", hex2Str( wrData, NFC_NFCV_BLOCK_LEN) );
        err = rfalNfcvPollerReadSingleBlock(reqFlag, uid, blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
        //printf(" Read Block: %s %s\r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK Data:", (err != RFAL_ERR_NONE) ? "" : hex2Str( &rxBuf[1], NFC_NFCV_BLOCK_LEN));
    #endif /* NFC_NFCV_WRITE_TAG */

#endif /* RFAL_FEATURE_NFCV */
}

/*!
 *****************************************************************************
 * \brief T2T Exchange
 *
 * Example how to exchange read blocks on a T2T tag
 *
 *****************************************************************************
 */
static void app_nfc_T2t( void )
{
#if RFAL_FEATURE_T2T
    ReturnCode            err;
    uint16_t              rcvLen;
    uint8_t               blockNum = 0;
    uint8_t               rxBuf[ RFAL_T2T_READ_DATA_LEN ];

    err = rfalT2TPollerRead(blockNum, rxBuf, sizeof(rxBuf), &rcvLen);
    //printf(" Read Block: %s %s\r\n", (err != RFAL_ERR_NONE) ? "FAIL": "OK Data:", (err != RFAL_ERR_NONE) ? "" : hex2Str( rxBuf, RFAL_T2T_READ_DATA_LEN));

#endif
}

/*!
 *****************************************************************************
 * \brief  P2P Exchange
 *
 * Sends a NDEF URI record 'http://www.ST.com' via NFC-DEP (P2P) protocol.
 *
 * This method sends a set of static predefined frames which tries to establish
 * a LLCP connection, followed by the NDEF record, and then keeps sending
 * LLCP SYMM packets to maintain the connection.
 *
 *
 *****************************************************************************
 */
void app_nfc_P2P( rfalNfcDevice *nfcDev )
{
#if RFAL_FEATURE_NFC_DEP

    uint16_t   *rxLen;
    uint8_t    *rxData;
    ReturnCode err;

    /* In Listen mode retrieve the first request from Initiator */
    if( nfcDev->type == RFAL_NFC_POLL_TYPE_AP2P )
    {
        app_nfc_TransceiveBlocking( NULL, 0, &rxData, &rxLen, 0);

        /* Initiator request is being ignored/discarded  */
    }

    printf(" Initialize device .. ");
    err = app_nfc_TransceiveBlocking( ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != RFAL_ERR_NONE )
    {
        printf("failed.\r\n");
        return;
    }
    printf("succeeded.\r\n");

    printf(" Push NDEF Uri: www.st.com/st25-demo ... ");
    err = app_nfc_TransceiveBlocking( ndefUriSTcom, sizeof(ndefUriSTcom), &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != RFAL_ERR_NONE )
    {
        printf("failed.\r\n");
        return;
    }
    printf("succeeded.\r\n");

    printf(" Device present, maintaining connection ");
    while(err == RFAL_ERR_NONE)
    {
        err = app_nfc_TransceiveBlocking( ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
        printf(".");
        platformDelay(50);
    }
    printf("\r\n Device removed.\r\n");

#endif /* RFAL_FEATURE_NFC_DEP */
}

/*!
 *****************************************************************************
 * \brief NFC Cycle
 *
 *  This function executes the state machine.
 *  It must be called periodically
 *****************************************************************************
 */
void app_nfc_Cycle( void )
{
    static rfalNfcDevice *nfcDevice;

    rfalNfcWorker();                                    /* Run RFAL worker periodically */

    //printf("State: %d\n\r",state);

    switch( state )
    {
        /*******************************************************************************/
        case NFC_ST_START_DISCOVERY:

          rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );
          rfalNfcDiscover( &discParam );

          multiSel = false;
          state    = NFC_ST_DISCOVERY;
          break;

        /*******************************************************************************/
        case NFC_ST_DISCOVERY:
            if( rfalNfcIsDevActivated( rfalNfcGetState() ) )
            {
                rfalNfcGetActiveDevice( &nfcDevice );

                switch( nfcDevice->type )
                {
                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCA:

                        // platformLedOn(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
                        switch( nfcDevice->dev.nfca.type )
                        {
                            case RFAL_NFCA_T1T:
                                //printf("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                                break;

                            case RFAL_NFCA_T4T:
                                //printf("NFCA Passive ISO-DEP device found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );

                                app_nfc_APDU();
                                // my add
                                state = NFC_ST_NOTINIT;
                                printf("New state: %d\n\r", state);
                                break;

                            case RFAL_NFCA_T4T_NFCDEP:
                            case RFAL_NFCA_NFCDEP:
                                //printf("NFCA Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );

                                app_nfc_P2P( nfcDevice );
                                break;

                            default:
                                //printf("ISO14443A/NFC-A card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );

                                app_nfc_T2t();
                                break;
                        }
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCB:

                        //printf("ISO14443B/NFC-B card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                        // platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);

                        if( rfalNfcbIsIsoDepSupported( &nfcDevice->dev.nfcb ) )
                        {
                            app_nfc_APDU();
                        }
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCF:

                        if( rfalNfcfIsNfcDepSupported( &nfcDevice->dev.nfcf ) )
                        {
                            //printf("NFCF Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
                            app_nfc_P2P( nfcDevice );
                        }
                        else
                        {
                            //printf("Felica/NFC-F card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));

                            app_nfc_Nfcf( &nfcDevice->dev.nfcf );
                        }

                        // platformLedOn(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCV:
                        {
                            uint8_t devUID[RFAL_NFCV_UID_LEN];

                            ST_MEMCPY( devUID, nfcDevice->nfcid, nfcDevice->nfcidLen );   /* Copy the UID into local var */
                            REVERSE_BYTES( devUID, RFAL_NFCV_UID_LEN );                 /* Reverse the UID for display purposes */
                            //printf("ISO15693/NFC-V card found. UID: %s\r\n", hex2Str(devUID, RFAL_NFCV_UID_LEN));

                            //// platformLedOn(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);

                            Qx3900NFC_AllBlock_Write( &nfcDevice->dev.nfcv );
                        }
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_ST25TB:

                        //printf("ST25TB card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));
                        // platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_AP2P:
                    case RFAL_NFC_POLL_TYPE_AP2P:

                        //printf("NFC Active P2P device found. NFCID3: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
                        // platformLedOn(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);

                        app_nfc_P2P( nfcDevice );
                        break;

                    /*******************************************************************************/
                    case RFAL_NFC_POLL_TYPE_NFCA:
                    case RFAL_NFC_POLL_TYPE_NFCF:

                        printf("Activated in CE %s mode.\r\n", (nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? "NFC-A" : "NFC-F");
                        // platformLedOn( ((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PORT : PLATFORM_LED_F_PORT),
                                       //((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PIN  : PLATFORM_LED_F_PIN)  );

                        app_nfc_CE( nfcDevice );
                        break;

                    /*******************************************************************************/
                    default:
                        break;
                }

               // rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );
            }
            break;

        /*******************************************************************************/
        case NFC_ST_NOTINIT:
        default:
            break;
    }
}

static void app_nfc_CE( rfalNfcDevice *nfcDev )
{
#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE

    ReturnCode err = RFAL_ERR_NONE;
    uint8_t *rxData;
    uint16_t *rcvLen;
    uint8_t  txBuf[150];
    uint16_t txLen;

    do
    {
        rfalNfcWorker();

        switch( rfalNfcGetState() )
        {
            case RFAL_NFC_STATE_ACTIVATED:
                err = app_nfc_TransceiveBlocking( NULL, 0, &rxData, &rcvLen, 0);
                break;

            case RFAL_NFC_STATE_DATAEXCHANGE:
            case RFAL_NFC_STATE_DATAEXCHANGE_DONE:

                txLen = ( (nfcDev->type == RFAL_NFC_POLL_TYPE_NFCA) ? demoCeT4T( rxData, *rcvLen, txBuf, sizeof(txBuf) ) : rfalConvBytesToBits( demoCeT3T( rxData, rfalConvBitsToBytes(*rcvLen), txBuf, sizeof(txBuf) ) ) );
                err   = app_nfc_TransceiveBlocking( txBuf, txLen, &rxData, &rcvLen, RFAL_FWT_NONE );
                break;

            case RFAL_NFC_STATE_START_DISCOVERY:
                return;

            case RFAL_NFC_STATE_LISTEN_SLEEP:
            default:
                break;
        }
    }
    while( (err == RFAL_ERR_NONE) || (err == RFAL_ERR_SLEEP_REQ) );

#else
    NO_WARNING(nfcDev);
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */
}

/*!
 *****************************************************************************
 * \brief Demo Blocking Transceive
 *
 * Helper function to send data in a blocking manner via the rfalNfc module
 *
 * \warning A protocol transceive handles long timeouts (several seconds),
 * transmission errors and retransmissions which may lead to a long period of
 * time where the MCU/CPU is blocked in this method.
 * This is a demo implementation, for a non-blocking usage example please
 * refer to the Examples available with RFAL
 *
 * \param[in]  txBuf      : data to be transmitted
 * \param[in]  txBufSize  : size of the data to be transmited
 * \param[out] rxData     : location where the received data has been placed
 * \param[out] rcvLen     : number of data bytes received
 * \param[in]  fwt        : FWT to be used (only for RF frame interface,
 *                                          otherwise use RFAL_FWT_NONE)
 *
 *
 *  \return ERR_PARAM     : Invalid parameters
 *  \return ERR_TIMEOUT   : Timeout error
 *  \return ERR_FRAMING   : Framing error detected
 *  \return ERR_PROTO     : Protocol error detected
 *  \return RFAL_ERR_NONE      : No error, activation successful
 *
 *****************************************************************************
 */
ReturnCode app_nfc_TransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt )
{
    ReturnCode err;

    err = rfalNfcDataExchangeStart( txBuf, txBufSize, rxData, rcvLen, fwt );
    if( err == RFAL_ERR_NONE )
    {
        do{
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        }
        while( err == RFAL_ERR_BUSY );
    }
    return err;
}

/*!
 *****************************************************************************
 * \brief Demo APDUs Exchange
 *
 * Example how to exchange a set of predefined APDUs with PICC. The NDEF
 * application will be selected and then CC will be selected and read.
 *
 *****************************************************************************
 */
void app_nfc_APDU( void )
{
    ReturnCode err;
    uint16_t   *rxLen;
    uint8_t    *rxData;

    err = app_nfc_TransceiveBlocking( APDU_1, sizeof(APDU_1), &rxData, &rxLen, RFAL_FWT_NONE );
    printf("\r\nSend APDU_1: %s", (err != RFAL_ERR_NONE) ? "FAIL\r\n": "OK");
    if(err == RFAL_ERR_NONE) print_hex(rxData, *rxLen);

    err = app_nfc_TransceiveBlocking( APDU_2, sizeof(APDU_2), &rxData, &rxLen, RFAL_FWT_NONE );
    printf("\r\nSend APDU_2: %s", (err != RFAL_ERR_NONE) ? "FAIL\r\n": "OK");
    if(err == RFAL_ERR_NONE) print_hex(rxData, *rxLen);

    err = app_nfc_TransceiveBlocking( APDU_3, sizeof(APDU_3), &rxData, &rxLen, RFAL_FWT_NONE );
    printf("\r\nSend APDU_3: %s", (err != RFAL_ERR_NONE) ? "FAIL\r\n": "OK");
    if(err == RFAL_ERR_NONE) print_hex(rxData, *rxLen);

    err = app_nfc_TransceiveBlocking( APDU_4, sizeof(APDU_4), &rxData, &rxLen, RFAL_FWT_NONE );
    printf("\r\nSend APDU_4: %s", (err != RFAL_ERR_NONE) ? "FAIL\r\n": "OK");
    if(err == RFAL_ERR_NONE) print_hex(rxData, *rxLen);

    err = app_nfc_TransceiveBlocking( APDU_6, sizeof(APDU_6), &rxData, &rxLen, RFAL_FWT_NONE );
    printf("\r\nSend APDU_6: %s", (err != RFAL_ERR_NONE) ? "FAIL\r\n": "OK");
    if(err == RFAL_ERR_NONE) {
		if(rxData[*rxLen - 2] == 0x61) {
			print_hex(rxData, *rxLen-2);
		} else {
			print_hex(rxData, *rxLen);
		}

		while(rxData[*rxLen - 2] == 0x61) {
			err = app_nfc_TransceiveBlocking( APDU_GET_REST_DATA, sizeof(APDU_GET_REST_DATA), &rxData, &rxLen, RFAL_FWT_NONE );
			if(err == RFAL_ERR_NONE) {
				if(rxData[*rxLen - 2] == 0x61) {
					print_hex(rxData, *rxLen-2);
				} else {
					print_hex(rxData, *rxLen);
				}
			}
		}
    }
}

/*!
 *****************************************************************************
 * \brief Demo Ini
 *
 *  This function Initializes the required layers for the demo
 *
 * \return true  : Initialization ok
 * \return false : Initialization failed
 *****************************************************************************
 */
bool app_nfc_Ini( void )
{
    ReturnCode err;

    err = rfalNfcInitialize();

    if( err == RFAL_ERR_NONE )
    {

        rfalNfcDefaultDiscParams( &discParam );

        discParam.devLimit      = 1U;

        ST_MEMCPY( &discParam.nfcid3, NFCID3, sizeof(NFCID3) );
        ST_MEMCPY( &discParam.GB, GB, sizeof(GB) );
        discParam.GBLen         = sizeof(GB);
        discParam.p2pNfcaPrio   = true;

        discParam.notifyCb             = app_nfc_Notif;
        discParam.totalDuration        = 1000U;
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;          /* For the demo, enable the NFC Technlogies based on RFAL Feature switches */

#if RFAL_FEATURE_NFCA
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_A;
#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_B;
#endif /* RFAL_FEATURE_NFCB */

#if RFAL_FEATURE_NFCF
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_F;
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_V;
#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_ST25TB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_ST25TB;
#endif /* RFAL_FEATURE_ST25TB */

#if ST25R95
        discParam.isoDepFS           = RFAL_ISODEP_FSXI_128;          /* ST25R95 cannot support 256 bytes of data block */
#endif /* ST25R95 */

#if RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP
        discParam.techs2Find |= RFAL_NFC_POLL_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE
        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE */

#if NFC_CARD_EMULATION_ONLY
        discParam.totalDuration        = 60000U;              /* 60 seconds */
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;  /* Overwrite any previous poller modes */
#endif /* NFC_CARD_EMULATION_ONLY */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
        demoCeInit( ceNFCF_nfcid2 );

#if RFAL_SUPPORT_MODE_LISTEN_NFCA
        /* Set configuration for NFC-A CE */
        ST_MEMCPY( discParam.lmConfigPA.SENS_RES, ceNFCA_SENS_RES, RFAL_LM_SENS_RES_LEN );     /* Set SENS_RES / ATQA */
        ST_MEMCPY( discParam.lmConfigPA.nfcid, ceNFCA_NFCID, RFAL_LM_NFCID_LEN_04 );           /* Set NFCID / UID */
        discParam.lmConfigPA.nfcidLen = RFAL_LM_NFCID_LEN_04;                                  /* Set NFCID length to 7 bytes */
        discParam.lmConfigPA.SEL_RES  = ceNFCA_SEL_RES;                                        /* Set SEL_RES / SAK */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_A;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCA */

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
        /* Set configuration for NFC-F CE */
        ST_MEMCPY( discParam.lmConfigPF.SC, ceNFCF_SC, RFAL_LM_SENSF_SC_LEN );                 /* Set System Code */
        ST_MEMCPY( &ceNFCF_SENSF_RES[RFAL_NFCF_CMD_LEN], ceNFCF_nfcid2, RFAL_NFCID2_LEN );     /* Load NFCID2 on SENSF_RES */
        ST_MEMCPY( discParam.lmConfigPF.SENSF_RES, ceNFCF_SENSF_RES, RFAL_LM_SENSF_RES_LEN );  /* Set SENSF_RES / Poll Response */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_F;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */

        /* Check for valid configuration by calling Discover once */
        err = rfalNfcDiscover( &discParam );
        rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );

        if( err != RFAL_ERR_NONE )
        {
        	printf("Discover failed %d\n\r", err);
            return false;
        }

        state = NFC_ST_START_DISCOVERY;
        return true;
    }
    return false;
}

void app_nfc_start_discovery(void)
{
    rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );
    rfalNfcDiscover( &discParam );

    multiSel = false;
    //state    = NFC_ST_DISCOVERY;
}

void app_nfc_stop_discovery(void)
{
	rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_DISCOVERY );
}

int app_nfc_get_ATR(uint8_t *atr)
{
	static rfalNfcDevice *nfcDevice;
	bool found= false;

	rfalNfcWorker();                                    /* Run RFAL worker periodically */

	//if(state != NFC_ST_DISCOVERY) {
	//	return (-1);
	//}

	if( rfalNfcIsDevActivated( rfalNfcGetState() ) ) {
		rfalNfcGetActiveDevice( &nfcDevice );

		switch( nfcDevice->type ) {
			/*******************************************************************************/
			case RFAL_NFC_LISTEN_TYPE_NFCA:
				//// platformLedOn(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
				switch( nfcDevice->dev.nfca.type ) {
					case RFAL_NFCA_T1T:
						//printf("ISO14443A/Topaz (NFC-A T1T) TAG found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	                    break;

	                case RFAL_NFCA_T4T:
	                	//printf("NFCA Passive ISO-DEP device found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	                	// ToDo : return ATR
	                	found = true;

	                	printf("Got RATS: ");
	                	print_hex(nfcDevice->proto.isoDep.activation.A.Listener.ATS.HB, nfcDevice->proto.isoDep.activation.A.Listener.ATSLen-5);

	                    //app_nfc_APDU();
	                	break;

	                case RFAL_NFCA_T4T_NFCDEP:
	                case RFAL_NFCA_NFCDEP:
	                	//printf("NFCA Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	                    //app_nfc_P2P( nfcDevice );
	                    break;

	                default:
	                	//printf("ISO14443A/NFC-A card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	                	printf("ISO14443A/NFC-A card found, UID: ");
	                	print_hex(nfcDevice->nfcid, nfcDevice->nfcidLen);
	                    //app_nfc_T2t();
	                    break;
	                }
	                break;

	        /*******************************************************************************/
	        case RFAL_NFC_LISTEN_TYPE_NFCB:
	        	//printf("ISO14443B/NFC-B card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	            //// platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);

	            if( rfalNfcbIsIsoDepSupported( &nfcDevice->dev.nfcb ) ) {
	            	//app_nfc_APDU();
	            }
	            break;

	         /*******************************************************************************/
	         case RFAL_NFC_LISTEN_TYPE_NFCF:
	        	 if( rfalNfcfIsNfcDepSupported( &nfcDevice->dev.nfcf ) ) {
	        		 //printf("NFCF Passive P2P device found. NFCID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ) );
	                 //app_nfc_P2P( nfcDevice );
	             } else {
	            	 //printf("Felica/NFC-F card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));
	                 //app_nfc_Nfcf( &nfcDevice->dev.nfcf );
	             }
	             //// platformLedOn(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
	             break;

	         /*******************************************************************************/
	         case RFAL_NFC_LISTEN_TYPE_NFCV:
	        	 uint8_t devUID[RFAL_NFCV_UID_LEN];

	             ST_MEMCPY( devUID, nfcDevice->nfcid, nfcDevice->nfcidLen );   /* Copy the UID into local var */
	             REVERSE_BYTES( devUID, RFAL_NFCV_UID_LEN );                 /* Reverse the UID for display purposes */
	             //printf("ISO15693/NFC-V card found. UID: %s\r\n", hex2Str(devUID, RFAL_NFCV_UID_LEN));
	             //// platformLedOn(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
	             //Qx3900NFC_AllBlock_Write( &nfcDevice->dev.nfcv );
	             break;

	         /*******************************************************************************/
	         case RFAL_NFC_LISTEN_TYPE_ST25TB:
	        	 //printf("ST25TB card found. UID: %s\r\n", hex2Str( nfcDevice->nfcid, nfcDevice->nfcidLen ));
	             //// platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
	             break;

	         /*******************************************************************************/
	         case RFAL_NFC_LISTEN_TYPE_AP2P:
	         case RFAL_NFC_POLL_TYPE_AP2P:
	        	 //printf("NFC Active P2P device found. NFCID3: %s\r\n", hex2Str(nfcDevice->nfcid, nfcDevice->nfcidLen));
	             //// platformLedOn(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
	             //app_nfc_P2P( nfcDevice );
	             break;

	         /*******************************************************************************/
	         case RFAL_NFC_POLL_TYPE_NFCA:
	         case RFAL_NFC_POLL_TYPE_NFCF:
	        	 printf("Activated in CE %s mode.\r\n", (nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? "NFC-A" : "NFC-F");
	             //// platformLedOn( ((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PORT : PLATFORM_LED_F_PORT),
	             //                   ((nfcDevice->type == RFAL_NFC_POLL_TYPE_NFCA) ? PLATFORM_LED_A_PIN  : PLATFORM_LED_F_PIN)  );
	             //app_nfc_CE( nfcDevice );
	             break;

	         /*******************************************************************************/
	         default:
	        	 break;
		}
	}

	if(found) {
		memcpy(atr, nfcDevice->nfcid, nfcDevice->nfcidLen);
		return nfcDevice->nfcidLen;
	}

	return 0;
}
