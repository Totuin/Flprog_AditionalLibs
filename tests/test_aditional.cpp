#include "batteryTester.h"
#include "sharicAuto.h"
#include <cassert>
#include <cstdint>
#include <limits>

uint32_t fakeMillis = 0;
uint32_t millis() { return fakeMillis; }

static void testBatteryStopAndManualDischarge() {
  FlprogBatteryTester t;
  assert(!t.getMainRelayStatus() && !t.getChangeoverRelayStatus());
  t.setCurrent(-1000);
  t.setVoltage(12000);
  t.handDischarge();
  t.pool();
  assert(t.getCurrentStep() == FLPROG_BATTERY_TESTER_HAND_DISCHARGE_STEP);
  assert(t.getMainRelayStatus() && !t.getChangeoverRelayStatus());
  assert(t.getRawResidualCapacity() == 0);
  fakeMillis += 1000;
  t.pool();
  assert(t.getRawResidualCapacity() == 1000);
  fakeMillis += 1000;
  t.pool();
  assert(t.getRawResidualCapacity() == 2000);
  t.stop();
  t.pool();
  assert(!t.getMainRelayStatus() && !t.getChangeoverRelayStatus());
  assert(t.getCurrentStep() == FLPROG_BATTERY_TESTER_STOP_STEP);
}

static void testBatteryInt16MinAndOverflow() {
  FlprogBatteryTester t;
  fakeMillis = 20000;
  t.setVoltage(12000);
  t.setCurrent(std::numeric_limits<int16_t>::min());
  t.handDischarge();
  t.pool();
  fakeMillis += 1000;
  t.pool();
  assert(t.getRawResidualCapacity() == 32768u);
  t.setRawResidualCapacity(UINT32_MAX - 100u);
  fakeMillis += 1000;
  t.pool();
  assert(t.getRawResidualCapacity() == UINT32_MAX);
  assert(t.getResidualCapacity() == UINT32_MAX / 3600u);
}

static void testBatteryRestoreAndMillisWrap() {
  FlprogBatteryTester t;
  fakeMillis = UINT32_MAX - 500u;
  t.setVoltage(12000);
  t.setCurrent(100);
  t.setCurrentStep(FLPROG_BATTERY_TESTER_DISCHARGE_STEP);
  t.pool();
  assert(t.getRawResidualCapacity() == 0); // No stale startup timestamp.
  fakeMillis += 999;
  t.pool();
  assert(t.getRawResidualCapacity() == 0);
  fakeMillis += 1;
  t.pool();
  assert(t.getRawResidualCapacity() == 100);
  t.setPolarityReversal(true);
  t.pool();
  assert(!t.getMainRelayStatus());
}

static void testBatteryInvalidStepDeenergizes() {
  FlprogBatteryTester t;
  fakeMillis = 500;
  t.setVoltage(12000);
  t.setCurrent(100);
  t.setCurrentStep(FLPROG_BATTERY_TESTER_HAND_CHARGE_STEP);
  t.pool();
  assert(t.getMainRelayStatus());
  t.setCurrentStep(255);
  t.pool();
  assert(!t.getMainRelayStatus() && !t.getChangeoverRelayStatus());
}

static void testBatteryAutoStages() {
  FlprogBatteryTester t;
  fakeMillis = 0;
  t.setVoltage(12000);
  t.setCurrent(100);
  t.setEndChargeCurrent(20);
  t.setEndChargeTime(2000);
  t.pusk();
  t.pool();
  assert(t.getCurrentStep() == FLPROG_BATTERY_TESTER_INITIAL_CHARGE_STEP);
  assert(t.getMainRelayStatus() && t.getChangeoverRelayStatus());
  fakeMillis = 100;
  t.setCurrent(10);
  t.pool();
  fakeMillis = 2100;
  t.pool();
  assert(t.getCurrentStep() == FLPROG_BATTERY_TESTER_DISCHARGE_STEP);
  assert(t.getMainRelayStatus() && !t.getChangeoverRelayStatus());
  t.stop(); t.pool();
}

static void testSharicStopFromClamping() {
  FlprogSharicAuto m;
  fakeMillis = 100;
  m.setCurrentPosition(-1);
  m.pusk();
  m.pool();
  assert(m.getCurrentMotorMode() == FLPROG_FIND_ZERO_STEP_MOTOR_MODE);
  m.setCurrentMotorStatus(FLPROG_END_FIND_ZERO_STEP_MOTOR_STATUS);
  m.pool();
  assert(m.getOpenClampOutputStatus());
  m.setOpenClampPinStatus(true);
  m.pool();
  assert(m.getOpenPartFeedOutputStatus());
  m.stop();
  assert(m.getCurrentMotorMode() == FLPROG_STOP_STEP_MOTOR_MODE);
  assert(m.getCurrentMotorSpeed() == 0);
  assert(!m.getOpenClampOutputStatus() && !m.getCloseClampOutputStatus());
  assert(!m.getOpenPartFeedOutputStatus() && !m.getClosePartFeedOutputStatus());
  m.stop(); // Idempotent.
  m.pool();
  assert(!m.getOpenPartFeedOutputStatus());
}

static void testSharicStopFromClosedClamp() {
  FlprogSharicAuto m;
  fakeMillis = 500;
  m.setCurrentPosition(-1);
  m.pusk(); m.pool();
  m.setCurrentMotorStatus(FLPROG_END_FIND_ZERO_STEP_MOTOR_STATUS); m.pool();
  m.setOpenClampPinStatus(true); m.pool();
  fakeMillis += 1000;
  m.pool(); // Close clamp command.
  assert(m.getCloseClampOutputStatus());
  m.setCloseClampPinStatus(true); m.pool();
  assert(m.getClosePartFeedOutputStatus());
  m.setWorkMode(false);
  assert(!m.getCloseClampOutputStatus() && !m.getClosePartFeedOutputStatus());
}

int main() {
  testBatteryStopAndManualDischarge();
  testBatteryInt16MinAndOverflow();
  testBatteryRestoreAndMillisWrap();
  testBatteryAutoStages();
  testBatteryInvalidStepDeenergizes();
  testSharicStopFromClamping();
  testSharicStopFromClosedClamp();
  return 0;
}
