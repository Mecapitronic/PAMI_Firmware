#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

namespace Hardware_Config
{
    enum class ServoPosition
    {
        VL53Min = 0,
        VL53Pos = 145,
        VL53Max = 290,

        BrasMin = 0,
        BrasPos = 145,
        BrasMax = 290,

        Min = 0,
        Max = 290
    };
    
    enum class ServoID
    {
        VL53 = 1,           // Servo pour bouger le VL53
        Bras = 2,           // Servo pour manger les noisettes
        BroadCast = 0xFE    // Broadcast ID pour communiquer avec tous les servos
    };
}
#endif