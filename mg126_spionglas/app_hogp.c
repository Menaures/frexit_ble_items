#include <Arduino.h>
#include <string.h>
#include "bsp.h"
#include "mg_api.h"


#include "mg126_ble_item.h"



/// Characteristic Properties Bit
#define ATT_CHAR_PROP_RD                            0x02
#define ATT_CHAR_PROP_W_NORSP                       0x04
#define ATT_CHAR_PROP_W                             0x08
#define ATT_CHAR_PROP_NTF                           0x10
#define ATT_CHAR_PROP_IND                           0x20
#define GATT_PRIMARY_SERVICE_UUID                   0x2800

#define TYPE_PRESENTATION_FORMAT  0x2904
#define TYPE_CHAR                 0x2803
#define TYPE_CFG                  0x2902
#define TYPE_INFO                 0x2901
#define TYPE_xRpRef               0x2907
#define TYPE_RpRef                0x2908
#define TYPE_INC                  0x2802
#define UUID16_FORMAT             0xff


// ===== Device Information Service =====
#define SOFTWARE_INFO "v1.0"
#define MANU_INFO     "FREXIT"
// ======================================

// ===== GAP =====
char DeviceInfo[9] =  "Spionglas";  /*max len is 24 bytes*/
// ===============

// ===== Item Service =====
char itemCharacteristicUserDescription[18] = "Show item solution";
byte cccd[2] = { 0, 0 };
// Format:1, Exponent:1, Unit:2, NameSpace:1, Description:2
byte itemCharacteristicPresentationFormat[7] = {0x01, 0x00, 0x00, 0x27};


uint16_t cur_notifyhandle = 0x21;


uint8_t* getDeviceInfoData(uint8_t* len)
{
  *len = sizeof(DeviceInfo);
  return (uint8_t*)DeviceInfo;
}


void updateDeviceInfoData(uint8_t* name, uint8_t len)
{
  memcpy(DeviceInfo, name, len);
  ble_set_name(name, len);
}


/**********************************************************************************
                 *****DataBase****

  01 - 06  GAP (Primary service) 0x1800
  03:04  name
  07 - 0f  Device Info (Primary service) 0x180a
  0a:0b  firmware version
  0e:0f  software version
  10 - 19  LED service (Primary service) 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
  11:12  6E400003-B5A3-F393-E0A9-E50E24DCCA9E(0x04)  RxNotify
  13     cfg
  14:15  6E400002-B5A3-F393-E0A9-E50E24DCCA9E(0x0C)  Tx
  16     cfg
  17:18  6E400004-B5A3-F393-E0A9-E50E24DCCA9E(0x0A)  BaudRate
  19     0x2901  info
************************************************************************************/


typedef struct ble_character16 {
  uint16_t type16;          //type2
  uint16_t handle_rec;      //handle
  uint8_t  characterInfo[5];//property1 - handle2 - uuid2
  uint8_t  uuid128_idx;     //0xff means uuid16,other is idx of uuid128
} BLE_CHAR;

typedef struct ble_UUID128 {
  uint8_t  uuid128[16];//uuid128 string: little endian
} BLE_UUID128;

//
// STEP 0: Character declare
//
const BLE_CHAR AttCharList[] = {
  // =====  GATT =====
  {TYPE_CHAR, 0x0003, {ATT_CHAR_PROP_RD, 0x04, 0, 0x00, 0x2a}, UUID16_FORMAT}, //name
  //05-06 reserved

  // ===== Device Information Service Characteristics =====
  {TYPE_CHAR, 0x0008, {ATT_CHAR_PROP_RD, 0x09, 0, 0x29, 0x2a}, UUID16_FORMAT}, //manufacture
  {TYPE_CHAR, 0x000a, {ATT_CHAR_PROP_RD, 0x0b, 0, 0x26, 0x2a}, UUID16_FORMAT}, //firmware version
  {TYPE_CHAR, 0x000e, {ATT_CHAR_PROP_RD, 0x0f, 0, 0x28, 0x2a}, UUID16_FORMAT}, //sw version

  // ===== Item Service Characteristics =====
  {TYPE_CHAR, 0x20, {ATT_CHAR_PROP_RD | ATT_CHAR_PROP_W | ATT_CHAR_PROP_W_NORSP | ATT_CHAR_PROP_NTF, 0x21, 0, 0}, 1}, // Characteristic
  {TYPE_INFO, 0x22},                                                                              // Characteristic User Description
  {TYPE_CFG, 0x23, {ATT_CHAR_PROP_RD|ATT_CHAR_PROP_W}},                                           // Client Characteristic Configuration
  {TYPE_PRESENTATION_FORMAT, 0x24},                                                               // Characteristic Pre-sentation Format
};

// List of 128-bit UUIDs
const BLE_UUID128 AttUuid128List[] = {
  {0x8f, 0x2e, 0x78, 0x51, 0xba, 0xfb, 0x95, 0xb9, 0xca, 0x41, 0x28, 0x3f, 0x00, 0x00, 0xe2, 0xcc},
  {0x8f, 0x2e, 0x78, 0x51, 0xba, 0xfb, 0x95, 0xb9, 0xca, 0x41, 0x28, 0x3f, 0x01, 0x00, 0xe2, 0xcc},
};

uint8_t GetCharListDim(void)
{
  return sizeof(AttCharList) / sizeof(AttCharList[0]);
}


//
// STEP 1: Service declare
//
void att_server_rdByGrType( uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd, uint16_t att_type )
{
  // GAP and GATT (start handle 0x01)
  if ((att_type == GATT_PRIMARY_SERVICE_UUID) && (st_hd == 1)) //hard code for device info service
  {
    //GAP Device Name
    uint8_t t[] = {0x00, 0x18};
    att_server_rdByGrTypeRspPrimaryService(pdu_type, 0x1, 0x6, (uint8_t*)(t), 2);
    return;
  }
  // Device Info Service (start handle 0x07)
  else if ((att_type == GATT_PRIMARY_SERVICE_UUID) && (st_hd <= 0x07))
  {
    uint8_t t[] = {0xa, 0x18};
    att_server_rdByGrTypeRspPrimaryService(pdu_type, 0x7, 0xf, (uint8_t*)(t), 2);
    return;
  }
  // Item Service (start handle 0x20)
  else if ((att_type == GATT_PRIMARY_SERVICE_UUID) && (st_hd <= 0x20))
  {
    att_server_rdByGrTypeRspPrimaryService(pdu_type, 0x20, 0x24, (uint8_t*)(AttUuid128List[0].uuid128), 16);
    return;
  }

  ///error handle
  att_notFd( pdu_type, attOpcode, st_hd );
}


//
// STEP 2: data coming
// write response
void ser_write_rsp(uint8_t pdu_type/*reserved*/, uint8_t attOpcode/*reserved*/,
                   uint16_t att_hd, uint8_t* attValue/*app data pointer*/, uint8_t valueLen_w/*app data size*/)
{
  switch (att_hd)
  {
    case 0x21:
      // copy the first byte of written data into the item characteristic value
      memcpy(&itemCharacteristicValue, attValue, 1);
      bItemCharacteristicValueChanged = true;
      ser_write_rsp_pkt(pdu_type);  /*if the related character has the property of WRITE(with response) or TYPE_CFG, one MUST invoke this func*/
      break;
    case 0x23:
      // write to the Client Characteristic Configuration Descriptor
      memcpy(&cccd, attValue, 2);
      ser_write_rsp_pkt(pdu_type);
    default:
      att_notFd(pdu_type, attOpcode, att_hd );  /*the default response, also for the purpose of error robust */
      break;
  }
}


//
// STEP 3: Read data
// read response
void server_rd_rsp(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type)
{
  uint8_t  d_len;
  uint8_t* ble_name = getDeviceInfoData(&d_len);

  switch (attHandle) // hard code
  {
    case 0x04: // GAP name
      att_server_rd( pdu_type, attOpcode, attHandle, ble_name, d_len);
      break;

    case 0x09: // MANU_INFO
      att_server_rd( pdu_type, attOpcode, attHandle, (uint8_t*)(MANU_INFO), sizeof(MANU_INFO) - 1);
      break;

    case 0x0b: // FIRMWARE_INFO
      att_server_rd(pdu_type, attOpcode, attHandle, GetFirmwareInfo(), strlen((const char*)GetFirmwareInfo()));
      break;

    case 0x0f:  // SOFTWARE_INFO
      att_server_rd(pdu_type, attOpcode, attHandle, (uint8_t*)(SOFTWARE_INFO), sizeof(SOFTWARE_INFO) - 1);
      break;

    case 0x21:  // Item Characteristic Value
      att_server_rd(pdu_type, attOpcode, attHandle, &itemCharacteristicValue, 1);
      break;

    case 0x22:  // Item Characteristic User Description
      att_server_rd(pdu_type, attOpcode, attHandle, (uint8_t*) itemCharacteristicUserDescription, sizeof(itemCharacteristicUserDescription));
      break;

    case 0x23:  // Item Client Characteristic Configuration
      {
        att_server_rd(pdu_type, attOpcode, attHandle, cccd, 2);
      }
      break;

    case 0x24: // Item Characteristic Presentation Format
      att_server_rd(pdu_type, attOpcode, attHandle, (uint8_t*)itemCharacteristicPresentationFormat, sizeof(itemCharacteristicPresentationFormat));
      break;

    default:
      att_notFd( pdu_type, attOpcode, attHandle );/*the default response, also for the purpose of error robust */
      break;
  }
}


void ser_prepare_write(unsigned short handle, unsigned char* attValue, unsigned short attValueLen, unsigned short att_offset) {

}


void ser_execute_write(void) {

}


void server_blob_rd_rsp(uint8_t attOpcode, uint16_t attHandle, uint8_t dataHdrP, uint16_t offset)
{
  
}

//return 1 means found
int GetPrimaryServiceHandle(unsigned short hd_start, unsigned short hd_end,
                            unsigned short uuid16,
                            unsigned short* hd_start_r, unsigned short* hd_end_r)
{
  return 0;
}


void gatt_user_send_notify_data_callback(void)
{

}


uint8_t* getsoftwareversion(void)
{
  return (uint8_t*)SOFTWARE_INFO;
}


static unsigned char gConnectedFlag = 0;

char GetConnectedStatus(void)
{
  return gConnectedFlag;
}


void ConnectStausUpdate(unsigned char IsConnectedFlag) //porting api
{
  LED_ONOFF(!IsConnectedFlag);

  if (IsConnectedFlag != gConnectedFlag)
  {
    gConnectedFlag = IsConnectedFlag;
  }
}


void UsrProcCallback(void) //porting api
{
  return;
}


unsigned char aes_encrypt_HW(unsigned char *_data, unsigned char *_key)
{
  return 0;
}
