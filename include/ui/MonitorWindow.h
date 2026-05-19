#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <windows.h>
#include "../SharedMemory.h"
#include "../AllocationRegistry.h"
#include "../SymbolResolver.h"
#include "MemoryMapWidget.h"
#include "CallstackWidget.h"

class MonitorWindow : public QWidget
{
public:
    explicit MonitorWindow(QWidget* parent = nullptr);
    ~MonitorWindow();

private slots:
    void updateProfilerData();

private:
    void initLayout();
    void connectToSharedMemory();

    HANDLE m_hMapFile = NULL;
    SharedMemoryPayload* m_payload = nullptr;
    uint32_t m_lastChangeCount = 0;

    QTimer* m_updateTimer;
    QLabel* m_statusLabel;
    QLabel* m_infoLabel;
    MemoryMapWidget* m_memoryMap;
    CallstackWidget* m_callstackList;

    SymbolResolver m_resolver;
};