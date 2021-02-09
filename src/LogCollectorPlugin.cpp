/*
 *
 */

#include "LogCollectorPlugin.h"

#include "LogCollector.h"

#include <QtCore/QLoggingCategory>

#include <HemeraCore/CommonOperations>

#include <GravitySupermassive/GalaxyManager>

Q_LOGGING_CATEGORY(LOG_DEVELOPERMODE, "LogCollectorPlugin")

using namespace Gravity;

LogCollectorPlugin::LogCollectorPlugin()
    : Gravity::Plugin()
{
    setName(QStringLiteral("Hemera Developer Mode"));
}

LogCollectorPlugin::~LogCollectorPlugin()
{
}

void LogCollectorPlugin::unload()
{
    qCDebug(LOG_DEVELOPERMODE) << "Log collector plugin unloaded";
    setUnloaded();
}

void LogCollectorPlugin::load()
{
    m_logCollector = new LogCollector(galaxyManager());

    connect(m_logCollector->init(), &Hemera::Operation::finished, this, [this] (Hemera::Operation *op){
        if (op->isError()) {
            qCDebug(LOG_DEVELOPERMODE) << "Error in loading LogCollector: " << op->errorName() << op->errorMessage();
        } else {
            qCDebug(LOG_DEVELOPERMODE) << "Log collector plugin loaded";
            setLoaded();
        }
    });
}
