#ifndef ADS1232_H
#define ADS1232_H

#include "main.h"

typedef struct {
    GPIO_TypeDef* DOUT_Port;
    uint16_t DOUT_Pin;
    GPIO_TypeDef* SCLK_Port;
    uint16_t SCLK_Pin;
    GPIO_TypeDef* PDWN_Port;
    uint16_t PDWN_Pin;

    int32_t offset;
    float scale;
} ADS1232_HandleTypeDef;

// Initialization and power control
void ADS1232_Init(ADS1232_HandleTypeDef* ads, GPIO_TypeDef* dout_port, uint16_t dout_pin, GPIO_TypeDef* sclk_port, uint16_t sclk_pin, GPIO_TypeDef* pdwn_port, uint16_t pdwn_pin);
void ADS1232_PowerDown(ADS1232_HandleTypeDef* ads);
void ADS1232_PowerUp(ADS1232_HandleTypeDef* ads);

// Data reading and calibration
int8_t ADS1232_ReadRaw(ADS1232_HandleTypeDef* ads, int32_t* value);
int8_t ADS1232_Tare(ADS1232_HandleTypeDef* ads, uint8_t times);
int8_t ADS1232_GetWeight(ADS1232_HandleTypeDef* ads, float* weight, uint8_t times);

#endif