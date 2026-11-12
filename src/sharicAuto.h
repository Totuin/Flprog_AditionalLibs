#pragma once
#include "Arduino.h"
#include "flprogUtilites.h"
#include "flprogAbstractStepMotor.h"

// Этапы технологического цикла. Значения используют проекты FLProg.
#define FLPROG_SHARIC_AUTO_STOP_STEP 0
#define FLPROG_SHARIC_AUTO_FIND_ZERO_STEP 1
#define FLPROG_SHARIC_AUTO_GOTO_START_FIND_ZERO_STEP_POSITION 2
#define FLPROG_SHARIC_AUTO_OPEN_CLAMP_STEP 3
#define FLPROG_SHARIC_AUTO_OPEN_PART_FEED_STEP 4
#define FLPROG_SHARIC_AUTO_CLOSE_CLAMP_STEP 5
#define FLPROG_SHARIC_AUTO_CLOSE_PART_FEED_STEP 6
#define FLPROG_SHARIC_AUTO_MOVE_TO_START_SLOT_CUTTER_POSITION_STEP 7
#define FLPROG_SHARIC_AUTO_MOVE_TO_END_SLOT_CUTTER_POSITION_STEP 8
#define FLPROG_SHARIC_AUTO_MOVE_TO_START_SIDE_MILLING_CUTTER_POSITION_STEP 9
#define FLPROG_SHARIC_AUTO_MOVE_TO_END_SIDE_MILLING_CUTTER_POSITION_STEP 10

// Один цикл или непрерывная обработка деталей.
#define FLPROG_SHARIC_AUTO_SINGLE_CYCLE_MODE 0
#define FLPROG_SHARIC_AUTO_CONTINUOUS_MODE 1

/**
 * @brief Автомат последовательности подачи, зажима и обработки детали.
 *
 * Класс только формирует логические команды для двигателя, зажима и подачи.
 * Реальную блокировку движений, аварийную остановку и межблочную защиту
 * должны выполнять внешние схемы и исполнительный контроллер.
 *
 * ВАЖНО: разрешения setPermissionMoveRight/Left() присутствуют в API,
 * но исходный автомат их не проверяет. Это незакрытый вопрос безопасности.
 * В названиях методов SlotСutter буква «С» (перед utter) — кириллическая,
 * её нельзя механически заменить на латинскую C без нарушения API.
 */
class FlprogSharicAuto
{
public:
  /** @brief Один шаг автомата; вызывать периодически в loop(). */
  void pool();
  /** @brief Начать цикл с поиска нуля (если не запущен). */
  void pusk();
  /** @brief Остановить автомат и сбросить логические исполнительные выходы. */
  void stop();
  /** @brief true — запуск; false — остановка. */
  void setWorkMode(bool value);
  /** @brief Выбрать одноцикловый или непрерывный режим. */
  void setMode(uint8_t mode) { _mode = mode; };
  // Скорости — шаги/с, положения — шаги от нулевой точки.
  void setFindZeroSpeed(uint32_t speed) { _findZeroSpeed = speed; };
  void setCurrentPosition(int32_t position) { _currentPosition = position; };

  void setStartSlotСutterPosition(int32_t position) { _startSlotСutterPosition = position; };
  void setStartSlotСutterSpeed(uint32_t speed) { _startSlotСutterSpeed = speed; };
  void setEndSlotСutterPosition(int32_t position) { _endSlotСutterPosition = position; };
  void setEndSlotСutterSpeed(uint32_t speed) { _endSlotСutterSpeed = speed; };

  void setStartSideMillingCutterPosition(int32_t position) { _startSideMillingCutterPosition = position; };
  void setStartSideMillingCutterSpeed(uint32_t speed) { _startSideMillingCutterSpeed = speed; };
  void setEndSideMillingCutterPosition(int32_t position) { _endSideMillingCutterPosition = position; };
  void setEndSideMillingCutterSpeed(uint32_t speed) { _endSideMillingCutterSpeed = speed; };

  // Эти флаги сохраняются, но текущая реализация автомата их не применяет:
  // необходим отдельный аппаратный/проектный контроль запрета движения.
  void setPermissionMoveRight(bool value) { _permissionMoveRight = value; };
  void setPermissionMoveLeft(bool value) { _permissionMoveLeft = value; };

  /** @brief Передать статус двигателя из внешнего контроллера шагов. */
  void setCurrentMotorStatus(uint8_t status) { _currentMotorStatus = status; };

  // Обратная связь датчиков нуля и зажима; их истинные уровни задаёт проект.
  void setZeroPinStatus(bool value) { _zeroPinStatus = value; };
  void setStartFindZeroPosition(int32_t position) { _startFindZeroPosition = position; };

  void setOpenClampPinStatus(bool value) { _openClampPinStatus = value; };
  void setCloseClampPinStatus(bool value) { _closeClampPinStatus = value; };

  // Время включения подачи в миллисекундах.
  void setOpenPartFeedTimeout(uint32_t timeout) { _openPartFeedTimeout = timeout; };
  void setClosePartFeedTimeout(uint32_t timeout) { _closePartFeedTimeout = timeout; };

  // Выходы логических команд — внешняя программа применяет их к оборудованию.
  uint8_t getCurrentMotorMode() { return _currentMotorMode; };
  int32_t getCurrentTargetMotorPosition() { return _currentTargetMotorPosition; };
  uint32_t getCurrentMotorSpeed() { return _currentMotorSpeed; };
  bool getOpenClampOutputStatus() { return _openClampOutputStatus; };
  bool getCloseClampOutputStatus() { return _closeClampOutputStatus; };
  bool getOpenPartFeedOutputStatus() { return _openPartFeedOutputStatus; };
  bool getClosePartFeedOutputStatus() { return _closePartFeedOutputStatus; };
  /** @brief Одноцикловый импульс команды остановки при завершении детали. */
  bool getStopCommand() { return _stopCommand; };
  /** @brief Одноцикловый признак завершённой детали. */
  bool getHasNewPart() { return _hasNewPart; };

protected:
  void executeFindZeroStep();
  void moveToStartFindZeroPosition();
  void executeOpenClampStep();
  void executeOpenPartFeedStep();
  void executeCloseClampStep();
  void executeClosePartFeedStep();
  void executeMoveToStartSlotСutterPositionStep();
  void executeMoveToEndSlotСutterPositionStep();
  void executeMoveToStartSideMillingCutterPositionStep();
  void executeMoveToEndSideMillingCutterPositionStep();

  bool _isRun = false;
  uint8_t _currentStep = FLPROG_SHARIC_AUTO_STOP_STEP;
  int32_t _currentPosition = 0;
  uint32_t _findZeroSpeed = 200; // скорость поиска нуля в шагах в секунду
  int32_t _startSlotСutterPosition = 0;
  uint32_t _startSlotСutterSpeed = 500;
  int32_t _endSlotСutterPosition = 0;
  uint32_t _endSlotСutterSpeed = 500;
  int32_t _startSideMillingCutterPosition = 0;
  uint32_t _startSideMillingCutterSpeed = 500;
  int32_t _endSideMillingCutterPosition = 0;
  uint32_t _endSideMillingCutterSpeed = 500;
  bool _permissionMoveRight = false;
  bool _permissionMoveLeft = false;
  uint8_t _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  uint8_t _currentMotorStatus = FLPROG_STOP_STEP_MOTOR_STATUS;
  int32_t _currentTargetMotorPosition = 0;
  uint32_t _currentMotorSpeed = 0;
  bool _zeroPinStatus = false;
  int32_t _startFindZeroPosition = 0;
  bool _openClampPinStatus = false;
  bool _closeClampPinStatus = false;
  bool _openClampOutputStatus = false;
  bool _closeClampOutputStatus = false;
  bool _openPartFeedOutputStatus = false;
  bool _closePartFeedOutputStatus = false;
  uint32_t _startTimerTime = 0;
  uint32_t _openPartFeedTimeout = 1000;
  uint32_t _closePartFeedTimeout = 1000;
  bool _isInit = false;
  uint8_t _mode = FLPROG_SHARIC_AUTO_CONTINUOUS_MODE;
  bool _stopCommand = false;
  bool _hasNewPart = false;
};
