#include "DeviceLogConfiguration.h"

#include "configurationconsumer.h"
#include "DeviceLogStatusProducer.h"

#include <QtCore/QFile>

DeviceLogConfiguration::DeviceLogConfiguration(QObject *parent)
    : QObject(parent)
    , m_deviceLogStatusProducer(new DeviceLogStatusProducer(this))
    , m_configurationConsumer(new ConfigurationConsumer(this))
{
}

DeviceLogConfiguration::~DeviceLogConfiguration()
{
}

void DeviceLogConfiguration::setFilterRulesRuleIdFilterKeyValue(const QString &matchValue, const QByteArray &ruleId, const QByteArray &matchKey)
{
    QByteArray previousMatchKey = m_rules.value(ruleId).split('=').first();

    if (matchKey == "HEMERA_APPLICATION") {
        QByteArray appCmdLine = applicationCmdLine(matchValue).toLatin1();
        if (!appCmdLine.isEmpty()) {
            m_rules.insert(ruleId, QByteArray("_CMDLINE=%1").replace("%1", appCmdLine));
            m_cmdLineToApp.insert(appCmdLine, matchValue);
        } else {
            return;
        }
    } else {
        m_rules.insert(ruleId, QByteArray("%1=%2").replace("%1", matchKey).replace("%2", matchValue.toLatin1()));
    }

    if (!previousMatchKey.isEmpty()) {
        m_deviceLogStatusProducer->unsetFilterRulesRuleIdFilterKeyValue(ruleId, previousMatchKey);
        if (previousMatchKey == "_CMDLINE") {
           m_deviceLogStatusProducer->unsetFilterRulesRuleIdFilterKeyValue(ruleId, "HEMERA_APPLICATION");
        }
    }
    m_deviceLogStatusProducer->setFilterRulesRuleIdFilterKeyValue(matchValue, ruleId, matchKey);

    emit rulesChanged();
}

void DeviceLogConfiguration::unsetFilterRulesRuleIdFilterKeyValue(const QByteArray &ruleId, const QByteArray &matchKey)
{
    m_rules.remove(ruleId);
    m_deviceLogStatusProducer->unsetFilterRulesRuleIdFilterKeyValue(ruleId, matchKey);

    emit rulesChanged();
}

QList<QByteArray> DeviceLogConfiguration::rules() const
{
    return m_rules.values();
}

QString DeviceLogConfiguration::applicationCmdLine(const QString &hemeraApplication)
{
    if (hemeraApplication.contains(QLatin1Char('/'))) {
        // hemera application name is not valid
        return QString();
    }

    // We have to find now the executable for the application. We cannot use _SYSTEMD_UNIT as it is not set for user units.
    // So, the only helpful parameter we can "easily" grab is the executable path.
    QFile service(QStringLiteral("/usr/lib/systemd/user/%1.service").arg(hemeraApplication));
    service.open(QIODevice::ReadOnly);
    QString cmdline;
    while (!service.atEnd()) {
        QByteArray line = service.readLine();
        if (line.startsWith("ExecStart=")) {
            service.close();
            // Trim away start and ending newline.
            return QLatin1String(line.mid(strlen("ExecStart="), line.length() - strlen("ExecStart=") - 1));
        }
    }
    service.close();

    return QString();
}

QString DeviceLogConfiguration::cmdLineToApplication(const QByteArray &cmdLine) const
{
    return m_cmdLineToApp.value(cmdLine);
}
