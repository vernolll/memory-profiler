#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <windows.h>
#include "../SharedMemory.h"

class MonitorWindow : public QMainWindow
{
public:
    MonitorWindow(QWidget* parent = nullptr);
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
    QListWidget* m_callstackList;
};