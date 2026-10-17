/**
 *
 * @brief Configuration file for all parameters
 *
 * @file app_config.h
 * @author Bernhard Fölk
 * @date 19.10.2025
 *
 */

#pragma once

#include <Arduino.h>

#include <Ton.h>
#include <EdgePosNeg.h>

#define DEBUGGING true

#if DEBUGGING == true
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// IOs
#define PIN_MOTOR_STEP 9
#define PIN_MOTOR_DIR 10

#define PIN_ENDSTOP_POS 0
#define PIN_ENDSTOP_NEG 1

#define PIN_BUTTON_FORWARD 6
#define PIN_BUTTON_BACKWARD 7
