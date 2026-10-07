# STM32 ADS1232 Driver (HAL)

A simple, lightweight C driver for the Texas Instruments **ADS1232** 24-bit ADC, built for STM32 microcontrollers using the HAL library. 

This driver was originally designed for **load cells** (weighing scales, thrust stands), but since it just reads the raw 24-bit differential voltage, you can easily adapt it for pressure sensors, thermistors, or any other application that uses the ADS1232.

## Features
- **Software SPI (Bit-banging):** You can connect the ADS1232 to any standard GPIO pins. No hardware SPI setup is required.
- **Timeout Protection:** The reading function won't lock up your MCU if the sensor gets disconnected or stuck.
- **Built-in Calibration:** Includes `Tare` and `GetWeight` functions out of the box.

## Getting Started

### 1. Initialization
Just include `ads1232.h` and initialize the struct with your specific GPIO ports and pins.

```c
#include "ads1232.h"

ADS1232_HandleTypeDef hx;

int main(void) {
    // HAL Init, System Clock Config, MX_GPIO_Init...
    
    // Init: Struct, DOUT_Port, DOUT_Pin, SCLK_Port, SCLK_Pin, PDWN_Port, PDWN_Pin
    ADS1232_Init(&hx, GPIOF, GPIO_PIN_13, GPIOE, GPIO_PIN_9, GPIOE, GPIO_PIN_11);
    
    // Give the ADC a moment to settle
    HAL_Delay(500); 
    
    // ...

## CubeMX (.ioc) Configuration
To make this driver work correctly, you need to configure three pins in STM32CubeMX. Since this driver uses software SPI, you configure them as standard GPIOs, **not** as hardware SPI pins.

*   **DOUT_Pin:** `GPIO_Input`
    *   *GPIO Pull-up/Pull-down:* No pull-up and no pull-down (or Pull-up if you want to be extra safe when the sensor is disconnected).
*   **SCLK_Pin:** `GPIO_Output`
    *   *GPIO output level:* Low
    *   *GPIO mode:* Output Push Pull
    *   *GPIO Pull-up/Pull-down:* No pull-up and no pull-down
    *   *Maximum output speed:* High
*   **PDWN_Pin:** `GPIO_Output`
    *   *GPIO output level:* High (Keeps the chip awake by default)(Or just hardwire it to 5V)
    *   *GPIO mode:* Output Push Pull

---

## Hardware Wiring (Load Cell Example)
The ADS1232 has several configuration pins that dictate its behavior (gain, speed, oscillator). If you are using a standard **Load Cell (Strain Gauge)**, here is the most common and robust wiring scheme. 

This setup uses the internal oscillator, sets the gain to 128 (perfect for load cells), and sets the sampling rate to 80 SPS.

### 1. Digital Control (To STM32)
*   **DOUT** -> STM32 `DOUT_Pin`
*   **SCLK** -> STM32 `SCLK_Pin`
*   **PDWN** -> STM32 `PDWN_Pin`

### 2. Tied to GND (0V)
Solder these pins together and connect them to common Ground:
*   **CLKIN:** Ties to GND to enable the internal oscillator.
*   **A0:** Ties to GND to select Channel 1 (`AINP1` / `AINN1`).
*   **TEMP:** Ties to GND to disable the internal temperature sensor.
*   **GND / AGND:** Main ground connections.

### 3. Tied to 5V (or 3.3V)
Solder these pins together and connect them to your VCC (usually 5V for better load cell excitation):
*   **GAIN0 & GAIN1:** Both tied to HIGH sets the internal PGA gain to 128.
*   **SPEED:** Tied to HIGH sets the data rate to 80 SPS (better for dynamic thrust/weight measurement).
*   **REFP (VREF+):** Reference voltage (connect to the same 5V).
*   **VDD / AVDD:** Power supply for the chip.
*   *Note:* You also power your Load Cell excitation (Red wire) from this exact same 5V node.

### 4. Load Cell Inputs
*   **AINP1:** Load Cell Signal + (Usually the Green wire)
*   **AINN1:** Load Cell Signal - (Usually the White wire)