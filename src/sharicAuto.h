#pragma once
#include "Arduino.h"
#include "flprogUtilites.h"
#include "flprogAbstractStepMotor.h"

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

class FlprogSharicAuto
{
public:
  FlprogSharicAuto() {};
  void pool();
  void pusk();
  void stop();
  void setWorkMode(bool value);
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

  void setPermissionMoveRight(bool value) { _permissionMoveRight = value; };
  void setPermissionMoveLeft(bool value) { _permissionMoveLeft = value; };

  void setCurrentMotorStatus(uint8_t status) { _currentMotorStatus = status; };

  void setZeroPinStatus(bool value) { _zeroPinStatus = value; };
  void setStartFindZeroPosition(int32_t position) { _startFindZeroPosition = position; };

  void setOpenClampPinStatus(bool value) { _openClampPinStatus = value; };
  void setCloseClampPinStatus(bool value) { _closeClampPinStatus = value; };

  void setOpenPartFeedTimeout(uint32_t timeout) { _openPartFeedTimeout = timeout; };

  uint8_t getCurrentMotorMode() { return _currentMotorMode; };
  uint8_t getCurrentTargetMotorPosition() { return _currentTargetMotorPosition; };
  uint32_t getCurrentMotorSpeed() { return _currentMotorSpeed; };
  bool getOpenClampOutputStatus() { return _openClampOutputStatus; };
  bool getCloseClampOutputStatus() { return _closeClampOutputStatus; };
  bool getOpenPartFeedOutputStatus() { return _openPartFeedOutputStatus; };
  bool getClosePartFeedOutputStatus() { return _closePartFeedOutputStatus; };

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
};
