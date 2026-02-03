#include "batteryTester.h"

void FlprogBatteryTester::pool()
{
  _errorCode = 0;
  if ((_stop) || (_polarityReversal))
  {
    _currentStep = FLPROG_BATTERY_TESTER_STOP_STEP;
    _stop = false;
    _pusk = false;
    _handCharge = false;
    _handDischarge = false;
    setOutputsStatus();
    return;
  }
  if (_handDischarge)
  {
    if (_currentStep == FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP)
    {
      setOutputsStatus();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP;
    _pusk = false;
    _handCharge = false;
    _handDischarge = false;
    _residualCapacity = 0;
    _startTimerCounterTime = millis();
    setOutputsStatus();
    return;
  }
  if (_handCharge)
  {
    if (_currentStep == FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP)
    {
      setOutputsStatus();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP;
    _pusk = false;
    _handCharge = false;
    _handDischarge = false;
    _residualCapacity = 0;
    setOutputsStatus();
    return;
  }
  if (_pusk)
  {
    _pusk = false;
    if (!((_currentStep == FLPROG_BATTERY_TESTER_STOP_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP)))
    {
      setOutputsStatus();
      return;
    }
    if (_voltage == 0)
    {
      setOutputsStatus();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP;
    _isWhiteTimeout = false;
    setOutputsStatus();
    executeChargeStep();
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP))
  {
    setOutputsStatus();
    executeChargeStep();
    return;
  }
  if (_currentStep == FLPROG_BATTERY_TESTER_DISCHARGE_STEP)
  {
    setOutputsStatus();
    executeDischargeStep();
    return;
  }
  if (_currentStep == FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP)
  {
    if (_voltage > 0)
    {
      setOutputsStatus();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_STOP_STEP;
    _stop = false;
    _pusk = false;
    _handCharge = false;
    _handDischarge = false;
    setOutputsStatus();
    return;
  }
  setOutputsStatus();
}

void FlprogBatteryTester::executeChargeStep()
{
  if (_errorCode == 1)
  {
    return;
  }
  if (abs(_current) > _endChargeCurrent)
  {
    _isWhiteTimeout = false;
    _startTimerTime = millis();
    setOutputsStatus();
    return;
  }
  if (!_isWhiteTimeout)
  {
    _startTimerTime = millis();
    _isWhiteTimeout = true;
    setOutputsStatus();
    return;
  }
  if (flprog::isTimer(_startTimerTime, _endChargeTime))
  {
    if (_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP)
    {
      _currentStep = FLPROG_BATTERY_TESTER_DISCHARGE_STEP;
      _startTimerTime = millis();
      _startTimerCounterTime = millis();
      _isWhiteTimeout = false;
      setOutputsStatus();
      executeDischargeStep();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP;
  }
  setOutputsStatus();
}

void FlprogBatteryTester::executeDischargeStep()
{
  if (_errorCode == 2)
  {
    return;
  }
  if (abs(_voltage) > _endDischargeVoltage)
  {
    _isWhiteTimeout = false;
    _startTimerTime = millis();
    setOutputsStatus();
    return;
  }
  if (!_isWhiteTimeout)
  {
    _startTimerTime = millis();
    _isWhiteTimeout = true;
    setOutputsStatus();
    return;
  }
  if (flprog::isTimer(_startTimerTime, 60000))
  {
    _currentStep = FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP;
    _startTimerTime = millis();
    _isWhiteTimeout = false;
    setOutputsStatus();
    executeChargeStep();
    return;
  }
  setOutputsStatus();
}

void FlprogBatteryTester::setOutputsStatus()
{
  if ((_currentStep == FLPROG_BATTERY_TESTER_STOP_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP))
  {
    _mainRelayStatus = false;
    _changeoverRelayStatus = false;
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP))
  {
    if (_currentStep != FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP)
    {
      _residualCapacity = 0;
    }
    if (_current == 0)
    {
      _errorCode = 1;
    }
    _mainRelayStatus = true;
    _changeoverRelayStatus = true;
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_DISCHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP))
  {
    if (_voltage == 0)
    {
      _errorCode = 2;
    }
    if (flprog::isTimer(_startTimerCounterTime, 1000))
    {
      _residualCapacity += abs(_current);
      _startTimerCounterTime = millis();
    }
    _mainRelayStatus = true;
    _changeoverRelayStatus = false;
    return;
  }
}
