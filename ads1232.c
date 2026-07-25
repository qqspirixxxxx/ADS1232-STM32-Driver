#include "ads1232.h"

void ADS1232_Init(ADS1232_HandleTypeDef* ads, GPIO_TypeDef* dout_port, uint16_t dout_pin, GPIO_TypeDef* sclk_port, uint16_t sclk_pin, GPIO_TypeDef* pdwn_port, uint16_t pdwn_pin) {
    ads->DOUT_Port = dout_port;
    ads->DOUT_Pin = dout_pin;
    ads->SCLK_Port = sclk_port;
    ads->SCLK_Pin = sclk_pin;
    ads->PDWN_Port = pdwn_port;
    ads->PDWN_Pin = pdwn_pin;
    
    ads->offset = 0;
    ads->scale = 1.0f;
    
    // Set DOUT high to avoid accidental reads
    HAL_GPIO_WritePin(dout_port, dout_pin, GPIO_PIN_SET);
}

void ADS1232_PowerUp(ADS1232_HandleTypeDef* ads) {
    HAL_GPIO_WritePin(ads->PDWN_Port, ads->PDWN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_RESET);
}

void ADS1232_PowerDown(ADS1232_HandleTypeDef* ads) {
    HAL_GPIO_WritePin(ads->PDWN_Port, ads->PDWN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_SET);
}

int8_t ADS1232_ReadRaw(ADS1232_HandleTypeDef* ads, int32_t* value) {
    int32_t val = 0;
    uint32_t tickstart = HAL_GetTick();

    // Wait for DOUT to go LOW (data ready) with a 200ms timeout
    while(HAL_GPIO_ReadPin(ads->DOUT_Port, ads->DOUT_Pin) == GPIO_PIN_SET) {
        if(HAL_GetTick() - tickstart > 200) {
            return -1; // Timeout error
        }
    }
    
    // Read 24 bits of data using software SPI (bit-banging)
    for(int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_SET);
        val = val << 1;
        val = val + HAL_GPIO_ReadPin(ads->DOUT_Port, ads->DOUT_Pin);
        HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_RESET);
    }
    
    // Apply 25th clock pulse to force DOUT HIGH (default state)
    HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ads->SCLK_Port, ads->SCLK_Pin, GPIO_PIN_RESET);

    // Sign extension: if the 24th bit is 1, fill the upper 8 bits with 1s to make it a valid 32-bit negative number
    if(val & 0x800000) {
        val |= 0xFF000000;
    }

    *value = val;
    return 0;
}

int8_t ADS1232_Tare(ADS1232_HandleTypeDef *ads, uint8_t times) {
    int64_t sum = 0;
    int32_t val = 0;

    // Take multiple readings to get a stable zero offset
    for (uint8_t i = 0; i < times; i++) {
        if(ADS1232_ReadRaw(ads, &val) != 0) return -1;
        sum += val;
        HAL_Delay(1);
    }
    
    ads->offset = sum / times;
    return 0;
}

int8_t ADS1232_GetWeight(ADS1232_HandleTypeDef *ads, float *weight, uint8_t times) {
    int64_t sum = 0;
    int32_t val = 0;

    // Take multiple readings and average them for stability
    for (uint8_t i = 0; i < times; i++) {
        if(ADS1232_ReadRaw(ads, &val) != 0) return -1;
        sum += val;
        HAL_Delay(1);
    }

    int32_t avg = sum / times;
    
    // Calculate final weight based on the offset and scale factor
    *weight = (float)(avg - ads->offset) / ads->scale;
    return 0;
}