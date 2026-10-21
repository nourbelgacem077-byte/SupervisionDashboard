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
    QTemporaryDir m_dir;   // dossier temporaire : supprimé à la fin des tests

    // chemin du fichier de réglages utilisé par AppSettings
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
};

// Une seule fois, avant tous les tests
void TestAppSettings::initTestCase()
{
    QVERIFY(m_dir.isValid());

    // redirige TOUS les QSettings du test vers le dossier temporaire
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_dir.path());

    // sécurité : si la redirection a échoué, on s'arrête AVANT de supprimer un vrai fichier
    const QString file = QDir::fromNativeSeparators(settingsFile());
    const QString base = QDir::fromNativeSeparators(m_dir.path());
    QVERIFY2(file.startsWith(base, Qt::CaseInsensitive),
             qPrintable("Redirection des reglages echouee : " + file));
}

// Avant CHAQUE test : on repart d'un fichier de réglages vide
void TestAppSettings::init()
{
    QFile::remove(settingsFile());
}

// Premier lancement : valeurs par défaut des quatre grandeurs
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

// Les quatre noms, dans l'ordre d'affichage
void TestAppSettings::quantitiesList()
{
    AppSettings s;
    const QStringList expected = {"Tension", "Courant", "Puissance", "Température"};
    QCOMPARE(s.quantities(), expected);
}

// Une grandeur inconnue donne 0 (aucune erreur)
void TestAppSettings::unknownQuantityGetters()
{
    AppSettings s;
    QCOMPARE(s.warning("Humidité"), 0.0);
    QCOMPARE(s.critical("Humidité"), 0.0);
    QCOMPARE(s.hysteresis("Humidité"), 0.0);
}

// Des valeurs cohérentes sont acceptées, enregistrées, et annoncées par un signal
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

// Modifier une grandeur ne touche pas aux autres
void TestAppSettings::otherQuantitiesUnaffected()
{
    AppSettings s;
    QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 2.0));

    QCOMPARE(s.warning("Courant"), 14.0);
    QCOMPARE(s.critical("Courant"), 16.0);
    QCOMPARE(s.warning("Température"), 75.0);
}

// Warning supérieur à Critical : refusé, rien ne change, aucun signal
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

// Cas limite : Warning égal à Critical est aussi refusé
void TestAppSettings::rejectWarningEqualsCritical()
{
    AppSettings s;
    QVERIFY(!s.setThresholds("Tension", 255.0, 255.0, 2.0));
    QCOMPARE(s.warning("Tension"), 250.0);
}

// Une hystérésis négative est refusée
void TestAppSettings::rejectNegativeHysteresis()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(!s.setThresholds("Tension", 245.0, 255.0, -1.0));

    QCOMPARE(s.hysteresis("Tension"), 3.0);
    QCOMPARE(count(spy), 0);
}

// Cas limite : une hystérésis de 0 est valide
void TestAppSettings::acceptZeroHysteresis()
{
    AppSettings s;
    QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 0.0));
    QCOMPARE(s.hysteresis("Tension"), 0.0);
}

// Une grandeur inconnue est refusée, sans signal
void TestAppSettings::rejectUnknownQuantity()
{
    AppSettings s;
    QSignalSpy spy(&s, &AppSettings::thresholdsChanged);

    QVERIFY(!s.setThresholds("Humidité", 10.0, 20.0, 1.0));
    QCOMPARE(count(spy), 0);
}

// Un autre objet, créé plus tard, relit les valeurs depuis le fichier
void TestAppSettings::persistsAcrossInstances()
{
    {
        AppSettings first;
        QVERIFY(first.setThresholds("Courant", 13.0, 15.0, 1.0));
    }   // first est détruit : seul le fichier garde la valeur

    AppSettings second;
    QCOMPARE(second.warning("Courant"), 13.0);
    QCOMPARE(second.critical("Courant"), 15.0);
    QCOMPARE(second.hysteresis("Courant"), 1.0);
    QCOMPARE(second.warning("Tension"), 250.0);   // inchangée
}

// resetDefaults : valeurs d'origine, UN SEUL signal, et sauvegarde
void TestAppSettings::resetDefaultsRestores()
{
    {
        AppSettings s;
        QVERIFY(s.setThresholds("Tension", 245.0, 255.0, 2.0));
        QVERIFY(s.setThresholds("Courant", 13.0, 15.0, 1.0));

        QSignalSpy spy(&s, &AppSettings::thresholdsChanged);
        s.resetDefaults();

        QCOMPARE(count(spy), 1);   // un signal pour les quatre grandeurs
        QCOMPARE(s.warning("Tension"), 250.0);
        QCOMPARE(s.warning("Courant"), 14.0);
    }

    AppSettings reread;   // la remise à zéro a bien été écrite dans le fichier
    QCOMPARE(reread.warning("Tension"), 250.0);
    QCOMPARE(reread.warning("Courant"), 14.0);
}

// Un fichier modifié à la main avec des valeurs absurdes : retour aux défauts
void TestAppSettings::corruptedFileFallsBack()
{
    {
        QSettings raw(QSettings::IniFormat, QSettings::UserScope,
                      "SupervisionDashboard", "settings");
        raw.setValue("alarms/voltage/warning", 300.0);    // supérieur à critical
        raw.setValue("alarms/voltage/critical", 200.0);
        raw.setValue("alarms/voltage/hysteresis", 3.0);
        raw.sync();
    }

    AppSettings s;
    QCOMPARE(s.warning("Tension"), 250.0);
    QCOMPARE(s.critical("Tension"), 260.0);
}

QTEST_GUILESS_MAIN(TestAppSettings)
#include "tst_appsettings.moc"