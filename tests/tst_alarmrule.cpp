
#include <QTest>

#include "alarmrule.h"

    class TestAlarmRule : public QObject
{
    Q_OBJECT

private:
    // Converts the level to an integer so Qt Test displays readable values in case of failure
    static int lv(AlarmLevel level) { return static_cast<int>(level); }

private slots:
    void startsNormal();
    void belowWarningStaysNormal();
    void warningAtThreshold();
    void criticalAtThreshold();
    void staysWarningInsideHysteresis();
    void returnsNormalBelowHysteresis();
    void warningHysteresisBoundary();
    void jumpsStraightToCritical();
    void warningToCritical();
    void criticalToWarning();
    void criticalHysteresisBoundary();
    void criticalToNormalDirect();
    void fullSequence();
};

// Common thresholds: warning at 250, critical at 260, hysteresis of 3.
// Warning return threshold: below 247 (250 - 3).
// Critical return threshold: below 257 (260 - 3).

void TestAlarmRule::startsNormal()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.level()), lv(AlarmLevel::Normal));
}

void TestAlarmRule::belowWarningStaysNormal()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(249.0)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::warningAtThreshold()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    // The threshold itself is included (>=)
    QCOMPARE(lv(rule.evaluate(250.0)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::criticalAtThreshold()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(260.0)), lv(AlarmLevel::Critical));
}

void TestAlarmRule::staysWarningInsideHysteresis()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(251.0)), lv(AlarmLevel::Warning));
    // 249 is below the threshold but above 247: the level remains Warning
    QCOMPARE(lv(rule.evaluate(249.0)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::returnsNormalBelowHysteresis()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(251.0)), lv(AlarmLevel::Warning));
    QCOMPARE(lv(rule.evaluate(246.0)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::warningHysteresisBoundary()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(251.0)), lv(AlarmLevel::Warning));
    // Exactly 247: the value is not yet below the limit, so the level remains Warning
    QCOMPARE(lv(rule.evaluate(247.0)), lv(AlarmLevel::Warning));
    // Just below the limit: returns to Normal
    QCOMPARE(lv(rule.evaluate(246.99)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::jumpsStraightToCritical()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    // From Normal, the level can jump directly to Critical
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
}

void TestAlarmRule::warningToCritical()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(251.0)), lv(AlarmLevel::Warning));
    QCOMPARE(lv(rule.evaluate(262.0)), lv(AlarmLevel::Critical));
}

void TestAlarmRule::criticalToWarning()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
    // 256 is below 257 but above 247: the level drops back to Warning
    QCOMPARE(lv(rule.evaluate(256.0)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::criticalHysteresisBoundary()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
    // Exactly 257: the level remains Critical
    QCOMPARE(lv(rule.evaluate(257.0)), lv(AlarmLevel::Critical));
    // Just below the limit: returns to Warning
    QCOMPARE(lv(rule.evaluate(256.99)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::criticalToNormalDirect()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
    // A sudden drop below 247 returns the level directly to Normal
    QCOMPARE(lv(rule.evaluate(240.0)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::fullSequence()
{
    AlarmRule rule(250.0, 260.0, 3.0);   // A SINGLE rule for the entire sequence

    const double values[] = {245.0, 251.0, 249.0, 246.0, 265.0,
                             258.0, 256.0, 246.0, 252.0, 240.0};
    const AlarmLevel expected[] = {
        AlarmLevel::Normal,   AlarmLevel::Warning, AlarmLevel::Warning,
        AlarmLevel::Normal,   AlarmLevel::Critical, AlarmLevel::Critical,
        AlarmLevel::Warning,  AlarmLevel::Normal,   AlarmLevel::Warning,
        AlarmLevel::Normal
    };

    for (int i = 0; i < 10; ++i) {
        const int got = lv(rule.evaluate(values[i]));
        QVERIFY2(got == lv(expected[i]),
                 qPrintable(QString("step %1: value %2, got %3, expected %4")
                                .arg(i).arg(values[i]).arg(got).arg(lv(expected[i]))));
    }
}

QTEST_APPLESS_MAIN(TestAlarmRule)
#include "tst_alarmrule.moc"
