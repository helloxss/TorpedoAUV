//
// ESC.cpp
// Library C++ code
// ----------------------------------
// Developed with embedXcode+
// http://embedXcode.weebly.com
//
// Details	    ST spin driver
// Project 		SMC
//
// Created by 	Alex Bondarenko, 2018-02-28 5:57 PM
// 				Alex Bondarenko
//
// Copyright 	(c) Alex Bondarenko, 2018
// Licence		<#licence#>
//
// See 			ESC.h and ReadMe.txt for references
//

#define CW 0
#define CCW 1
#define SPI_COMMS_DELAY_MICROSECONDS 10

#define MINIMUM_SPEED (500)

// Library header
#include "ESC.h"
#include "SPI.h"



ESC_Struct ESC[6];

unsigned int ESC_init(ESC_Struct* ESC_hande,int pin);
unsigned int ESC_init_all(void);
unsigned int ESCStart(ESC_Struct* ESC_hande);
unsigned int ESCStop(ESC_Struct* ESC_hande);
unsigned int ESCSetSpeed(ESC_Struct* ESC_hande,int16_t speed);
unsigned int ESCSetDirection(ESC_Struct* ESC_hande,uint8_t direction);
unsigned int ESCSetAcceleration(ESC_Struct* ESC_hande,int16_t acceleration);


unsigned int ESC_init_all()
{
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    ESC_init(&ESC[0],ESC_PIN_FL);
    ESC_init(&ESC[1],ESC_PIN_FR);
    ESC_init(&ESC[2],ESC_PIN_FC);
    ESC_init(&ESC[3],ESC_PIN_RL);
    ESC_init(&ESC[4],ESC_PIN_RR);
    ESC_init(&ESC[5],ESC_PIN_RC);
}

unsigned int ESC_init(ESC_Struct* ESC_hande,int pin)
{
    ESC_hande->pin = pin;
    ESC_hande->direction = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(ESC_hande->pin, HIGH);
    
    return 0;
}

unsigned int ESCStart(ESC_Struct* ESC_hande)
{
    
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_Start);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    return 0;
}

unsigned int ESCStop(ESC_Struct* ESC_hande)
{
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_Stop);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    return 0;
}

unsigned int ESCSetSpeed(ESC_Struct* ESC_hande,int16_t newSpeedSetPoint)
{
    uint8_t direction;
    if(newSpeedSetPoint>0)
    {
        direction = 1;
    }
    else
    {
        direction = 0;
    }
    if ((direction && newSpeedSetPoint>MINIMUM_SPEED) || (!direction && newSpeedSetPoint<(-1*MINIMUM_SPEED)))
    {
        if (direction !=ESC_hande->direction)
        {
            ESCSetDirection(ESC_hande, direction);
        }
        if(newSpeedSetPoint!=ESC_hande->speedSetPoint)
        {
            uint16_t newSpeed;
            if(direction)
            {
                newSpeed = newSpeedSetPoint;
            }
            else
            {
                newSpeed = - newSpeedSetPoint;
            }
            digitalWrite(ESC_hande->pin, LOW);
            SPI.transfer(ESC_CMD_SetSpeed);
            SPI.transfer(newSpeed>>8);
            SPI.transfer(newSpeed & 0xff);
            digitalWrite(ESC_hande->pin, HIGH);
            ESC_hande->speedSetPoint =newSpeedSetPoint;
        }
        if(ESCGetStatus(ESC_hande)==STOP)
        {
            ESCStart(ESC_hande);
        }
    }
    else
    {
        ESCStop(ESC_hande);
    }
    

    return 0;
}
uint16_t ESCGetSpeed(ESC_Struct* ESC_hande)
{
    uint16_t speed = 0;
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_GetSpeed);
    SPI.transfer(0);
    SPI.transfer(0);
    delayMicroseconds(SPI_COMMS_DELAY_MICROSECONDS);
    speed = SPI.transfer(0);
    speed = speed<<8;
    speed = speed | SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    ESC_hande->speedMeasured =speed;
    return speed;
}
uint16_t ESCGetCurrent(ESC_Struct* ESC_hande)
{
    uint16_t current = 0;
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_GetCurrent);
    SPI.transfer(0);
    SPI.transfer(0);
    delayMicroseconds(SPI_COMMS_DELAY_MICROSECONDS);
    current = SPI.transfer(0);
    current = current<<8;
    current = current | SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    ESC_hande->speedMeasured =current;
    return current;
}

unsigned int ESCSetDirection(ESC_Struct* ESC_hande,uint8_t direction)
{
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_SetDirection);
    SPI.transfer(direction);
    SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    ESC_hande->direction = direction;
    return 0;
}

unsigned int ESCSetAcceleration(ESC_Struct* ESC_hande,int16_t acceleration)
{
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_SetAcceleration);
    SPI.transfer(acceleration>>8);
    SPI.transfer(acceleration & 0xff);
    digitalWrite(ESC_hande->pin, HIGH);
    
    return 0;
}

unsigned int ESCGetStatus(ESC_Struct* ESC_hande )
{
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_GetRunState);
    SPI.transfer(0);
    SPI.transfer(0);
    delayMicroseconds(SPI_COMMS_DELAY_MICROSECONDS);
    int id = SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    
    return id;
}
ESC_StatusStruct ESCGetStatusStruct(ESC_Struct* ESC_hande )
{
    ESC_StatusStructUnion newData;
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_GetStatusStruct);
    SPI.transfer(0);
    SPI.transfer(0);
    delayMicroseconds(SPI_COMMS_DELAY_MICROSECONDS);
    for (int i = 0;i<sizeof(newData); i++)
    {
        newData.stuctRaw[i] = SPI.transfer(0);
    }
    digitalWrite(ESC_hande->pin, HIGH);
    
    return newData.statusStruct;
}

unsigned int ESCReset(ESC_Struct* ESC_hande)
{
    digitalWrite(ESC_hande->pin, LOW);
    SPI.transfer(ESC_CMD_RESET);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ESC_hande->pin, HIGH);
    return 0;
}






// Code
