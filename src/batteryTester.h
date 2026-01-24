#pragma once
#include "Arduino.h"
#include "flprogUtilites.h"

#define FLPROG_BATTERY_TESTER_STOP_STEP 0
#define FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP 1
#define FLPROG_BATTERY_TESTER_DISCHARGE_STEP 2
#define FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP 3
#define FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP 4
#define FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP 5
#define FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP 6

class FlprogBatteryTester
{
public:
  void pool();
  void pusk(){_pusk = true; };
  void stop(){_stop = true; };
  void handCharge() { _handCharge = true; };
  void handDischarge() { _handDischarge = true; };
  void setPolarityReversal(bool value) { _polarityReversal = value; };

  void setCurrent(int16_t current) { _current = current; };
  void setVoltage(int16_t voltage) { _voltage = voltage; };
  void setEndChargeTime(uint32_t time) { _endChargeTime = time; };
  void setEndChargeCurrent(int16_t current) { _endChargeCurrent = current; };
  void setEndDischargeVoltage(int16_t voltage) { _endDischargeVoltage = voltage; };

  uint32_t getResidualCapacity() { return (uint32_t)((_residualCapacity / 3600.0)); };
  bool getMainRelayStatus() { return _mainRelayStatus; };
  bool getChangeoverRelayStatus() { return _changeoverRelayStatus; };
  uint8_t getCurrentStep() { return _currentStep; };

protected:
  void executeChargeStep();
  void executeDischargeStep();
  void setOutputsStatus();
  bool _pusk = false;
  bool _stop = false;
  bool _handCharge = false;
  bool _handDischarge =false;
  uint8_t _currentStep = FLPROG_BATTERY_TESTER_STOP_STEP;
  int16_t _current = 0;
  int16_t _voltage = 0;
  uint32_t _endChargeTime = 0;
  int16_t _endChargeCurrent = 0;
  uint32_t _startTimerTime = 0;
  bool _isWhiteTimeout = false;
  uint32_t _residualCapacity = 0;       // Остаточная емкость в мАcek
  int16_t _endDischargeVoltage = 10000; // Напряжение конца разряда в мВ
  bool _mainRelayStatus = false;
  bool _changeoverRelayStatus = false;
  bool _polarityReversal = false;
};
