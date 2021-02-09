/*
 *
 */

#ifndef GRAVITY_LOGCOLLECTOR_PLUGIN_H
#define GRAVITY_LOGCOLLECTOR_PLUGIN_H

#include <gravityplugin.h>

namespace Hemera {
    class Operation;
}

class LogCollector;

namespace Gravity {

struct Orbit;

class LogCollectorPlugin : public Gravity::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ispirata.Hemera.GravityCenter.Plugin")
    Q_CLASSINFO("D-Bus Interface", "com.ispirata.Hemera.GravityCenter.Plugins.LogCollector")
    Q_INTERFACES(Gravity::Plugin)

public:
    explicit LogCollectorPlugin();
    virtual ~LogCollectorPlugin();

protected:
    virtual void unload() override final;
    virtual void load() override final;

private:
    LogCollector *m_logCollector;
};
}

#endif // GRAVITY_DEVELOPERMODEPLUGIN_H
