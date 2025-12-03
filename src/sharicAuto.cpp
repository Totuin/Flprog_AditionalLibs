#include "sharicAuto.h"

void FlprogSharicAuto::pool()
{
  if (_stopCommand)
  {
    _stopCommand = false;
  }
  if (_hasNewPart)
  {
    _hasNewPart = false;
  }
  if (!_isRun)
  {
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_STOP_STEP)
  {
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_FIND_ZERO_STEP)
  {
    executeFindZeroStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_GOTO_START_FIND_ZERO_STEP_POSITION)
  {
    moveToStartFindZeroPosition();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_OPEN_CLAMP_STEP)
  {
    executeOpenClampStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_OPEN_PART_FEED_STEP)
  {
    executeOpenPartFeedStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_CLOSE_CLAMP_STEP)
  {
    executeCloseClampStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_CLOSE_PART_FEED_STEP)
  {
    executeClosePartFeedStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_MOVE_TO_START_SLOT_CUTTER_POSITION_STEP)
  {
    executeMoveToStartSlotСutterPositionStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_MOVE_TO_END_SLOT_CUTTER_POSITION_STEP)
  {
    executeMoveToEndSlotСutterPositionStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_MOVE_TO_START_SIDE_MILLING_CUTTER_POSITION_STEP)
  {
    executeMoveToStartSideMillingCutterPositionStep();
    return;
  }
  if (_currentStep == FLPROG_SHARIC_AUTO_MOVE_TO_END_SIDE_MILLING_CUTTER_POSITION_STEP)
  {
    executeMoveToEndSideMillingCutterPositionStep();
    return;
  }
}

void FlprogSharicAuto::setWorkMode(bool value)
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

void FlprogSharicAuto::stop()
{
  if (!_isRun)
  {
    return;
  }
  _isRun = false;
  _isInit = false;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  _currentStep = FLPROG_SHARIC_AUTO_STOP_STEP;
}

void FlprogSharicAuto::pusk()
{
  if (_isRun)
  {
    return;
  }
  _isRun = true;
  _currentStep = FLPROG_SHARIC_AUTO_FIND_ZERO_STEP;
  _isInit = false;
  if (_currentPosition >= 0)
  {
    moveToStartFindZeroPosition();
    return;
  }
}

void FlprogSharicAuto::executeFindZeroStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_FIND_ZERO_STEP;
  _currentMotorMode = FLPROG_FIND_ZERO_STEP_MOTOR_MODE;
  _currentMotorSpeed = _findZeroSpeed;
  if (_currentMotorStatus == FLPROG_END_FIND_ZERO_STEP_MOTOR_STATUS)
  {
    _currentStep = FLPROG_SHARIC_AUTO_OPEN_CLAMP_STEP;
    executeOpenClampStep();
  }
}

void FlprogSharicAuto::moveToStartFindZeroPosition()
{
  _currentStep = FLPROG_SHARIC_AUTO_GOTO_START_FIND_ZERO_STEP_POSITION;
  _currentMotorSpeed = _findZeroSpeed;
  _currentTargetMotorPosition = _startFindZeroPosition;
  _currentMotorMode = FLPROG_POSITION_TRANSITION_STEP_MOTOR_MODE;
  if (_currentMotorStatus == FLPROG_POSITION_TRANSITION_WAIT_COMMAND_STEP_MOTOR_STATUS)
  {
    _currentStep = FLPROG_SHARIC_AUTO_FIND_ZERO_STEP;
    executeFindZeroStep();
  }
}

void FlprogSharicAuto::executeOpenClampStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_OPEN_CLAMP_STEP;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  _closeClampOutputStatus = false;
  _openClampOutputStatus = true;
  if (_openClampPinStatus)
  {
    if (_isInit)
    {
      _hasNewPart = true;
      if (_mode == FLPROG_SHARIC_AUTO_SINGLE_CYCLE_MODE)
      {
        _stopCommand = true;
        stop();
        return;
      }
    }
    else
    {
      _isInit = true;
    }
    _currentStep = FLPROG_SHARIC_AUTO_OPEN_PART_FEED_STEP;
    _startTimerTime = millis();
    executeOpenPartFeedStep();
  }
}

void FlprogSharicAuto::executeOpenPartFeedStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_OPEN_PART_FEED_STEP;
  _openPartFeedOutputStatus = true;
  _closePartFeedOutputStatus = false;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  if (flprog::isTimer(_startTimerTime, _openPartFeedTimeout))
  {
    _currentStep = FLPROG_SHARIC_AUTO_CLOSE_CLAMP_STEP;
    executeCloseClampStep();
  }
}

void FlprogSharicAuto::executeCloseClampStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_CLOSE_CLAMP_STEP;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  _openClampOutputStatus = false;
  _closeClampOutputStatus = true;
  if (_closeClampPinStatus)
  {
    _currentStep = FLPROG_SHARIC_AUTO_CLOSE_PART_FEED_STEP;
    _startTimerTime = millis();
    executeClosePartFeedStep();
  }
}

void FlprogSharicAuto::executeClosePartFeedStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_CLOSE_PART_FEED_STEP;
  _closePartFeedOutputStatus = true;
  _openPartFeedOutputStatus = false;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  if (flprog::isTimer(_startTimerTime, _closePartFeedTimeout))
  {
    _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_START_SLOT_CUTTER_POSITION_STEP;
    executeMoveToStartSlotСutterPositionStep();
  }
}

void FlprogSharicAuto::executeMoveToStartSlotСutterPositionStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_START_SLOT_CUTTER_POSITION_STEP;
  _currentMotorSpeed = _startSlotСutterSpeed;
  _currentTargetMotorPosition = _startSlotСutterPosition;
  _currentMotorMode = FLPROG_POSITION_TRANSITION_STEP_MOTOR_MODE;
  if (_currentPosition >= _startSlotСutterPosition)
  {
    _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_END_SLOT_CUTTER_POSITION_STEP;
    executeMoveToEndSlotСutterPositionStep();
  }
}

void FlprogSharicAuto::executeMoveToEndSlotСutterPositionStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_END_SLOT_CUTTER_POSITION_STEP;
  _currentMotorSpeed = _endSlotСutterSpeed;
  _currentTargetMotorPosition = _endSlotСutterPosition;
  _currentMotorMode = FLPROG_POSITION_TRANSITION_STEP_MOTOR_MODE;
  if (_currentPosition >= _endSlotСutterPosition)
  {
    _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_START_SIDE_MILLING_CUTTER_POSITION_STEP;
    executeMoveToStartSideMillingCutterPositionStep();
  }
}

void FlprogSharicAuto::executeMoveToStartSideMillingCutterPositionStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_START_SIDE_MILLING_CUTTER_POSITION_STEP;
  _currentMotorSpeed = _startSideMillingCutterSpeed;
  _currentTargetMotorPosition = _startSideMillingCutterPosition;
  _currentMotorMode = FLPROG_POSITION_TRANSITION_STEP_MOTOR_MODE;
  if (_currentPosition <= _startSideMillingCutterPosition)
  {
    _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_END_SIDE_MILLING_CUTTER_POSITION_STEP;
    executeMoveToEndSideMillingCutterPositionStep();
  }
}

void FlprogSharicAuto::executeMoveToEndSideMillingCutterPositionStep()
{
  _currentStep = FLPROG_SHARIC_AUTO_MOVE_TO_END_SIDE_MILLING_CUTTER_POSITION_STEP;
  _currentMotorSpeed = _endSideMillingCutterSpeed;
  _currentTargetMotorPosition = _endSideMillingCutterPosition;
  _currentMotorMode = FLPROG_POSITION_TRANSITION_STEP_MOTOR_MODE;
  if (_currentPosition <= _endSideMillingCutterPosition)
  {
    _currentStep = FLPROG_SHARIC_AUTO_FIND_ZERO_STEP;
    executeFindZeroStep();
  }
}