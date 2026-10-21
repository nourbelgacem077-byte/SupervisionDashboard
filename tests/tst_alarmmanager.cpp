#include <QSignalSpy>
#include <QTest>

#include "alarmmanager.h"

class TestAlarmManager : public QObject
{
    Q_OBJECT

private:
    // petits outils : des int pour que Qt Test affiche des valeurs lisibles
    static int lv(AlarmLevel level) { return static_cast<int>(level); }
    static int count(const AlarmManager &m) { return static_cast<int>(m.events().size()); }
    static int count(const QSignalSpy &s) { return static_cast<int>(s.count()); }

    // règle commune : warning 250, critical 270, hystérésis 3
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

// Une grandeur sans règle est ignorée, sans erreur
void TestAlarmManager::unknownQuantityIsIgnored()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Humidity", 999.0);   // aucune règle pour Humidity

    QCOMPARE(count(manager), 0);
}

// Sous le seuil : aucun événement
void TestAlarmManager::noEventBelowThreshold()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 220.0);

    QCOMPARE(count(manager), 0);
    QCOMPARE(manager.activeCount(), 0);
}

// Seuil Warning dépassé : un événement complet
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

// Seuil Critical dépassé : un événement Critical
void TestAlarmManager::criticalRaised()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 271.0);

    QCOMPARE(count(manager), 1);
    QCOMPARE(lv(manager.events().at(0).level), lv(AlarmLevel::Critical));
}

// Rester en Warning ne crée pas de nouvelle alarme
void TestAlarmManager::noDuplicateWhileStayingWarning()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);
    manager.processMeasurement("Voltage", 252.0);
    manager.processMeasurement("Voltage", 249.0);   // hystérésis : toujours Warning

    QCOMPARE(count(manager), 1);
}

// Passer de Warning à Critical crée un second événement
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

// Critical vers Warning : on est toujours en situation anormale, donc un événement
void TestAlarmManager::criticalToWarningCreatesEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 275.0);   // Critical
    manager.processMeasurement("Voltage", 262.0);   // sous 267 : retour à Warning

    QCOMPARE(count(manager), 2);
    QCOMPARE(lv(manager.events().at(1).level), lv(AlarmLevel::Warning));
}

// Retour à Normal : ce n'est pas une alarme
void TestAlarmManager::returnToNormalCreatesNoEvent()
{
    AlarmManager manager;
    addVoltageRule(manager);

    manager.processMeasurement("Voltage", 251.0);   // Warning : 1 événement
    manager.processMeasurement("Voltage", 245.0);   // sous 247 : Normal

    QCOMPARE(count(manager), 1);
}

// Chaque grandeur a sa propre règle et sa propre mémoire
void TestAlarmManager::independentRules()
{
    AlarmManager manager;
    manager.addRule("Voltage", 250.0, 270.0, 3.0);
    manager.addRule("Temperature", 75.0, 90.0, 5.0);

    manager.processMeasurement("Voltage", 251.0);       // Warning pour Voltage
    manager.processMeasurement("Temperature", 70.0);    // Normal pour Temperature
    QCOMPARE(count(manager), 1);

    manager.processMeasurement("Temperature", 80.0);    // Warning pour Temperature
    QCOMPARE(count(manager), 2);
    QCOMPARE(manager.events().at(1).quantity, QString("Temperature"));
}

// Les deux signaux partent une fois par nouvelle alarme
void TestAlarmManager::signalsEmitted()
{
    AlarmManager manager;
    addVoltageRule(manager);

    QSignalSpy raised(&manager, &AlarmManager::alarmRaised);
    QSignalSpy changed(&manager, &AlarmManager::eventsChanged);

    manager.processMeasurement("Voltage", 251.0);

    QCOMPARE(count(raised), 1);
    QCOMPARE(count(changed), 1);

    manager.processMeasurement("Voltage", 252.0);   // rien de nouveau

    QCOMPARE(count(raised), 1);
    QCOMPARE(count(changed), 1);
}

// Acquitter une alarme : elle n'est plus active
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

// Un index invalide ne fait rien : ni plantage, ni signal
void TestAlarmManager::invalidAcknowledgeIndex()
{
    AlarmManager manager;
    addVoltageRule(manager);
    manager.processMeasurement("Voltage", 251.0);

    QSignalSpy changed(&manager, &AlarmManager::eventsChanged);   // créé après l'alarme
    manager.acknowledge(5);
    manager.acknowledge(-1);

    QCOMPARE(count(manager), 1);
    QCOMPARE(manager.activeCount(), 1);
    QCOMPARE(count(changed), 0);
}

QTEST_APPLESS_MAIN(TestAlarmManager)
#include "tst_alarmmanager.moc"