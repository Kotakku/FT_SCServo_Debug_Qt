#pragma once

#include "servo/scserial.h"

namespace feetech_servo
{

//-------EPROM(只读)--------
#define SCSCL_VERSION_L 3
#define SCSCL_VERSION_H 4

//-------EPROM(读写)--------
#define SCSCL_ID 5
#define SCSCL_BAUD_RATE 6
#define SCSCL_MIN_ANGLE_LIMIT_L 9
#define SCSCL_MIN_ANGLE_LIMIT_H 10
#define SCSCL_MAX_ANGLE_LIMIT_L 11
#define SCSCL_MAX_ANGLE_LIMIT_H 12
#define SCSCL_CW_DEAD 26
#define SCSCL_CCW_DEAD 27

//-------SRAM(读写)--------
#define SCSCL_TORQUE_ENABLE 40
#define SCSCL_GOAL_POSITION_L 42
#define SCSCL_GOAL_POSITION_H 43
#define SCSCL_GOAL_TIME_L 44
#define SCSCL_GOAL_TIME_H 45
#define SCSCL_GOAL_SPEED_L 46
#define SCSCL_GOAL_SPEED_H 47
#define SCSCL_LOCK 48

//-------SRAM(只读)--------
#define SCSCL_PRESENT_POSITION_L 56
#define SCSCL_PRESENT_POSITION_H 57
#define SCSCL_PRESENT_SPEED_L 58
#define SCSCL_PRESENT_SPEED_H 59
#define SCSCL_PRESENT_LOAD_L 60
#define SCSCL_PRESENT_LOAD_H 61
#define SCSCL_PRESENT_VOLTAGE 62
#define SCSCL_PRESENT_TEMPERATURE 63
#define SCSCL_MOVING 66
#define SCSCL_PRESENT_CURRENT_L 69
#define SCSCL_PRESENT_CURRENT_H 70

class SCSCL
{
public:
    SCSCL(SCSerial *scserial_)
        : scserial_(scserial_)
    {
        // scserial_->set_end(1);
    }

	int write_pos(uint8_t ID, uint16_t Position, uint16_t Time, uint16_t Speed = 0)//普通写单个舵机位置指令
    {
        uint8_t bBuf[6];
        scserial_->host_2_scs(bBuf+0, bBuf+1, Position);
        scserial_->host_2_scs(bBuf+2, bBuf+3, Time);
        scserial_->host_2_scs(bBuf+4, bBuf+5, Speed);
        
        return scserial_->gen_write(ID, SCSCL_GOAL_POSITION_L, bBuf, 6);
    }

	int reg_write_pos(uint8_t ID, uint16_t Position, uint16_t Time, uint16_t Speed = 0)//异步写单个舵机位置指令(RegWriteAction生效)
    {
        uint8_t bBuf[6];
        scserial_->host_2_scs(bBuf+0, bBuf+1, Position);
        scserial_->host_2_scs(bBuf+2, bBuf+3, Time);
        scserial_->host_2_scs(bBuf+4, bBuf+5, Speed);
        
        return scserial_->reg_write(ID, SCSCL_GOAL_POSITION_L, bBuf, 6);
    }

	void sync_write_pos(uint8_t ID[], uint8_t IDN, uint16_t Position[], uint16_t Time[], uint16_t Speed[])//同步写多个舵机位置指令
    {
        uint8_t offbuf[6*IDN];
        for(uint8_t i = 0; i<IDN; i++){
            uint16_t T, V;
            if(Time){
                T = Time[i];
            }else{
                T = 0;
            }
            if(Speed){
                V = Speed[i];
            }else{
                V = 0;
            }
            scserial_->host_2_scs(offbuf+i*6+0, offbuf+i*6+1, Position[i]);
            scserial_->host_2_scs(offbuf+i*6+2, offbuf+i*6+3, T);
            scserial_->host_2_scs(offbuf+i*6+4, offbuf+i*6+5, V);
        }
        scserial_->sync_write(ID, IDN, SCSCL_GOAL_POSITION_L, offbuf, 6);
    }

	int pwm_mode(uint8_t ID)//PWM输出模式
    {
        uint8_t bBuf[4];
        bBuf[0] = 0;
        bBuf[1] = 0;
        bBuf[2] = 0;
        bBuf[3] = 0;
        return scserial_->gen_write(ID, SCSCL_MIN_ANGLE_LIMIT_L, bBuf, 4);	
    }

	int write_pwm(uint8_t ID, int16_t pwmOut)//PWM输出模式指令
    {
        if(pwmOut<0){
            pwmOut = -pwmOut;
            pwmOut |= (1<<10);
        }
        uint8_t bBuf[2];
        scserial_->host_2_scs(bBuf+0, bBuf+1, pwmOut);
        
        return scserial_->gen_write(ID, SCSCL_GOAL_TIME_L, bBuf, 2);
    }

	int enable_torque(uint8_t ID, uint8_t Enable)//扭矩控制指令
    {
        return scserial_->write_byte(ID, SCSCL_TORQUE_ENABLE, Enable);
    }

	int unlock_eprom(uint8_t ID)//
    {
        return scserial_->write_byte(ID, SCSCL_LOCK, 0);
    }

	int lock_eprom(uint8_t ID)//eprom加锁
    {
        return scserial_->write_byte(ID, SCSCL_LOCK, 1);
    }

	int feedback(int ID)//反馈舵机信息
    {
        int nLen = scserial_->read(ID, SCSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
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
            Pos = Mem[SCSCL_PRESENT_POSITION_L-SCSCL_PRESENT_POSITION_L];
            Pos <<= 8;
            Pos |= Mem[SCSCL_PRESENT_POSITION_H-SCSCL_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Pos = scserial_->read_word(ID, SCSCL_PRESENT_POSITION_L);
            if(Pos==-1){
                err_ = 1;
            }
        }
        return Pos;
    }

	int read_speed(int ID)//读速度
    {
        int Speed = -1;
        if(ID==-1){
            Speed = Mem[SCSCL_PRESENT_SPEED_L-SCSCL_PRESENT_POSITION_L];
            Speed <<= 8;
            Speed |= Mem[SCSCL_PRESENT_SPEED_H-SCSCL_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Speed = scserial_->read_word(ID, SCSCL_PRESENT_SPEED_L);
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
            Load = Mem[SCSCL_PRESENT_LOAD_L-SCSCL_PRESENT_POSITION_L];
            Load <<= 8;
            Load |= Mem[SCSCL_PRESENT_LOAD_H-SCSCL_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Load = scserial_->read_word(ID, SCSCL_PRESENT_LOAD_L);
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
            Voltage = Mem[SCSCL_PRESENT_VOLTAGE-SCSCL_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Voltage = scserial_->read_byte(ID, SCSCL_PRESENT_VOLTAGE);
            if(Voltage==-1){
                err_ = 1;
            }
        }
        return Voltage;
    }

	int read_temperature(int ID)//读温度
    {
        int Temper = -1;
        if(ID==-1){
            Temper = Mem[SCSCL_PRESENT_TEMPERATURE-SCSCL_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Temper = scserial_->read_byte(ID, SCSCL_PRESENT_TEMPERATURE);
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
            Move = Mem[SCSCL_MOVING-SCSCL_PRESENT_POSITION_L];	
        }else{
            err_ = 0;
            Move = scserial_->read_byte(ID, SCSCL_MOVING);
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
            Current = Mem[SCSCL_PRESENT_CURRENT_L-SCSCL_PRESENT_POSITION_L];
            Current <<= 8;
            Current |= Mem[SCSCL_PRESENT_CURRENT_H-SCSCL_PRESENT_POSITION_L];
        }else{
            err_ = 0;
            Current = scserial_->read_word(ID, SCSCL_PRESENT_CURRENT_L);
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
            Goal = scserial_->read_word(ID, SCSCL_GOAL_POSITION_L);
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
	uint8_t Mem[SCSCL_PRESENT_CURRENT_H-SCSCL_PRESENT_POSITION_L+1];
};

}
