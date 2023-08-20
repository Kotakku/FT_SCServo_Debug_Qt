#pragma once

#include "servo/scserial.h"

namespace feetech_servo
{

//-------EPROM(只读)--------
#define SMS_STS_MODEL_L 3
#define SMS_STS_MODEL_H 4

//-------EPROM(读写)--------
#define SMS_STS_ID 5
#define SMS_STS_BAUD_RATE 6
#define SMS_STS_MIN_ANGLE_LIMIT_L 9
#define SMS_STS_MIN_ANGLE_LIMIT_H 10
#define SMS_STS_MAX_ANGLE_LIMIT_L 11
#define SMS_STS_MAX_ANGLE_LIMIT_H 12
#define SMS_STS_CW_DEAD 26
#define SMS_STS_CCW_DEAD 27
#define SMS_STS_OFS_L 31
#define SMS_STS_OFS_H 32
#define SMS_STS_MODE 33

//-------SRAM(读写)--------
#define SMS_STS_TORQUE_ENABLE 40
#define SMS_STS_ACC 41
#define SMS_STS_GOAL_POSITION_L 42
#define SMS_STS_GOAL_POSITION_H 43
#define SMS_STS_GOAL_TIME_L 44
#define SMS_STS_GOAL_TIME_H 45
#define SMS_STS_GOAL_SPEED_L 46
#define SMS_STS_GOAL_SPEED_H 47
#define SMS_STS_TORQUE_LIMIT_L 48
#define SMS_STS_TORQUE_LIMIT_H 49
#define SMS_STS_LOCK 55

//-------SRAM(只读)--------
#define SMS_STS_PRESENT_POSITION_L 56
#define SMS_STS_PRESENT_POSITION_H 57
#define SMS_STS_PRESENT_SPEED_L 58
#define SMS_STS_PRESENT_SPEED_H 59
#define SMS_STS_PRESENT_LOAD_L 60
#define SMS_STS_PRESENT_LOAD_H 61
#define SMS_STS_PRESENT_VOLTAGE 62
#define SMS_STS_PRESENT_TEMPERATURE 63
#define SMS_STS_MOVING 66
#define SMS_STS_PRESENT_CURRENT_L 69
#define SMS_STS_PRESENT_CURRENT_H 70

class SMS_STS
{
public:
    SMS_STS(SCSerial *scserial_)
        : scserial_(scserial_)
    {
        // scserial_->set_end(0);
    }

	int write_pos_ex(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t ACC = 0)//普通写单个舵机位置指令
    {
        if(Position<0){
            Position = -Position;
            Position |= (1<<15);
        }
        uint8_t bBuf[7];
        bBuf[0] = ACC;
        scserial_->host_2_scs(bBuf+1, bBuf+2, Position);
        scserial_->host_2_scs(bBuf+3, bBuf+4, 0);
        scserial_->host_2_scs(bBuf+5, bBuf+6, Speed);
        
        return scserial_->gen_write(ID, SMS_STS_ACC, bBuf, 7);
    }

	int reg_write_pos_ex(uint8_t ID, int16_t Position, uint16_t Speed, uint8_t ACC = 0)//异步写单个舵机位置指令(RegWriteAction生效)
    {
        if(Position<0){
            Position = -Position;
            Position |= (1<<15);
        }
        uint8_t bBuf[7];
        bBuf[0] = ACC;
        scserial_->host_2_scs(bBuf+1, bBuf+2, Position);
        scserial_->host_2_scs(bBuf+3, bBuf+4, 0);
        scserial_->host_2_scs(bBuf+5, bBuf+6, Speed);
        
        return scserial_->reg_write(ID, SMS_STS_ACC, bBuf, 7);
    }

	void sync_write_pos_ex(uint8_t ID[], uint8_t IDN, int16_t Position[], uint16_t Speed[], uint8_t ACC[])//同步写多个舵机位置指令
    {
        uint8_t offbuf[7*IDN];
        for(uint8_t i = 0; i<IDN; i++){
            if(Position[i]<0){
                Position[i] = -Position[i];
                Position[i] |= (1<<15);
            }
            uint16_t V;
            if(Speed){
                V = Speed[i];
            }else{
                V = 0;
            }
            if(ACC){
                offbuf[i*7] = ACC[i];
            }else{
                offbuf[i*7] = 0;
            }
            scserial_->host_2_scs(offbuf+i*7+1, offbuf+i*7+2, Position[i]);
            scserial_->host_2_scs(offbuf+i*7+3, offbuf+i*7+4, 0);
            scserial_->host_2_scs(offbuf+i*7+5, offbuf+i*7+6, V);
        }
        scserial_->sync_write(ID, IDN, SMS_STS_ACC, offbuf, 7);
    }

	int rotation_mode(uint8_t ID)
    {
        return scserial_->write_byte(ID, SMS_STS_MODE, 0);
    }

	int wheel_mode(uint8_t ID)//恒速模式
    {
        return scserial_->write_byte(ID, SMS_STS_MODE, 1);
    }

	int open_loop_wheel_mode(uint8_t ID)
    {
        return scserial_->write_byte(ID, SMS_STS_MODE, 2);
    }
    
	int write_speed(uint8_t ID, int16_t Speed, uint8_t ACC = 0)//恒速模式控制指令
    {
        if(Speed<0){
            Speed = -Speed;
            Speed |= (1<<15);
        }
        uint8_t bBuf[2];
        bBuf[0] = ACC;
        scserial_->gen_write(ID, SMS_STS_ACC, bBuf, 1);
        scserial_->host_2_scs(bBuf+0, bBuf+1, Speed);
        
        return scserial_->gen_write(ID, SMS_STS_GOAL_SPEED_L, bBuf, 2);
    }

	int enable_torque(uint8_t ID, uint8_t Enable)//扭力控制指令
    {
        return scserial_->write_byte(ID, SMS_STS_TORQUE_ENABLE, Enable);
    }

	int unlock_eprom(uint8_t ID)//eprom解锁
    {
        return scserial_->write_byte(ID, SMS_STS_LOCK, 0);
    }

	int lock_eprom(uint8_t ID)//eprom加锁
    {
        return scserial_->write_byte(ID, SMS_STS_LOCK, 1);
    }

	int calibration_offset(uint8_t ID)//中位校准
    {
        return scserial_->write_byte(ID, SMS_STS_TORQUE_ENABLE, 128);
    }

	int feedback(int ID)//反馈舵机信息
    {
        int nLen = scserial_->read(ID, SMS_STS_PRESENT_POSITION_L, Mem, sizeof(Mem));
        if(nLen!=sizeof(Mem)){
            err_ = 1;
            return -1;
        }
        err_ = 0;
        return nLen;
    }

	int read_position(int ID)//读位置
    {
        int Pos = -1;
        if(ID==-1){
            Pos = Mem[SMS_STS_PRESENT_POSITION_H-SMS_STS_PRESENT_POSITION_L];
            Pos <<= 8;
            Pos |= Mem[SMS_STS_PRESENT_POSITION_L-SMS_STS_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Pos = scserial_->read_word(ID, SMS_STS_PRESENT_POSITION_L);
            if(Pos==-1){
                err_ = 1;
            }
        }
        if(!err_ && (Pos&(1<<15))){
            Pos = -(Pos&~(1<<15));
        }
        
        return Pos;
    }
	
    int read_speed(int ID)//读速度
    {
        int Speed = -1;
        if(ID==-1){
            Speed = Mem[SMS_STS_PRESENT_SPEED_H-SMS_STS_PRESENT_POSITION_L];
            Speed <<= 8;
            Speed |= Mem[SMS_STS_PRESENT_SPEED_L-SMS_STS_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Speed = scserial_->read_word(ID, SMS_STS_PRESENT_SPEED_L);
            if(Speed==-1){
                err_ = 1;
                return -1;
            }
        }
        if(!err_ && (Speed&(1<<15))){
            Speed = -(Speed&~(1<<15));
        }	
        return Speed;
    }

	int read_load(int ID)//读输出至电机的电压百分比(0~1000)
    {
        int Load = -1;
        if(ID==-1){
            Load = Mem[SMS_STS_PRESENT_LOAD_H-SMS_STS_PRESENT_POSITION_L];
            Load <<= 8;
            Load |= Mem[SMS_STS_PRESENT_LOAD_L-SMS_STS_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Load = scserial_->read_word(ID, SMS_STS_PRESENT_LOAD_L);
            if(Load==-1){
                err_ = 1;
            }
        }
        if(!err_ && (Load&(1<<10))){
            Load = -(Load&~(1<<10));
        }
        return Load;
    }

	int read_voltage(int ID)//读电压
    {
        int Voltage = -1;
        if(ID==-1){
            Voltage = Mem[SMS_STS_PRESENT_VOLTAGE-SMS_STS_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Voltage = scserial_->read_byte(ID, SMS_STS_PRESENT_VOLTAGE);
            if(Voltage==-1){
                err_ = 1;
            }
        }
        return Voltage;
    }

    double read_voltage_double(int ID)
    {
        return read_voltage(ID)/10.0;
    }

	int read_temperature(int ID)//读温度
    {
        int Temper = -1;
        if(ID==-1){
            Temper = Mem[SMS_STS_PRESENT_TEMPERATURE-SMS_STS_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Temper = scserial_->read_byte(ID, SMS_STS_PRESENT_TEMPERATURE);
            if(Temper==-1){
                err_ = 1;
            }
        }
        return Temper;
    }

	int read_move(int ID)//读移动状态
    {
        int Move = -1;
        if(ID==-1){
            Move = Mem[SMS_STS_MOVING-SMS_STS_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Move = scserial_->read_byte(ID, SMS_STS_MOVING);
            if(Move==-1){
                err_ = 1;
            }
        }
        return Move;
    }

	int read_current(int ID)//读电流
    {
        int Current = -1;
        if(ID==-1){
            Current = Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L];
            Current <<= 8;
            Current |= Mem[SMS_STS_PRESENT_CURRENT_L-SMS_STS_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Current = scserial_->read_word(ID, SMS_STS_PRESENT_CURRENT_L);
            if(Current==-1){
                err_ = 1;
                return -1;
            }
        }
        if(!err_ && (Current&(1<<15))){
            Current = -(Current&~(1<<15));
        }	
        return Current;
    }

	int read_goal(int ID)//读电流
    {
        int Goal = -1;
        {
            err_ = 0;
            Goal = scserial_->read_word(ID, SMS_STS_GOAL_POSITION_L);
            if(Goal==-1){
                err_ = 1;
                return -1;
            }
        }
        if(!err_ && (Goal&(1<<15))){
            Goal = -(Goal&~(1<<15));
        }	
        return Goal;
    }

private:
    int err_ = 0;
    SCSerial *scserial_;
    uint8_t Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L+1];
};

}
