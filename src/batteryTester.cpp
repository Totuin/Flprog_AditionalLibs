#include "batteryTester.h"

void FlprogBatteryTester::pool()
{
  if (_stopCommand)
  {
    _stopCommand = false;
  }
  if (_currentStep == FLPROG_BATTERY_TESTER_STOP_STEP)
  {
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP))
  {
    executeChargeStep();
    return;
  }
  if (_currentStep == FLPROG_BATTERY_TESTER_DISCHARGE_STEP)
  {
    executeDischargeStep();
    return;
  }
}

void FlprogBatteryTester::pusk()
{
  if (_currentStep != FLPROG_BATTERY_TESTER_STOP_STEP)
  {
    return;
  }
  _currentStep = FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP;
  _residualCapacity = 0;
  _isWhiteTimeout = false;
  _mainRelayStatus = true;
  _changeoverRelayStatus = true;
  executeChargeStep();
}

void FlprogBatteryTester::stop()
{
  _currentStep = FLPROG_BATTERY_TESTER_STOP_STEP;
  _mainRelayStatus = false;
  _changeoverRelayStatus = false;
}

void FlprogBatteryTester::setWorkMode(bool value)
{
  if (value)
  {
    pusk();
  }
  else
  {
    stop();
  }
}

void FlprogBatteryTester::executeChargeStep()
{
  if (abs(_current) > _endChargeCurrent)
  {
    return;
    _isWhiteTimeout = false;
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
      _mainRelayStatus = true;
      _changeoverRelayStatus = false;
      executeDischargeStep();
      return;
    }
    _currentStep = FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP;
    _stopCommand = true;
    _mainRelayStatus = false;
    _changeoverRelayStatus = false;
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
    _mainRelayStatus = true;
    _changeoverRelayStatus = true;
    executeChargeStep();
    return;
  }
}