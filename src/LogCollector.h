#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <HemeraCore/AsyncInitObject>

namespace Gravity {
class GalaxyManager;
}

class DeviceLogConfiguration;
class DeviceLogProducer;
struct sd_journal;

class LogCollector : public Hemera::AsyncInitObject
{
    public:
        explicit LogCollector(Gravity::GalaxyManager *manager, QObject *parent = nullptr);
        virtual ~LogCollector();

    protected:
        virtual void initImpl() override final;

    private slots:
        void configureFilters();
        void startLogging();
        void readJournal();

    private:
        DeviceLogConfiguration *m_deviceLogConfiguration;
        DeviceLogProducer *m_deviceLogProducer;
        int m_fd;
        sd_journal *m_journal;
};

#endif
