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

#include "Ton.h"
#include "EdgePosNeg.h"
#include "FastAccelStepper.h"

#define DEBUGGING true

#if DEBUGGING == true
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// Constants
#define PIN_FEED_FORW_BUTTON 7
#define PIN_FEED_BACKW_BUTTON 6

#define PIN_ENDSTOP_NEGATIVE 2
#define PIN_ENDSTOP_POSITIVE 3

#define PIN_STEPPER_STEP 1
#define PIN_STEPPER_DIR 0
