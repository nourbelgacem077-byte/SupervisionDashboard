#include <QTest>

#include "alarmrule.h"

class TestAlarmRule : public QObject
{
    Q_OBJECT

private:
    // convertit en entier : Qt Test affichera des valeurs lisibles en cas d'échec
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

// Seuils communs : warning 250, critical 260, hystérésis 3.
// Zone de retour de Warning : sous 247 (250 - 3).
// Zone de retour de Critical : sous 257 (260 - 3).

void TestAlarmRule::startsNormal()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.level()), lv(AlarmLevel::Normal));
}

void TestAlarmRule::belowWarningStaysNormal()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(245.0)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::warningAtThreshold()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    // le seuil lui-même est inclus (>=)
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
    // 249 est sous le seuil mais au-dessus de 247 : on reste en Warning
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
    // exactement 247 : pas encore "sous" la limite, on reste en Warning
    QCOMPARE(lv(rule.evaluate(247.0)), lv(AlarmLevel::Warning));
    // juste en dessous : retour à Normal
    QCOMPARE(lv(rule.evaluate(246.99)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::jumpsStraightToCritical()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    // depuis Normal, on peut sauter directement à Critical
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
    // 256 est sous 257 mais au-dessus de 247 : on redescend à Warning
    QCOMPARE(lv(rule.evaluate(256.0)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::criticalHysteresisBoundary()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
    // exactement 257 : on reste en Critical
    QCOMPARE(lv(rule.evaluate(257.0)), lv(AlarmLevel::Critical));
    // juste en dessous : Warning
    QCOMPARE(lv(rule.evaluate(256.99)), lv(AlarmLevel::Warning));
}

void TestAlarmRule::criticalToNormalDirect()
{
    AlarmRule rule(250.0, 260.0, 3.0);
    QCOMPARE(lv(rule.evaluate(265.0)), lv(AlarmLevel::Critical));
    // une chute brutale sous 247 ramène directement à Normal
    QCOMPARE(lv(rule.evaluate(240.0)), lv(AlarmLevel::Normal));
}

void TestAlarmRule::fullSequence()
{
    AlarmRule rule(250.0, 260.0, 3.0);   // UNE seule règle pour toute la séquence

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
                 qPrintable(QString("etape %1 : valeur %2, obtenu %3, attendu %4")
                                .arg(i).arg(values[i]).arg(got).arg(lv(expected[i]))));
    }
}

QTEST_APPLESS_MAIN(TestAlarmRule)
#include "tst_alarmrule.moc"