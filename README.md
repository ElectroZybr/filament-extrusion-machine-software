# ESP32 Filament Extruder Control System

Embedded software for controlling a filament production machine based on ESP32.

## Features
- Real-time control of 3 stepper motors (pulling, winding, packing)
- PID-based diameter and tension regulation
- Digital micrometer integration
- LCD menu with rotary encoder navigation
- EEPROM-based settings storage
- Production statistics: speed, weight, time left

## Hardware
- ESP32
- Stepper drivers (3x)
- Digital micrometer
- Rotary encoder with button
- Endstop & tension sensors
- I2C LCD 20x4

## Architecture
- Timer interrupt for motor stepping & encoder handling
- Main loop for UI, logic and PID updates
- Finite State Machine: Wait → Prepare → Work → Done / Error

## Custom Libraries
- **LcdManager** – reusable embedded UI/menu framework for I2C LCD + rotary encoder
- **DigitalMicrometer** – hardware abstraction layer for micrometer sensor

## Technologies
- C++ / Arduino / ESP-IDF style
- FreeRTOS
- GyverPID, GyverStepper, GyverEncoder
- I2C, EEPROM

## What I would improve next
- Separate logic into modules
- Add error logging & diagnostics
- Replace globals with structured state objects
- Add unit tests for logic parts
