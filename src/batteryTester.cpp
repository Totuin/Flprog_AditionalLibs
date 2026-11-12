#include "batteryTester.h"

namespace
{
// int на 8-битных AVR имеет 16 бит. abs(INT16_MIN) переполняет int,
// поэтому модуль измерения вычисляется после расширения до int32_t.
uint32_t batteryMagnitude(int16_t value)
{
  const int32_t wideValue = static_cast<int32_t>(value);
  return static_cast<uint32_t>(wideValue < 0 ? -wideValue : wideValue);
}
}


// Приоритет команд: аварийный STOP/переполюсовка -> ручной режим ->
// запуск автоматического цикла -> выполнение текущего этапа.
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

// Условия завершения заряда должны удерживаться непрерывно в течение
// _endChargeTime. При превышении тока окно ожидания сбрасывается.
void FlprogBatteryTester::executeChargeStep()
{
  if (_errorCode == 1)
  {
    return;
  }
  if (static_cast<int32_t>(batteryMagnitude(_current)) > static_cast<int32_t>(_endChargeCurrent))
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

// Завершение разряда подтверждается устойчивым снижением напряжения
// (в исходном алгоритме — в течение одной минуты).
void FlprogBatteryTester::executeDischargeStep()
{
  if (_errorCode == 2)
  {
    return;
  }
  if (static_cast<int32_t>(batteryMagnitude(_voltage)) > static_cast<int32_t>(_endDischargeVoltage))
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

// Выходы являются логическими запросами к реле, а не аппаратными защитами.
// Код ошибки 1/2 сообщает об отсутствии измерения, но сам по себе не
// отключает питание. Это исходная логика, требующая проверки на схеме.
void FlprogBatteryTester::setOutputsStatus()
{
  if ((_currentStep == FLPROG_BATTERY_TESTER_STOP_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP))
  {
    _mainRelayStatus = false;
    _changeoverRelayStatus = false;
    _isDischargeCounterInitialized = false;
    return;
  }
  if ((_currentStep == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP) || (_currentStep == FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP))
  {
    _isDischargeCounterInitialized = false;
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
    // Счётчик включается только при первом реальном входе в разряд.
    // Это защищает от преждевременного накопления, если объект создан
    // до запуска millis() или режим восстановлен через setCurrentStep().
    if (!_isDischargeCounterInitialized)
    {
      _startTimerCounterTime = millis();
      _isDischargeCounterInitialized = true;
    }
    else if (flprog::isTimer(_startTimerCounterTime, 1000))
    {
      // Историческая модель: за каждый отсчитанный секундный интервал
      // добавить |ток| в мА·с. Не меняем единицы/формат существующего API.
      const uint32_t increment = batteryMagnitude(_current);
      if (increment > UINT32_MAX - _residualCapacity)
      {
        _residualCapacity = UINT32_MAX; // Не допускаем перехода счётчика через 0.
      }
      else
      {
        _residualCapacity += increment;
      }
      _startTimerCounterTime = millis();
    }
    _mainRelayStatus = true;
    _changeoverRelayStatus = false;
    return;
  }
  // Неизвестный код этапа (в том числе переданный setCurrentStep):
  // никогда не оставляем активным прежнее состояние реле.
  _mainRelayStatus = false;
  _changeoverRelayStatus = false;
  _isDischargeCounterInitialized = false;
}
