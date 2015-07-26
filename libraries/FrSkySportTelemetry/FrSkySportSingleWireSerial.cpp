/*
  FrSky single wire serial class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20150319
  Not for commercial use
*/

#include "FrSkySportSingleWireSerial.h"

#define FRSKY_SENSOR_DATA_FRAME 0x10
#define FRSKY_STUFFING 0x7D

FrSkySportSingleWireSerial::FrSkySportSingleWireSerial()
{
  uartC3 = NULL;
  port = NULL;
  softSerial = NULL;
}

void FrSkySportSingleWireSerial::begin(SerialId id)
{
  /*if(softSerial != NULL)
  {
    delete softSerial;
    softSerial = NULL;
  }*/
  if(softSerial == NULL)
  {
  softSerialId = id;
  softSerial = new SoftwareSerial(softSerialId, softSerialId, true);
  port = softSerial;
  }
  softSerial->begin(57600);
  crc = 0;
}

void FrSkySportSingleWireSerial::stop()
{
  if(softSerial != NULL)
  {
    softSerial->end();
    
  }
}

void FrSkySportSingleWireSerial::listen()
{
  if(softSerial != NULL)
  {
    softSerial->listen();
    
  }
}

void FrSkySportSingleWireSerial::setMode(SerialMode mode)
{
  if(port != NULL)
  {
    if(mode == TX)
    {
        pinMode(softSerialId, OUTPUT);
    }
    else if(mode == RX)
    {
        pinMode(softSerialId, INPUT);
    }
  }
}

void FrSkySportSingleWireSerial::sendByte(uint8_t byte)
{
  if(port != NULL)
  {
    if(byte == 0x7E)
    {
      port->write(FRSKY_STUFFING);
      port->write(0x5E); // 0x7E xor 0x20
    }
    else if(byte == 0x7D)
    {
      port->write(FRSKY_STUFFING);
      port->write(0x5D); // 0x7D xor 0x20
    }
    else
    {
      port->write(byte);
    }
    crc += byte;
    crc += crc >> 8; crc &= 0x00ff;
    crc += crc >> 8; crc &= 0x00ff;
  }
}

void FrSkySportSingleWireSerial::sendCrc()
{
  // Send and reset CRC
  if(port != NULL)
  {
    port->write(0xFF - crc);
    crc = 0;
  }
}

void FrSkySportSingleWireSerial::sendData(uint16_t dataTypeId, uint32_t data)
{
  if(port != NULL)
  {
    setMode(TX);
    sendByte(FRSKY_SENSOR_DATA_FRAME);
    uint8_t *bytes = (uint8_t*)&dataTypeId;
    sendByte(bytes[0]);
    sendByte(bytes[1]);
    bytes = (uint8_t*)&data;
    sendByte(bytes[0]);
    sendByte(bytes[1]);
    sendByte(bytes[2]);
    sendByte(bytes[3]);
    sendCrc();
    port->flush();
    setMode(RX);
  }
}
