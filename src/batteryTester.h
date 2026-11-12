#pragma once
#include "Arduino.h"
#include "flprogUtilites.h"
#include <stdint.h>

// Состояния цикла тестирования аккумулятора. Числовые значения являются
// частью существующего API FLProg — не перенумеровывать без миграции проектов.
#define FLPROG_BATTERY_TESTER_STOP_STEP 0
#define FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP 1
#define FLPROG_BATTERY_TESTER_DISCHARGE_STEP 2
#define FLPROG_BATTERY_TESTER_FINAL_CHARGE_STEP 3
#define FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP 4
#define FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP 5
#define FLPROG_BATTERY_TESTER_WAITING_FOR_STOP_STEP 6

/**
 * @brief Программная модель последовательности заряд–разряд–заряд.
 *
 * Класс НЕ управляет реле напрямую. После pool() внешний код FLProg обязан
 * прочитать getMainRelayStatus()/getChangeoverRelayStatus() и применить
 * значения к оборудованию. Значение false не гарантирует физическое
 * обесточивание без независимой схемы безопасности.
 *
 * Измерения: ток в мА (со знаком), напряжение и порог разряда в мВ;
 * интервалы задаются в мс. Модель ёмкости: раз в секунду накапливается
 * |ток| в единицах мА·с; ёмкость в мА·ч возвращается делением на 3600.
 * Реальные ограничения тока, напряжения, температуры и защита от КЗ
 * должны обеспечиваться независимо от этого класса.
 */
class FlprogBatteryTester
{
public:
  /** @brief Выполнить одну итерацию автомата; вызывать из Arduino loop(). */
  void pool();

  // Команды защёлкиваются и обрабатываются следующим вызовом pool().
  void pusk() { _pusk = true; };               // Запустить стандартный цикл.
  void stop() { _stop = true; };               // Запросить остановку цикла.
  void handCharge() { _handCharge = true; };   // Перейти в ручной заряд.
  void handDischarge() { _handDischarge = true; }; // Перейти в ручной разряд.
  /** @brief Признак переполюсовки: при true pool() переводит цикл в STOP. */
  void setPolarityReversal(bool value) { _polarityReversal = value; };

  // Измерения и пороги. Приложение отвечает за их достоверность и диапазоны.
  void setCurrent(int16_t current) { _current = current; }; // мА
  void setVoltage(int16_t voltage) { _voltage = voltage; }; // мВ
  void setEndChargeTime(uint32_t time) { _endChargeTime = time; }; // мс
  void setEndChargeCurrent(int16_t current) { _endChargeCurrent = current; }; // мА
  void setEndDischargeVoltage(int16_t voltage) { _endDischargeVoltage = voltage; }; // мВ

  /** @brief Приближённая накопленная ёмкость в мА·ч (усечение дробной части). */
  uint32_t getResidualCapacity() { return _residualCapacity / 3600UL; };
  /** @brief Сырой интегратор ёмкости: мА·с; не мА·ч. */
  uint32_t getRawResidualCapacity() { return _residualCapacity; };
  /** @brief Восстановление интегратора в тех же единицах мА·с. */
  void setRawResidualCapacity(uint32_t residualCapacity) { _residualCapacity = residualCapacity; };

  // Только логические команды для внешней схемы коммутации.
  bool getMainRelayStatus() { return _mainRelayStatus; };
  bool getChangeoverRelayStatus() { return _changeoverRelayStatus; };
  uint8_t getCurrentStep() { return _currentStep; };
  /** @brief Восстановить состояние по константам FLPROG_BATTERY_TESTER_*. */
  void setCurrentStep(uint8_t currentStep) { _currentStep = currentStep; };
  /** @brief Ошибка текущего вызова pool(): 1 — нет тока, 2 — нет напряжения. */
  uint8_t getErrorCode() { return _errorCode; };

protected:
  void executeChargeStep();    // Контроль завершения заряда по току и времени.
  void executeDischargeStep(); // Контроль завершения разряда по напряжению.
  void setOutputsStatus();    // Вычисление логических выходов и учёт ёмкости.

  bool _pusk = false;
  bool _stop = false;
  bool _handCharge = false;
  bool _handDischarge = false;
  uint8_t _currentStep = FLPROG_BATTERY_TESTER_STOP_STEP;
  int16_t _current = 0;
  int16_t _voltage = 0;
  uint32_t _endChargeTime = 0;
  int16_t _endChargeCurrent = 0;
  // Временные метки ставятся при реальном входе в режим, а не в конструкторе.
  uint32_t _startTimerTime = 0;
  uint32_t _startTimerCounterTime = 0;
  bool _isDischargeCounterInitialized = false;
  bool _isWhiteTimeout = false; // Историческое имя: ожидание устойчивого порога.
  uint32_t _residualCapacity = 0; // Сырой интегратор ёмкости, мА·с.
  int16_t _endDischargeVoltage = 10000; // мВ
  bool _mainRelayStatus = false;
  bool _changeoverRelayStatus = false;
  bool _polarityReversal = false;
  uint8_t _errorCode = 0;
};
