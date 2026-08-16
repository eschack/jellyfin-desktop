#include <QtTest/QtTest>

#include "../src/player/MpvConfiguration.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

class TestMpvConfiguration : public QObject
{
  Q_OBJECT

private slots:
  void loadsConfigAndScriptsInStableOrder();
  void ignoresMissingAndInvalidEntries();
  void continuesLoadingAfterAnError();
};

static void createFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    qFatal("Failed to create test file: %s", qPrintable(path));
  file.write("test");
}

void TestMpvConfiguration::loadsConfigAndScriptsInStableOrder()
{
  QTemporaryDir profileDir;
  QVERIFY(profileDir.isValid());

  const QString configPath = profileDir.filePath("mpv.conf");
  const QString scriptsPath = profileDir.filePath("scripts");
  QVERIFY(QDir().mkpath(scriptsPath));

  createFile(configPath);
  createFile(QDir(scriptsPath).filePath("beta.lua"));
  createFile(QDir(scriptsPath).filePath("Alpha.lua"));
  createFile(QDir(scriptsPath).filePath("ignored.txt"));
  QVERIFY(QDir().mkpath(QDir(scriptsPath).filePath("nested")));
  createFile(QDir(scriptsPath).filePath("nested/ignored.lua"));

  QList<QByteArray> configs;
  QList<QByteArray> scripts;
  QStringList loadOrder;
  MpvConfiguration::loadProfile(
    profileDir.path(),
    [&configs, &loadOrder](const QByteArray& path) {
      configs.append(path);
      loadOrder.append("config");
      return 0;
    },
    [&scripts, &loadOrder](const QByteArray& path) {
      scripts.append(path);
      loadOrder.append("script");
      return 0;
    });

  QCOMPARE(loadOrder, QStringList({"config", "script", "script"}));
  QCOMPARE(configs, QList<QByteArray>({QFileInfo(configPath).absoluteFilePath().toUtf8()}));
  QCOMPARE(scripts, QList<QByteArray>({
    QFileInfo(QDir(scriptsPath).filePath("Alpha.lua")).absoluteFilePath().toUtf8(),
    QFileInfo(QDir(scriptsPath).filePath("beta.lua")).absoluteFilePath().toUtf8()
  }));
}

void TestMpvConfiguration::ignoresMissingAndInvalidEntries()
{
  QTemporaryDir profileDir;
  QVERIFY(profileDir.isValid());

  QVERIFY(QDir().mkpath(profileDir.filePath("mpv.conf")));
  QVERIFY(QDir().mkpath(profileDir.filePath("scripts/not-a-file.lua")));

  int configLoads = 0;
  int scriptLoads = 0;
  MpvConfiguration::loadProfile(
    profileDir.path(),
    [&configLoads](const QByteArray&) {
      ++configLoads;
      return 0;
    },
    [&scriptLoads](const QByteArray&) {
      ++scriptLoads;
      return 0;
    });

  QCOMPARE(configLoads, 0);
  QCOMPARE(scriptLoads, 0);
}

void TestMpvConfiguration::continuesLoadingAfterAnError()
{
  QTemporaryDir profileDir;
  QVERIFY(profileDir.isValid());

  const QString scriptsPath = profileDir.filePath("scripts");
  QVERIFY(QDir().mkpath(scriptsPath));
  createFile(profileDir.filePath("mpv.conf"));
  createFile(QDir(scriptsPath).filePath("first.lua"));
  createFile(QDir(scriptsPath).filePath("second.lua"));

  int configLoads = 0;
  int scriptLoads = 0;
  MpvConfiguration::loadProfile(
    profileDir.path(),
    [&configLoads](const QByteArray&) {
      ++configLoads;
      return -1;
    },
    [&scriptLoads](const QByteArray&) {
      ++scriptLoads;
      return -1;
    });

  QCOMPARE(configLoads, 1);
  QCOMPARE(scriptLoads, 2);
}

QTEST_APPLESS_MAIN(TestMpvConfiguration)
#include "test_mpvconfiguration.moc"
