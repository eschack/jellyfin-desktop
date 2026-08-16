#include "MpvConfiguration.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <mpv/client.h>

namespace MpvConfiguration
{

void loadProfile(const QString& profileDataDir,
                 const LoadFunction& loadConfig,
                 const LoadFunction& loadScript)
{
  const QFileInfo configFile(QDir(profileDataDir).filePath("mpv.conf"));

  if (configFile.isFile())
  {
    const QByteArray configPath = configFile.absoluteFilePath().toUtf8();
    const int result = loadConfig(configPath);
    if (result < 0)
      qWarning() << "Failed to load mpv configuration:" << configFile.absoluteFilePath() << mpv_error_string(result);
    else
      qInfo() << "Loaded mpv configuration:" << configFile.absoluteFilePath();
  }
  else
  {
    qDebug() << "No profile mpv configuration found at:" << configFile.absoluteFilePath();
  }

  const QDir scriptsDir(QDir(profileDataDir).filePath("scripts"));
  const QFileInfoList scripts = scriptsDir.entryInfoList(
    QStringList() << "*.lua",
    QDir::Files | QDir::Readable,
    QDir::Name | QDir::IgnoreCase);

  for (const QFileInfo& script : scripts)
  {
    const QByteArray scriptPath = script.absoluteFilePath().toUtf8();
    const int result = loadScript(scriptPath);
    if (result < 0)
      qWarning() << "Failed to load mpv script:" << script.absoluteFilePath() << mpv_error_string(result);
    else
      qInfo() << "Loaded mpv script:" << script.absoluteFilePath();
  }
}

} // namespace MpvConfiguration
