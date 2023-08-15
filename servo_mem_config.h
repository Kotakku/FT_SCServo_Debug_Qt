#ifndef SERVO_MEM_CONFIG_H
#define SERVO_MEM_CONFIG_H

#include <map>

namespace feetech_servo
{

struct MemoryConfig
{
    uint8_t address;
    QString name;
    uint8_t size;
    uint16_t default_value;
    bool is_eprom;
    bool is_readonly;
    int16_t min_val;
    int16_t max_val;
};

const std::vector<MemoryConfig> STSMemConfig =
{
    {0, "Firmare Main Version NO.", 1, 0, true, true, -1, -1},
    {1, "Firmware Secondary Version NO.", 1, 0, true, true, -1, -1},
    {3, "Servo Main Version", 1, 0, true, true, -1, -1},
    {4, "Servo Sub Version", 1, 0, true, true, -1, -1},
    {5, "ID", 1, 0, true, false, 0, 253},
    {6, "Baud Rate", 1, 4, true, false, 0, 7},
    {7, "Return Delay Time", 1, 250, true, false, 0, 254},
    {8, "Status Return Level", 1, 1, true, false, 0, 1},
    {9, "Min Position Limit", 2, 0, true, false, -1, -1},
    {11, "Max Position Limit", 2, 0, true, false, -1, -1},
    {13, "Max Temperature limit", 1, 80, true, false, 0, 100},
    {14, "Max Input Voltage", 1, 140, true, false, 0, 254},
    {15, "Min Input Voltage", 1, 80, true, false, 0, 254},
    {16, "Max Torque Limit", 2, 1000, true, false, 0, 1000},
    {18, "Setting Byte", 1, 0, true, false, 0, 254},
    {19, "Protection Switch", 1, 37, true, false, 0, 254},
    {20, "LED Alarm Condition", 1, 37, true, false, 0, 254},
    {21, "Position P Gain", 1, 32, true, false, 0, 254},
    {22, "Position D Gain", 1, 0, true, false, 0, 254},
    {23, "Position I Gain", 1, 0, true, false, 0, 254},
    {24, "Punch", 2, 0, true, false, 0, 1000},
    {26, "CW Dead Band", 1, 0, true, false, 0, 32},
    {27, "CCW Dead Band", 1, 0, true, false, 0, 32},
    {28, "Overload Current", 2, 0, true, false, 0, 511},
    {30, "Angular Resolution", 1, 1, true, false, 1, 100},
    {31, "Position Offset Value", 2, 0, true, false, -2047, 2047},
    {33, "Work Mode", 1, 0, true, false, 0, 3},
    {34, "Protect Torque", 1, 40, true, false, 0, 254},
    {35, "Overload Protection Time", 1, 80, true, false, 0, 254},
    {36, "Overload Torque", 1, 80, true, false, 0, 254},
    {37, "Velocity P Gain", 1, 32, true, false, 0, 254},
    {38, "Overcurrent Protection Time", 1, 100, true, false, 0, 254},
    {39, "Velocity I Gain", 1, 0, true, false, 0, 254},
    {40, "Torque Enable", 1, 0, false, false, 0, 254},
    {41, "Goal Acceleration", 1, 0, false, false, 0, 254},
    {42, "Goal Position", 2, 0, false, false, -32766, 32766},
    {46, "Goal Velocity", 2, 0, false, false, -1000, 1000},
    {48, "Torque Limit", 2, 1000, false, false, 0, 1000},
    {55, "Lock", 1, 1, false, false, 0, 1},
    {56, "Present Position", 2, 0, false, true, -1, -1},
    {58, "Present Velocity", 2, 0, false, true, -1, -1},
    {60, "Present PWM", 2, 0, false, true, -1, -1},
    {62, "Present Input Voltage", 1, 0, false, true, -1, -1},
    {63, "Present Temperature", 1, 0, false, true, -1, -1},
    {64, "Sync Write Flag", 1, 0, false, true, -1, -1},
    {65, "Hardware Error Status", 1, 0, false, true, -1, -1},
    {66, "Moving Status", 1, 0, false, true, -1, -1},
    {69, "Present Current", 2, 0, false, true, -1, -1},
};

}

#endif // SERVO_MEM_CONFIG_H
