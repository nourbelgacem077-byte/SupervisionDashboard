
#include <QSignalSpy>
#include <QTest>

#include "alarmmanager.h"

    class TestAlarmManager : public QObject
{
    Q_OBJECT

private:

    static int lv(AlarmLevel level) { return static_cast<int>(level); }
    static int count(const AlarmManager &m) { return static_cast<int>(m.events().size()); }
    static int count(const QSignalSpy &s) { return static_cast<int>(s.count()); }

    // Common rule: warning at 250, critical at 270, hysteresis of 3
    static void addVoltageRule(AlarmManager &m) { m.addRule("Voltage", 250.0, 270.0, 3.0); }

private slots:
    void unknownQuantityIsIgnored();
    void noEventBelowThreshold();
    void warningRaised();
    void criticalRaised();
    void noDuplicateWhileStayingWarning();
    void escalationCreatesSecondEvent();
    void criticalToWarningCreatesEvent();
    void returnToNormalCreatesNoEvent();
    void independentRules();
    void signalsEmitted();
    void acknowledgeEvent();
    void invalidAcknowledgeIndex();
};


void TestAlarmManager::unknownQuantityIsIgnored()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Humidity", 999.0);

    QCOMPARE(count(manager), 0);
}

// Below the threshold: no event is created
void TestAlarmManager::noEventBelowThreshold()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 220.0);

    QCOMPARE(count(manager), 0);
    QCOMPARE(manager.activeCount(), 0);
}

// Warning threshold exceeded: one complete event is created
void TestAlarmManager::warningRaised()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);

    QCOMPARE(count(manager), 1);
    const AlarmEvent &e = manager.events().at(0);
    QCOMPARE(lv(e.level), lv(AlarmLevel::Warning));
    QCOMPARE(e.quantity, QString("Voltage"));
    QCOMPARE(e.value, 251.0);
    QCOMPARE(e.acknowledged, false);
    QVERIFY(e.time.isValid());
}

// Critical threshold exceeded: one Critical event is created
void TestAlarmManager::criticalRaised()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 271.0);

    QCOMPARE(count(manager), 1);
    QCOMPARE(lv(manager.events().at(0).level), lv(AlarmLevel::Critical));
}

// Remaining in the Warning state does not create a new alarm
void TestAlarmManager::noDuplicateWhileStayingWarning()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);
    manager.processMeasurement("Voltage", 252.0);
    manager.processMeasurement("Voltage", 249.0);   // Hysteresis: still in Warning state

    QCOMPARE(count(manager), 1);
}

// Escalating from Warning to Critical creates a second event
void TestAlarmManager::escalationCreatesSecondEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);
    manager.processMeasurement("Voltage", 275.0);

    QCOMPARE(count(manager), 2);
    QCOMPARE(lv(manager.events().at(0).level), lv(AlarmLevel::Warning));
    QCOMPARE(lv(manager.events().at(1).level), lv(AlarmLevel::Critical));
}

// Transitioning from Critical to Warning: the condition is still abnormal, so an event is created
void TestAlarmManager::criticalToWarningCreatesEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 275.0);   // Critical
    manager.processMeasurement("Voltage", 262.0);   // Below 267: returns to Warning

    QCOMPARE(count(manager), 2);
    QCOMPARE(lv(manager.events().at(1).level), lv(AlarmLevel::Warning));
}

// Returning to Normal does not create an alarm
void TestAlarmManager::returnToNormalCreatesNoEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);   // Warning: 1 event
    manager.processMeasurement("Voltage", 245.0);   // Below 247: returns to Normal

    QCOMPARE(count(manager), 1);
}

// Each measurement quantity has its own rule and state
void TestAlarmManager::independentRules()
{
    AlarmManager manager;
    manager.addRule("Voltage", 250.0, 270.0, 3.0);
    manager.addRule("Temperature", 75.0, 90.0, 5.0);

    manager.processMeasurement("Voltage", 251.0);       // Warning for Voltage
    manager.processMeasurement("Temperature", 70.0);    // Normal for Temperature
    QCOMPARE(count(manager), 1);

    manager.processMeasurement("Temperature", 80.0);    // Warning for Temperature
    QCOMPARE(count(manager), 2);
    QCOMPARE(manager.events().at(1).quantity, QString("Temperature"));
}

// Both signals are emitted once for each new alarm
void TestAlarmManager::signalsEmitted()
{
    AlarmManager manager;
    addVoltageRule(manager);

    QSignalSpy raised(&manager, &AlarmManager::alarmRaised);
    QSignalSpy changed(&manager, &AlarmManager::eventsChanged);

    manager.processMeasurement("Voltage", 251.0);

    QCOMPARE(count(raised), 1);
    QCOMPARE(count(changed), 1);

    manager.processMeasurement("Voltage", 252.0);   // No new event

    QCOMPARE(count(raised), 1);
    QCOMPARE(count(changed), 1);
}

// Acknowledging an alarm makes it no longer active
void TestAlarmManager::acknowledgeEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);
    manager.processMeasurement("Voltage", 271.0);
    QCOMPARE(manager.activeCount(), 1);

    QSignalSpy changed(&manager, &AlarmManager::eventsChanged);
    manager.acknowledge(0);

    QCOMPARE(manager.activeCount(), 0);
    QVERIFY(manager.events().at(0).acknowledged);
    QCOMPARE(count(changed), 1);
}

// An invalid index has no effect: no crash and no signal emitted
void TestAlarmManager::invalidAcknowledgeIndex()
{
    AlarmManager manager;
    addVoltageRule(manager);
    manager.processMeasurement("Voltage", 251.0);

    QSignalSpy changed(&manager, &AlarmManager::eventsChanged);   // Created after the alarm
    manager.acknowledge(5);
    manager.acknowledge(-1);

    QCOMPARE(count(manager), 1);
    QCOMPARE(manager.activeCount(), 1);
    QCOMPARE(count(changed), 0);
}

QTEST_APPLESS_MAIN(TestAlarmManager)
#include "tst_alarmmanager.moc"
