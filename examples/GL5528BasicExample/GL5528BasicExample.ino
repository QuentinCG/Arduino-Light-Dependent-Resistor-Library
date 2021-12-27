/*
 * \brief Get light intensity in lux every second
 *
 * Schematics:
 *                           ^
 *            _____      ___/___
 *    5V |---|_____|----|__/____|--| GND
 *    or      3.3K        /
 *   3.3V                 GL5528
 *
 * \author Quentin Comte-Gaz <quentin@comte-gaz.com>
 * \date 27 December 2021
 * \license MIT License (contact me if too restrictive)
 * \copyright Copyright (c) 2021 Quentin Comte-Gaz
 * \version 1.3
 */

#include <LightDependentResistor.h>

#define OTHER_RESISTOR 3300 //ohms
#define USED_PIN A0
#define USED_PHOTOCELL LightDependentResistor::GL5528

// Create a GL5528 photocell instance (on A0 pin)
LightDependentResistor photocell(USED_PIN, OTHER_RESISTOR, USED_PHOTOCELL/*, 10 <-- Default ADC resolution, 10 <-- Default smooth if used */);

void setup(void)
{
  Serial.begin(9600);

  // Uncomment if your photocell is connected to 5V/3.3V instead of the other resistor
  //photocell.setPhotocellPositionOnGround(false);

  Serial.print("Light intensity will be given every second...\n");
}

void loop()
{
  // Check light intensity every second

  float intensity_in_lux = photocell.getCurrentLux();
  // prefer "float intensity_in_lux = photocell.getSmoothedLux()" if you want to have a "smoothed" value

  Serial.print("Light intensity: ");
  Serial.print(intensity_in_lux);
  Serial.print("lx (");
  Serial.print(LightDependentResistor::luxToFootCandles(intensity_in_lux));
  Serial.print("fc)\n");

  delay(1000);
}
