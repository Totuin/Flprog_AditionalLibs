#include <batteryTester.h>

// Демонстрация API без подключённых реле и аккумулятора.
// Команды get*RelayStatus() только выводятся в Serial, не управляют GPIO.
FlprogBatteryTester tester;

void setup()
{
  Serial.begin(115200);
  tester.setVoltage(12000);      // Демонстрационное напряжение, мВ.
  tester.setCurrent(-1000);      // Демонстрационный ток, мА.
  tester.setEndDischargeVoltage(10000);
  tester.handDischarge();        // Только логическое состояние.
}

void loop()
{
  tester.pool();
  Serial.print("step=");
  Serial.print(tester.getCurrentStep());
  Serial.print(" capacity_mAh=");
  Serial.print(tester.getResidualCapacity());
  Serial.print(" relay_request=");
  Serial.println(tester.getMainRelayStatus());
  delay(500);
}
