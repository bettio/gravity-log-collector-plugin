#ifndef _ONDEMANDCONFIGURATION_H_
#define _ONDEMANDCONFIGURATION_H_

#include <QtCore/QHash>
#include <QtCore/QObject>

class ConfigurationConsumer;
class DeviceLogStatusProducer;

class DeviceLogConfiguration : public QObject
{
    Q_OBJECT

    public:
        DeviceLogConfiguration(QObject *parent = nullptr);
        virtual ~DeviceLogConfiguration();

        void setFilterRulesRuleIdFilterKeyValue(const QString &matchValue, const QByteArray &ruleId, const QByteArray &matchKey);
        void unsetFilterRulesRuleIdFilterKeyValue(const QByteArray &ruleId, const QByteArray &matchKey);

        QList<QByteArray> rules() const;

        static QString applicationCmdLine(const QString &hemeraApplication);
        QString cmdLineToApplication(const QByteArray &cmdLine) const;

    signals:
        void rulesChanged();

    private:
        DeviceLogStatusProducer * m_deviceLogStatusProducer;
        ConfigurationConsumer *m_configurationConsumer;
        QHash<QByteArray, QByteArray> m_rules;
        QHash<QByteArray, QString> m_cmdLineToApp;
};

#endif
