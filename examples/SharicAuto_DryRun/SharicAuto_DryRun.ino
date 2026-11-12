#include <sharicAuto.h>

// Логический тест без двигателей, зажимов и выходов GPIO.
// Датчики и статусы двигателя здесь намеренно не эмулируются:
// автомат остаётся в режиме поиска нуля до поступления реальной обратной связи.
FlprogSharicAuto machine;

void setup()
{
  Serial.begin(115200);
  machine.setCurrentPosition(-1);
  machine.pusk();
}

void loop()
{
  machine.pool();
  Serial.print("motor_mode=");
  Serial.print(machine.getCurrentMotorMode());
  Serial.print(" motor_speed=");
  Serial.println(machine.getCurrentMotorSpeed());
  delay(500);
}
