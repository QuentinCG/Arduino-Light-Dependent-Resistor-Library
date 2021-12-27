/*
 * \brief et light intensity from Light dependent Resistor (implementation)
 *
 * \author Quentin Comte-Gaz <quentin@comte-gaz.com>
 * \date 14 June 2020
 * \license MIT License (contact me if too restrictive)
 * \copyright Copyright (c) 2020 Quentin Comte-Gaz
 * \version 1.2
 */

#include "LightDependentResistor.h"

LightDependentResistor::LightDependentResistor(int pin, unsigned long other_resistor, ePhotoCellKind kind, unsigned int adc_resolution_bits, unsigned int smoothing_history_size) :
  _pin (pin),
  _other_resistor (other_resistor),
  _mult_value(32017200),
  _pow_value(1.5832),
  _photocell_on_ground (true),
  _adc_resolution_bits(adc_resolution_bits)
{
  switch (kind) {
    case GL5516:
      _mult_value = 29634400;
      _pow_value = 1.6689;
    break;
    case GL5537_1:
      _mult_value = 32435800;
      _pow_value = 1.4899;
    break;
    case GL5537_2:
      _mult_value = 2801820;
      _pow_value = 1.1772;
    break;
    case GL5539:
      _mult_value = 208510000;
      _pow_value = 1.4850;
    break;
    case GL5549:
      _mult_value = 44682100;
      _pow_value = 1.2750;
    break;
    case GL5528:
    default:
      _mult_value = 32017200;
      _pow_value = 1.5832;
    }
  _init_smoothing(smoothing_history_size);
}

LightDependentResistor::LightDependentResistor(int pin, unsigned long other_resistor, float mult_value, float pow_value, unsigned int adc_resolution_bits, unsigned int smoothing_history_size) :
  _pin (pin),
  _other_resistor (other_resistor),
  _mult_value (mult_value),
  _pow_value (pow_value),
  _photocell_on_ground (true),
  _adc_resolution_bits(adc_resolution_bits)
{
  _init_smoothing(smoothing_history_size);
}

void LightDependentResistor::updatePhotocellParameters(float mult_value, float pow_value)
{
  _mult_value = mult_value;
  _pow_value = pow_value;
}

float LightDependentResistor::luxToFootCandles(float intensity_in_lux)
{
  return intensity_in_lux/10.764;
}

float LightDependentResistor::footCandlesToLux(float intensity_in_footcandles)
{
  return 10.764*intensity_in_footcandles;
}

void LightDependentResistor::setPhotocellPositionOnGround(bool on_ground)
{
  _photocell_on_ground = on_ground;
}

float LightDependentResistor::getCurrentLux() const
{
  // Analog resolution setter is not handled on all boards (not compatible boards: MEGA, ESP8266, Uno)
  #if !defined(__AVR_ATmega1280__) && !defined(__AVR_ATmega2560__) && !defined(ESP8266) && !defined(__AVR_ATmega328P__) && !defined(__AVR_ATmega168__) 
    analogReadResolution(_adc_resolution_bits);
  #endif
  int photocell_value = analogRead(_pin);

  if (pow(2, _adc_resolution_bits)==photocell_value) photocell_value--;
  
  unsigned long photocell_resistor;

  float ratio = ((float)pow(2, _adc_resolution_bits)/(float)photocell_value) - 1;
  if (_photocell_on_ground) {
    photocell_resistor = _other_resistor / ratio;
  } else {
    photocell_resistor = _other_resistor * ratio;
  }

  return _mult_value / (float)pow(photocell_resistor, _pow_value);
}

float LightDependentResistor::getCurrentFootCandles() const
{
  return luxToFootCandles(getCurrentLux());
}

void LightDependentResistor::_init_smoothing(unsigned int smoothing_history_size)
{
  _smoothing_history_size=smoothing_history_size;
  _smoothing_history_values=new float[_smoothing_history_size];
  _smoothing_sum=0.0f;
  _smoothing_history_next=0;
  for (unsigned int i=0;i<_smoothing_history_size;i++) {
    _smoothing_history_values[i]=-1.0f;
  }
}

float LightDependentResistor::getSmoothedLux()
{
  if (_smoothing_history_size==0) {
    //no smoothing enabled. Return current value.
    return getCurrentLux();
  }
  else {
    if (_smoothing_history_values[_smoothing_history_next]<-0.1f) {
      //Smoothing enabled. Not all values filled yet.
      _smoothing_history_values[_smoothing_history_next]=getCurrentLux();
      _smoothing_sum+=_smoothing_history_values[_smoothing_history_next];
      if (_smoothing_history_next<_smoothing_history_size-1) {
        //still not all buffers filled
        _smoothing_history_next++;
        return _smoothing_sum/_smoothing_history_next;
      }
      else {
        //all buffers filled now, start regular operation
        _smoothing_history_next=0;
        return  _smoothing_sum/_smoothing_history_size;
      }
    }
    else {
    ///Smoothing enabled. All buffers filled previously. Regular operation from now on.
    _smoothing_sum-=_smoothing_history_values[_smoothing_history_next];
    _smoothing_history_values[_smoothing_history_next]=getCurrentLux();
    _smoothing_sum+=_smoothing_history_values[_smoothing_history_next];
    _smoothing_history_next=(_smoothing_history_next<_smoothing_history_size-1) ? _smoothing_history_next+1 : 0;
    return _smoothing_sum/_smoothing_history_size;
    }
  }
}

float LightDependentResistor::getSmoothedFootCandles()
{
  return luxToFootCandles(getSmoothedLux());
}
