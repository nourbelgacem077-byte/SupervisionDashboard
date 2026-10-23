
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include "appsettings.h"

    class TestAppSettings : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;   // Temporary directory: removed when the tests finish

    // Path to the settings file used by AppSettings
    static QString settingsFile()
    {
        QSettings probe(QSettings::IniFormat, QSettings::UserScope,
                        "SupervisionDashboard", "settings");
        return probe.fileName();
    }

    static int count(const QSignalSpy &s) { return static_cast<int>(s.count()); }

private slots:
    void initTestCase();
    void init();

    void defaultsOnFirstRun();
    void quantitiesList();
    void unknownQuantityGetters();
    void acceptValidThresholds();
    void otherQuantitiesUnaffected();
    void rejectWarningAboveCritical();
    void rejectWarningEqualsCritical();
    void rejectNegativeHysteresis();
    void acceptZeroHysteresis();
    void rejectUnknownQuantity();
    void persistsAcrossInstances();
    void resetDefaultsRestores();
    void corruptedFileFallsBack();
    void portTooLowIsRejected();
    void portTooHighIsRejected();
    void emptyHostIsRejected();
    void unknownTypeIsRejected();
    void portLimitsAreAccepted();

};

// Runs once before all tests
void TestAppSettings::initTestCase()
{
    QVERIFY(m_dir.isValid());

    // Redirect ALL QSettings instances used by the test to the temporary directory
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_dir.path());

    // Safety check: if redirection failed, stop BEFORE deleting any real settings file
    const QString file = QDir::fromNativeSeparators(settingsFile());
    const QString base = QDir::fromNativeSeparators(m_dir.path());
    QVERIFY2(file.startsWith(base, Qt::CaseInsensitive),
             qPrintable("Settings redirection failed: " + file));
}

// Before EACH test: start with an empty settings file
void TestAppSettings::init()
{
    QFile::remove(settingsFile());
}

// First run: default values for all four quantities
void TestAppSettings::defaultsOnFirstRun()
{
    AppSettings s;

    QCOMPARE(s.warning("Tension"), 250.0);
    QCOMPARE(s.critical("Tension"), 260.0);
    QCOMPARE(s.hysteresis("Tension"), 3.0);

    QCOMPARE(s.warning("Courant"), 14.0);
    QCOMPARE(s.critical("Courant"), 16.0);
    QCOMPARE(s.hysteresis("Courant"), 0.5);

    QCOMPARE(s.warning("Puissance"), 3.2);
    QCOMPARE(s.critical("Puissance"), 3.7);
    QCOMPARE(s.hysteresis("Puissance"), 0.1);

    QCOMPARE(s.warning("Température"), 75.0);
    QCOMPARE(s.critical("Température"), 90.0);
    QCOMPARE(s.hysteresis("Température"), 5.0);
}

// The four names, in display order
void TestAppSettings::quantitiesList()
{
    AppSettings s;
    const QStringList expected = {"Tension", "Courant", "Puissance", "Température"};
    QCOMPARE(s.quantities(), expected);
}

// An unknown quantity returns 0 (no error)
void TestAppSettings::unknownQuantityGetters()
{
    AppSettings s;
    QCOMPARE(s.warning("Humidité"), 0.0);
    QCOMPARE(s.critical("Humidité"), 0.0);
    QCOMPARE(s.hysteresis("Humidité"), 0.0);
}

// Valid threshold values are accepted, saved, and reported through a signal
void TestAppSettings::acceptValidThresholds()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 2.0));

    QCOMPARE(s.warning("Tension"), 245.0);
    QCOMPARE(s.critical("Tension"), 255.0);
    QCOMPARE(s.hysteresis("Tension"), 2.0);
    QCOMPARE(count(spy), 1);
}

// Changing one quantity does not affect the others
void TestAppSettings::otherQuantitiesUnaffected()
{
    AppSettings s;
    QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 2.0));

    QCOMPARE(s.warning("Courant"), 14.0);
    QCOMPARE(s.critical("Courant"), 16.0);
    QCOMPARE(s.warning("Température"), 75.0);
}

// Warning greater than Critical: rejected, no changes, no signal
void TestAppSettings::rejectWarningAboveCritical()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(!s.setThresholds("Tension", 260.0, 250.0, 2.0));

    QCOMPARE(s.warning("Tension"), 250.0);
    QCOMPARE(s.critical("Tension"), 260.0);
    QCOMPARE(s.hysteresis("Tension"), 3.0);
    QCOMPARE(count(spy), 0);
}

// Boundary case: Warning equal to Critical is also rejected
void TestAppSettings::rejectWarningEqualsCritical()
{
    AppSettings s;
    QVERIFY(!s.setThresholds("Tension", 255.0, 255.0, 2.0));
    QCOMPARE(s.warning("Tension"), 250.0);
}

// Negative hysteresis is rejected
void TestAppSettings::rejectNegativeHysteresis()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(!s.setThresholds("Tension", 245.0, 255.0, -1.0));

    QCOMPARE(s.hysteresis("Tension"), 3.0);
    QCOMPARE(count(spy), 0);
}

// Boundary case: zero hysteresis is valid
void TestAppSettings::acceptZeroHysteresis()
{
    AppSettings s;
    QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 0.0));
    QCOMPARE(s.hysteresis("Tension"), 0.0);
}

// An unknown quantity is rejected without emitting a signal
void TestAppSettings::rejectUnknownQuantity()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(!s.setThresholds("Humidité", 10.0, 20.0, 1.0));
    QCOMPARE(count(spy), 0);
}

// A new object created later reads the values back from the file
void TestAppSettings::persistsAcrossInstances()
{
    {
        AppSettings first;
        QVERIFY(first.setThresholds("Courant", 13.0, 15.0, 1.0));
    }   // first is destroyed: only the file retains the values

    AppSettings second;
    QCOMPARE(second.warning("Courant"), 13.0);
    QCOMPARE(second.critical("Courant"), 15.0);
    QCOMPARE(second.hysteresis("Courant"), 1.0);
    QCOMPARE(second.warning("Tension"), 250.0);   // Unchanged
}

// resetDefaults: restores original values, emits ONE signal, and saves the changes
void TestAppSettings::resetDefaultsRestores()
{
    {
        AppSettings s;
        QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 2.0));
        QVERIFY(s.setThresholds("Courant", 13.0, 15.0, 1.0));

        QSignalSpy spy(&s, &AppSettings::thresholdsChanged);
        s.resetDefaults();

        QCOMPARE(count(spy), 1);   // One signal for all four quantities
        QCOMPARE(s.warning("Tension"), 250.0);
        QCOMPARE(s.warning("Courant"), 14.0);
    }

    AppSettings reread;   // The reset values have been written to the file
    QCOMPARE(reread.warning("Tension"), 250.0);
    QCOMPARE(reread.warning("Courant"), 14.0);
}

// A settings file manually modified with invalid values: falls back to defaults
void TestAppSettings::corruptedFileFallsBack()
{
    {
        QSettings raw(QSettings::IniFormat, QSettings::UserScope,
                      "SupervisionDashboard", "settings");
        raw.setValue("alarms/voltage/warning", 300.0);    // Greater than Critical
        raw.setValue("alarms/voltage/critical", 200.0);
        raw.setValue("alarms/voltage/hysteresis", 3.0);
        raw.sync();
    }

    AppSettings s;
    QCOMPARE(s.warning("Tension"), 250.0);
    QCOMPARE(s.critical("Tension"), 260.0);
}
void TestAppSettings::portTooLowIsRejected()
{
    AppSettings s;
    QVERIFY(!s.setSource("simulator", "localhost", 0));
}

void TestAppSettings::portTooHighIsRejected()
{
    AppSettings s;
    QVERIFY(!s.setSource("simulator", "localhost", 70000));
}

void TestAppSettings::emptyHostIsRejected()
{
    AppSettings s;
    QVERIFY(!s.setSource("mqtt", "   ", 1883));
}

void TestAppSettings::unknownTypeIsRejected()
{
    AppSettings s;
    QVERIFY(!s.setSource("foo", "localhost", 1883));
}

void TestAppSettings::portLimitsAreAccepted()
{
    AppSettings s;
    QVERIFY(s.setSource("mqtt", "localhost", 1));
    QVERIFY(s.setSource("mqtt", "localhost", 65535));
}
QTEST_GUILESS_MAIN(TestAppSettings)
#include "tst_appsettings.moc"
