#ifndef MPVCONFIGURATION_H
#define MPVCONFIGURATION_H

#include <QByteArray>
#include <QString>

#include <functional>

namespace MpvConfiguration
{

using LoadFunction = std::function<int(const QByteArray&)>;

void loadProfile(const QString& profileDataDir,
                 const LoadFunction& loadConfig,
                 const LoadFunction& loadScript);

} // namespace MpvConfiguration

#endif // MPVCONFIGURATION_H
