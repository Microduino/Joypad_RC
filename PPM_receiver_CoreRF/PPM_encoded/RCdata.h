#include "arduino.h"
//======================
/*
[head,2byte,0xAA 0xBB] [type,1byte,0xC8] [data,16byte] [body,2byte,0x0D,0x0A]
 Example:
 AA BB C8 1A 01 1A 01 1A 01 2A 01 3A 01 4A 01 5A 01 6A 01 0D 0A
 */

//-------------------------
int16_t RC_data[8];
int type;

//=============================
int p;
int16_t read16(byte* _Buf) {
  byte _Buf1 = _Buf[p++];
  byte _Buf2 = _Buf[p++];
  int16_t r = (_Buf1 & 0xff) | ((_Buf2 << 8) & 0xff00);
  return r;
}

void read_data(int _num, byte* _buf)
{
  p = 0;
  _num = _num / 2;

  int16_t _bufin[_num];

  for (int i = 0; i < _num; i++)
  {
    _bufin[i] = read16(_buf);
  }

#ifdef _DEBUG
  DEBUG.println("\n\r");
#endif

#ifdef _DEBUG
  DEBUG.print("DATA_PPM: ");
#endif
  int16_t RC_data_c[16];
  for (int a = 0; a < _num; a++)
  {
#ifdef _DEBUG
    DEBUG.print(_bufin[a]);
    DEBUG.print(",");
#endif
    RC_data_c[a] = _bufin[a];
  }

  for (int a = 0; a < _num; a++)
  {
    RC_data[a] = RC_data_c[PPM_LINE[a]];
  }


#ifdef _DEBUG
  DEBUG.println(" ");
#endif
}

byte getChecksum(byte length, byte cmd, byte mydata[])
{
  //三个参数分别为： 数据长度  ，  指令代码  ，  实际数据数组
  byte checksum = 0;
  checksum ^= (length & 0xFF);
  checksum ^= (cmd & 0xFF);
  for (int i = 0; i < length; i++)
  {
    checksum ^= (mydata[i] & 0xFF);
  }
  return checksum;
}

byte inChar, inCache;
byte buffer[256];
unsigned long num = 0;

boolean sta = false;
boolean error = false;

void protocol()
{
  if (FROM.available() > 0) {
    inCache = inChar;
    inChar = FROM.read();

    buffer[num] = inChar;
#ifdef _DEBUG
    //DEBUG.print(inCache,HEX);
    //DEBUG.print(inChar,HEX);
#endif
    num++;
    delayMicroseconds(200);
  }

  if (sta) {
    sta = false;
    switch (inChar) {
      case 0xC8:
        error = false;
        type = 1;
#ifdef _DEBUG
        DEBUG.println("\n\rType 1 RAW: ");
#endif
        break;
      default:
        error = true;
        type = 0;
#ifdef _DEBUG
        DEBUG.println("\n\rType ERROR ");
#endif
    }
    num = 0;
  }

  if (inChar == 0xBB && inCache == 0xAA) {
    sta = true;
#ifdef _DEBUG
    DEBUG.println("\n\r");
    DEBUG.println("\n\r----START----");
#endif
  }

  if (num  == (CHANNEL_NUM * 2 + 1)) {
    inCache = buffer[CHANNEL_NUM * 2];
    buffer[CHANNEL_NUM * 2] = NULL;
    inChar = getChecksum(CHANNEL_NUM * 2, 200, buffer);

#ifdef _DEBUG
    DEBUG.print("NUM[");
    DEBUG.print(CHANNEL_NUM * 2);
    DEBUG.print("]:");
    for (long a = 0; a < num; a++) {
      DEBUG.print(buffer[a], HEX);
      DEBUG.print(" ");
    }
    DEBUG.println(" ");
#endif

    if (!error && inCache == inChar) {
#ifdef _DEBUG
      DEBUG.println("DATA OK");
#endif
      read_data(CHANNEL_NUM * 2, buffer);
#ifdef _DEBUG
      DEBUG.println("\n\rsendDATA READY");
#endif
      RC_write(RC_data);
      ///      delay(8);
    }
    else {
#ifdef _DEBUG
      DEBUG.println("DATA ERROR");
#endif
    }
#ifdef _DEBUG
    //DEBUG.print("\n\r");
    //DEBUG.println(num);
#endif
    num = 0;

#ifdef _DEBUG
    DEBUG.println("\n\r----END----");
#endif
  }
}
