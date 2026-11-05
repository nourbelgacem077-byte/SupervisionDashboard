#include <QtTest>

#include "measurementparser.h"

class TestMqttParsing : public QObject
{
    Q_OBJECT

private slots:
    void measurement_data();
    void measurement();
    void measurementValues();
    void measurementUntouchedOnFailure();
    void status_data();
    void status();
    void statusUntouchedOnFailure();
};

// ---------- Mesures : valide ou non ----------

void TestMqttParsing::measurement_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::addColumn<bool>("valid");

    // Cas valides
    QTest::newRow("valid")
        << QByteArray(R"({"ts":1,"voltage":230.4,"current":10.1,"power":2.32,"temperature":55.2})")
        << true;
    QTest::newRow("without ts")
        << QByteArray(R"({"voltage":230.4,"current":10.1,"power":2.32,"temperature":55.2})")
        << true;
    QTest::newRow("extra keys ignored")
        << QByteArray(R"({"voltage":1,"current":2,"power":3,"temperature":4,"foo":"bar"})")
        << true;
    QTest::newRow("integers accepted")
        << QByteArray(R"({"voltage":230,"current":10,"power":2,"temperature":55})")
        << true;
    QTest::newRow("negative values accepted")
        << QByteArray(R"({"voltage":-5.5,"current":-1,"power":-0.2,"temperature":-20})")
        << true;
    QTest::newRow("zero values accepted")
        << QByteArray(R"({"voltage":0,"current":0,"power":0,"temperature":0})")
        << true;

    // Clés manquantes (une par une)
    QTest::newRow("missing voltage")
        << QByteArray(R"({"current":10.1,"power":2.32,"temperature":55.2})")
        << false;
    QTest::newRow("missing current")
        << QByteArray(R"({"voltage":230.4,"power":2.32,"temperature":55.2})")
        << false;
    QTest::newRow("missing power")
        << QByteArray(R"({"voltage":230.4,"current":10.1,"temperature":55.2})")
        << false;
    QTest::newRow("missing temperature")
        << QByteArray(R"({"voltage":230.4,"current":10.1,"power":2.32})")
        << false;
    QTest::newRow("empty object")
        << QByteArray("{}")
        << false;

    // Mauvais types
    QTest::newRow("string value")
        << QByteArray(R"({"voltage":"abc","current":10.1,"power":2.32,"temperature":55.2})")
        << false;
    QTest::newRow("numeric string")
        << QByteArray(R"({"voltage":"230.4","current":10.1,"power":2.32,"temperature":55.2})")
        << false;
    QTest::newRow("null value")
        << QByteArray(R"({"voltage":null,"current":10.1,"power":2.32,"temperature":55.2})")
        << false;
    QTest::newRow("bool value")
        << QByteArray(R"({"voltage":true,"current":10.1,"power":2.32,"temperature":55.2})")
        << false;

    // JSON cassé ou de mauvaise forme
    QTest::newRow("invalid json")
        << QByteArray(R"({"voltage":)")
        << false;
    QTest::newRow("not json")
        << QByteArray("hello")
        << false;
    QTest::newRow("json array")
        << QByteArray("[1,2,3]")
        << false;
    QTest::newRow("json number")
        << QByteArray("42")
        << false;
    QTest::newRow("empty message")
        << QByteArray()
        << false;
}

void TestMqttParsing::measurement()
{
    QFETCH(QByteArray, json);
    QFETCH(bool, valid);

    Measurement m;
    QCOMPARE(parseMeasurement(json, m), valid);
}

// ---------- Mesures : les valeurs arrivent dans les bons champs ----------

void TestMqttParsing::measurementValues()
{
    // Quatre valeurs toutes différentes : une confusion entre champs se verrait
    const QByteArray json =
        R"({"ts":1761120000000,"voltage":231.5,"current":9.75,"power":2.25,"temperature":57.5})";

    Measurement m;
    QVERIFY(parseMeasurement(json, m));

    QCOMPARE(m.voltage, 231.5);
    QCOMPARE(m.current, 9.75);
    QCOMPARE(m.power, 2.25);
    QCOMPARE(m.temperature, 57.5);
}

void TestMqttParsing::measurementUntouchedOnFailure()
{
    // En cas d'échec, la structure ne doit pas être modifiée
    Measurement m;
    m.voltage = 1.0;
    m.current = 2.0;
    m.power = 3.0;
    m.temperature = 4.0;

    // Cette mesure a un voltage valide mais pas de "power" : refusée
    const QByteArray json =
        R"({"voltage":999,"current":999,"temperature":999})";
    QVERIFY(!parseMeasurement(json, m));

    QCOMPARE(m.voltage, 1.0);
    QCOMPARE(m.current, 2.0);
    QCOMPARE(m.power, 3.0);
    QCOMPARE(m.temperature, 4.0);
}

// ---------- Status ----------

void TestMqttParsing::status_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<bool>("expectedOnline");

    QTest::newRow("online")            << QByteArray("online")      << true  << true;
    QTest::newRow("offline")           << QByteArray("offline")     << true  << false;
    QTest::newRow("online + newline")  << QByteArray("online\n")    << true  << true;
    QTest::newRow("offline + spaces")  << QByteArray("  offline ")  << true  << false;

    QTest::newRow("unknown text")      << QByteArray("banana")      << false << false;
    QTest::newRow("uppercase")         << QByteArray("ONLINE")      << false << false;
    QTest::newRow("empty")             << QByteArray()              << false << false;
    QTest::newRow("only spaces")       << QByteArray("   ")         << false << false;
    QTest::newRow("json instead")      << QByteArray(R"({"status":"online"})") << false << false;
}

void TestMqttParsing::status()
{
    QFETCH(QByteArray, text);
    QFETCH(bool, valid);
    QFETCH(bool, expectedOnline);

    bool online = !expectedOnline;

    QCOMPARE(parseStatus(text, online), valid);
    if (valid) {
        QCOMPARE(online, expectedOnline);
    }
}

void TestMqttParsing::statusUntouchedOnFailure()
{
    bool online = true;
    QVERIFY(!parseStatus("banana", online));
    QVERIFY(online);        // inchangé

    online = false;
    QVERIFY(!parseStatus("", online));
    QVERIFY(!online);       // inchangé
}

QTEST_APPLESS_MAIN(TestMqttParsing)
#include "tst_mqttparsing.moc"