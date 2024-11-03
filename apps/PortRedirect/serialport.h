#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include "libserialport.h"
#include "stdint.h"
#include "stdlib.h"

class SerialPort
{
    struct sp_port *port;
public:
    SerialPort();
    bool Open(const char *portname,int baudrate=115200,int databits=8,char parity='N',int stopbits=1);
    bool IsOpen();
    bool Close();
    size_t Read(void *buffer,size_t buffer_length);
    size_t Write(const void *buffer,size_t buffer_length);
    virtual ~SerialPort();

protected:

private:
};

#endif // __SERIALPORT_H__
