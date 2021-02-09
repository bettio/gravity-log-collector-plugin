set(GravityLogCollectorPlugin_SRCS
    DeviceLogConfiguration.cpp
    LogCollectorPlugin.cpp
    LogCollector.cpp
)

# Hyperspace interface
hyperspace_add_qt5_producer(GravityLogCollectorPlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.DeviceLog.Status.json
                            DeviceLogStatusProducer DeviceLogStatusProducer)

hyperspace_add_qt5_producer(GravityLogCollectorPlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.DeviceLog.json
                            DeviceLogProducer DeviceLogProducer)

hyperspace_add_qt5_consumer(GravityLogCollectorPlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.DeviceLog.Configuration.json DeviceLogConfiguration.h DeviceLogConfiguration)

# final lib
add_library(gravity-center-plugin-log-collector SHARED ${GravityLogCollectorPlugin_SRCS})

target_link_libraries(gravity-center-plugin-log-collector
                      Gravity::Supermassive
                      HemeraQt5SDK::Core
                      HyperspaceQt5::Core HyperspaceQt5::ProducerConsumer
                      Qt5::Core Qt5::Network Qt5::DBus
                      ${LIBSYSTEMD_JOURNAL_LIBRARIES}
                     )

# Install phase
install(TARGETS gravity-center-plugin-log-collector
        RUNTIME DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        COMPONENT gravity-log-collector)
