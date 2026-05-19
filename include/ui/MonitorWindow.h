#pragma once

#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <windows.h>

#include "SharedMemory.h"
#include "AllocationRegistry.h"
#include "SymbolResolver.h"
#include "MemoryMapWidget.h"
#include "CallstackWidget.h"

class MonitorWindow : public QWidget
{
public:
    explicit MonitorWindow(QWidget* parent = nullptr);
    ~MonitorWindow();

private slots:
    void updateProfilerData();
    void applyFilter(int index);
    void forceReconnect();

private:
    void initLayout();
    void connectToSharedMemory();

    HANDLE m_hMapFile = NULL;
    SharedMemoryPayload* m_payload = nullptr;
    uint32_t m_lastChangeCount = 0;

    QComboBox* m_filterCombo;
    QPushButton* m_reconnectButton;

    QTimer* m_updateTimer;
    QLabel* m_statusLabel;
    QLabel* m_infoLabel;
    MemoryMapWidget* m_memoryMap;
    CallstackWidget* m_callstackList;

    SymbolResolver m_resolver;

    size_t m_minSizeFilter = 0;
};