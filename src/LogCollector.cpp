#include "LogCollector.h"

#include "DeviceLogConfiguration.h"
#include "DeviceLogProducer.h"

#include "logcollectorconfig.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSocketNotifier>

#include <HemeraCore/CommonOperations>
#include <HemeraCore/Literals>

#include <systemd/sd-journal.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(LOG_LOGCOLLECTOR, "LogCollector")

#define ADD_OPTIONAL_JOURNAL_FIELD(NAME) \
if (sd_journal_get_data(m_journal, NAME, (const void **)&d, &l) >= 0) {\
    message.insert(QStringLiteral(NAME), QLatin1String(QByteArray::fromRawData(d+(strlen(NAME)+1), l-(strlen(NAME)+1))));\
}

LogCollector::LogCollector(Gravity::GalaxyManager *manager, QObject *parent)
    : Hemera::AsyncInitObject(parent)
    , m_deviceLogConfiguration(new DeviceLogConfiguration(this))
    , m_deviceLogProducer(new DeviceLogProducer(this))
    , m_fd(-1)
    , m_journal(nullptr)
{
}

LogCollector::~LogCollector()
{
}

void LogCollector::initImpl()
{
    // Time to open our journal!
    int r = sd_journal_open(&m_journal, SD_JOURNAL_LOCAL_ONLY);
    if (r < 0) {
        setInitError(Hemera::Literals::literal(Hemera::Literals::Errors::failedRequest()),
                     QString::fromLatin1("Could not open journal! %1").arg(QLatin1String(strerror(-r))));
        return;
    }

    configureFilters();
    connect(m_deviceLogConfiguration, &DeviceLogConfiguration::rulesChanged, this, &LogCollector::configureFilters);

    connect(m_deviceLogProducer, &DeviceLogProducer::ready, this, &LogCollector::startLogging);

    setReady();
}

void LogCollector::configureFilters()
{
    QList<QByteArray> rules = m_deviceLogConfiguration->rules();

    // cleanup previous rules
    sd_journal_flush_matches(m_journal);

    if (rules.isEmpty()) {
        // if no rules, do not match anything
        sd_journal_add_match(m_journal, "NO_MATCH_FILTER_KEY=NO_MATCH_FILTER_VALUE", 0);
        return;
    }

    bool needDisjunction = false;
    for (QByteArray rule : rules) {
        if (needDisjunction) {
            sd_journal_add_disjunction(m_journal);
        }
        sd_journal_add_match(m_journal, rule, 0);
        needDisjunction = true;
    }
}

void LogCollector::startLogging()
{
    // Fast forward
    sd_journal_seek_tail(m_journal);
    // Go next, as we don't care about the tail message
    sd_journal_next(m_journal);

    // Ok! Now, we're ready for some action.
    m_fd = sd_journal_get_fd(m_journal);

    QSocketNotifier *notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &LogCollector::readJournal);
}

void LogCollector::readJournal()
{
    // There's data available. Sort it out.
    if (sd_journal_process(m_journal) < 0) {
        qWarning() << "js_journal_process returned < 0";
    }

    while (sd_journal_next(m_journal) > 0) {
        const char *d;
        size_t l;

        int r = sd_journal_get_data(m_journal, "MESSAGE", (const void **)&d, &l);
        if (r != 0) {
            qWarning() << "Error retrieving data!" << QLatin1String(strerror(-r));
        }

        // Get timestamp
        uint64_t timestamp;
        sd_journal_get_realtime_usec(m_journal, &timestamp);
        // In milliseconds, microseconds is a bit too precise.
        timestamp /= 1000;

        uint64_t monotonicUSec;
        sd_id128_t bootId;
        sd_journal_get_monotonic_usec(m_journal, &monotonicUSec, &bootId);

        DeviceLogProducer::Data logEntry;
        logEntry.setTimestamp(QDateTime::fromMSecsSinceEpoch(timestamp));
        logEntry.setMonotonicTimestamp(monotonicUSec);
        logEntry.setBootId(QString::fromLatin1(QByteArray((char *) &bootId, 16).toHex()));
        logEntry.setMessage(QString::fromLatin1(QByteArray::fromRawData(d+(strlen("message")+1), l-(strlen("message")+1))));

        if (sd_journal_get_data(m_journal, "_PID", (const void **)&d, &l) >= 0) {
            QByteArray pidDigits = QByteArray::fromRawData(d+(strlen("_PID")+1), l-(strlen("_PID")+1));
            bool ok;
            int pid = pidDigits.toInt(&ok);
            if (ok) {
                logEntry.setPid(pid);
            }
        }

        if (sd_journal_get_data(m_journal, "_PRIORITY", (const void **)&d, &l) >= 0) {
            QByteArray priorityDigits = QByteArray::fromRawData(d+(strlen("_PRIORITY")+1), l-(strlen("_PRIORITY")+1));
            bool ok;
            int priority = priorityDigits.toInt(&ok);
            if (ok) {
                logEntry.setSeverity(priority);
            } else {
                logEntry.setSeverity(6);
            }
        } else {
            logEntry.setSeverity(6);
        }

        if (sd_journal_get_data(m_journal, "_CMDLINE", (const void **)&d, &l) >= 0) {
            QByteArray cmdLine = QByteArray::fromRawData(d+(strlen("_CMDLINE")+1), l-(strlen("_CMDLINE")+1));
            logEntry.setCmdLine(QString::fromLatin1(cmdLine));
            QString app = m_deviceLogConfiguration->cmdLineToApplication(cmdLine);
            if (!app.isEmpty()) {
                logEntry.setApplicationId(app);
            }
        }

        if (sd_journal_get_data(m_journal, "QT_CATEGORY", (const void **)&d, &l) >= 0) {
            QByteArray category = QByteArray::fromRawData(d+(strlen("QT_CATEGORY")+1), l-(strlen("QT_CATEGORY")+1));
            logEntry.setCategory(QString::fromLatin1(category));
        }

        m_deviceLogProducer->streamData(logEntry);
    }
}
