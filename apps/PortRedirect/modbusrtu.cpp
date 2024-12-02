#include "modbusrtu.h"
#include "hbox.h"
#include <map>
#include <mutex>

typedef struct
{
    std::map<modbus_data_address_t,bool> coils;
    std::map<modbus_data_address_t,bool> discrete_inputs;
    std::map<modbus_data_address_t,modbus_data_register_t> registers;
    std::map<modbus_data_address_t,modbus_data_register_t> input_registers;
} modbus_data_t;
static std::map<modbus_rtu_slave_tiny_context_t*,modbus_data_t> mb_data;
static std::recursive_mutex mb_data_lock;
static bool    read_coil(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        if(mb_data[ctx].coils.find(addr)!=mb_data[ctx].coils.end())
        {
            return mb_data[ctx].coils[addr];
        }
    }
    return false;
}
static bool    read_discrete_input(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        if(mb_data[ctx].discrete_inputs.find(addr)!=mb_data[ctx].discrete_inputs.end())
        {
            return mb_data[ctx].discrete_inputs[addr];
        }
    }
    return !read_coil(ctx,addr);
}
static modbus_data_register_t  read_holding_register(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        if(mb_data[ctx].registers.find(addr)!=mb_data[ctx].registers.end())
        {
            return mb_data[ctx].registers[addr];
        }
    }
    return 0xDEAD;
}
static modbus_data_register_t  read_input_register(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        if(mb_data[ctx].input_registers.find(addr)!=mb_data[ctx].input_registers.end())
        {
            return mb_data[ctx].input_registers[addr];
        }
    }
    return ~ read_holding_register(ctx, addr);
}
static void    write_coil(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr,bool value)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        mb_data[ctx].coils[addr]=value;
    }
}
static void    write_holding_register(modbus_rtu_slave_tiny_context_t* ctx,modbus_data_address_t addr,modbus_data_register_t value)
{
    std::lock_guard<std::recursive_mutex> lock(mb_data_lock);
    if(mb_data.find(ctx)!=mb_data.end())
    {
        mb_data[ctx].registers[addr]=value;
    }
}


void modbus_init_ctx(modbus_rtu_slave_tiny_context_t* ctx)
{
    if(ctx==NULL)
    {
        return;
    }
    (*ctx)=modbus_rtu_slave_tiny_context_default();
    mb_data[ctx]=modbus_data_t();
    ctx->read_coil=read_coil;
    ctx->read_discrete_input=read_discrete_input;
    ctx->read_holding_register=read_holding_register;
    ctx->read_input_register=read_input_register;
    ctx->write_coil=write_coil;
    ctx->write_holding_register=write_holding_register;
}
