#include "batteryTester.h"

void FlprogBatteryTester::pool()
{
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
}

void FlprogBatteryTester::executeChargeStep()
{
  if (abs(_current) > _endChargeCurrent)
  {
    _isWhiteTimeout = false;
    return;
  }
  if (!_isWhiteTimeout)
  {
    _startTimerTime = millis();
    _isWhiteTimeout = true;
    return;
  }
  if (flprog::isTimer(_startTimerTime, _endChargeTime))
  {
    if (_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP)
    {
      _currentStep = FLPROG_BATTERY_TESTER_DISCHARGE_STEP;
      _startTimerTime = millis();
      setOutputsStatus();
      executeDischargeStep();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP;
    setOutputsStatus();
  }
}

void FlprogBatteryTester::executeDischargeStep()
{
  if (flprog::isTimer(_startTimerTime, 1000))
  {
    _residualCapacity += abs(_current);
    _startTimerTime = millis();
  }
  if (abs(_voltage) < _endDischargeVoltage)
  {
    _currentStep = FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP;
    setOutputsStatus();
    executeChargeStep();
    return;
  }
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
    if (_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP)
    {
      _residualCapacity = 0;
    }
    _mainRelayStatus = true;
    _changeoverRelayStatus = true;
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_DISCHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP))
  {
    _mainRelayStatus = true;
    _changeoverRelayStatus = false;
    return;
  }
}
