#include "sharicAuto.h"

// Однопроходный автомат. Флаги _stopCommand/_hasNewPart видимы
// один вызов pool() после установки, затем сбрасываются.
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
  // Остановка идемпотентна: повторная команда тоже должна обесточить
  // логические выходы, в том числе после прерванного цикла зажима/подачи.
  // Реальную безопасность отключения исполнительных цепей обеспечивает
  // аппаратная схема: состояние false здесь лишь программный запрос.
  _isRun = false;
  _isInit = false;
  _currentMotorMode = FLPROG_STOP_STEP_MOTOR_MODE;
  _currentMotorSpeed = 0;
  _openClampOutputStatus = false;
  _closeClampOutputStatus = false;
  _openPartFeedOutputStatus = false;
  _closePartFeedOutputStatus = false;
  _currentStep = FLPROG_SHARIC_AUTO_STOP_STEP;
}

// Запуск не даёт напрямую электрических команд; полная последовательность
// требует корректной обратной связи датчика нуля и статуса шагового двигателя.
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

// Переходим дальше только при подтверждении END_FIND_ZERO извне.
// Проверка датчиков и обеспечение безопасного направления — вне этого класса.
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

// Зажим: активен только один из двух выходов. Тайм-аута ожидания
// концевого датчика здесь нет — потребуется проектный контроль отказа.
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

// Открываем подачу по длительности, без дополнительного датчика.
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

// Закрываем зажим; переход к подаче при срабатывании датчика закрытия.
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

// Закрываем подачу по тайм-ауту, затем запускаем позиционирование.
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

// Начало фрезерования паза (положительное направление координаты).
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

// Начало бокового фрезерования (отрицательное направление координаты).
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