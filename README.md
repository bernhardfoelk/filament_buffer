# 🧵 Active Filament Buffer / Extruder Assist

This project is an automated filament delivery system (buffer) for 3D printers. It monitors filament tension and presence to actively "push" material toward the printer, reducing the load on the printer's own extruder—especially useful for large setups or high-speed printing.

![Filament buffer](images/filament_buffer.HEIC)

## 🚀 The Engineering Behind It
While many buffers are passive, this is an **active system** controlled by an ESP32 and a stepper motor.

### Technical Highlights:
* **Custom Stepper Driver Class:** I wrote a dedicated driver for the **A4988** from scratch. It handles direction, stepping, and positioning without blocking the main loop, featuring integrated "Jog" modes for manual loading 
* **Industrial Logic Patterns:** Like my other projects, this uses my custom `Ton` (On-Delay) and `EdgePosNeg` classes to debounce sensors and manage timing-sensitive transitions.
* **Closed-Loop Logic:** The system uses filament sensors and endstops to determine when to feed more material or when to stop, preventing filament grinding or snapping.
* **Modular Architecture:** Separation of hardware configuration (`app_config.h`), motor logic (`StepperDriver_A4988`), and the main process state machine.

## 🛠️ Features
* **Auto-Feeding:** Automatically detects when the printer needs more filament and feeds a precise amount (e.g., 30mm bursts).
* **Manual Override:** Physical buttons for "Jog Forward" and "Jog Backward" to make filament swaps easy.
* **Safety Interlocks:** Built-in error states if the filament is missing or if an endstop is triggered unexpectedly.
* **Status Monitoring:** Serial debugging and state tracking to monitor the feeding process in real-time.

## 📁 Project Structure
* `src/main.cpp`: The core state machine managing the feeding process.
* `lib/StepperDriver_A4988/`: My custom-built motor controller.
* `lib/Ton/` & `lib/EdgePosNeg/`: PLC-style timing and edge detection utilities.
* `include/app_config.h`: Centralized pin mapping and hardware constants.

## 🔧 Technical Stack
* **Controller:** ESP32-C3
* **Motor Driver:** A4988 Stepper Driver
* **Language:** C++ (Object-Oriented)
* **Toolchain:** PlatformIO

---
*Status: Work in Progress. Currently working on the stepper driver if it works correctly on slower and faster speeds.*
