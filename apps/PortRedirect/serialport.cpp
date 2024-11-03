#include "serialport.h"
#include "stdlib.h"

SerialPort::SerialPort():port(NULL)
{

}

bool SerialPort::Open(const char *portname,int baudrate,int databits,char parity,int stopbits)
{
    if(IsOpen())
    {
        return true;
    }
    if(SP_OK!=sp_get_port_by_name(portname,&port))
    {
        return false;
    }
    if(SP_OK!=sp_open(port,SP_MODE_READ_WRITE))
    {
        Close();
        return false;
    }
    if(SP_OK!=sp_set_baudrate(port,baudrate))
    {
        Close();
        return false;
    }
    if(SP_OK!=sp_set_bits(port,databits))
    {
        Close();
        return false;
    }
    {
        sp_parity _parity=SP_PARITY_NONE;
        switch(parity)
        {
        case 'N':
        {
            _parity=SP_PARITY_NONE;
        }
        break;
        case 'O':
        {
            _parity=SP_PARITY_ODD;
        }
        break;
        case 'E':
        {
            _parity=SP_PARITY_EVEN;
        }
        break;
        case 'M':
        {
            _parity=SP_PARITY_MARK;
        }
        break;
        case 'S':
        {
            _parity=SP_PARITY_SPACE;
        }
        break;
        default:
            break;
        }
        if(SP_OK!=sp_set_parity(port,_parity))
        {
            Close();
            return false;
        }
    }
    if(SP_OK!=sp_set_stopbits(port,stopbits))
    {
        Close();
        return false;
    }
    return IsOpen();
}
bool SerialPort::IsOpen()
{
    if(port!=NULL)
    {
        return true;
    }
    return false;
}
bool SerialPort::Close()
{
    if(IsOpen())
    {
        sp_close(port);
        sp_free_port(port);
        port=NULL;
    }
    return true;
}

size_t SerialPort::Read(void *buffer,size_t buffer_length)
{
    if(!IsOpen())
    {
        return 0;
    }
    int64_t ret=sp_blocking_read(port,buffer,buffer_length,500);
    if(ret>=0)
    {
        return ret;
    }
    else
    {
        Close();
        return 0;
    }
}
size_t SerialPort::Write(const void *buffer,size_t buffer_length)
{
    if(!IsOpen())
    {
        return 0;
    }
    int64_t ret=sp_blocking_write(port,buffer,buffer_length,30000);
    if(ret>=0)
    {
        return ret;
    }
    else
    {
        Close();
        return 0;
    }
}

SerialPort::~SerialPort()
{
    Close();
}
