#include "../../include/ui/MonitorWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <vector>

MonitorWindow::MonitorWindow(QWidget* parent) : QWidget(parent)
{
    initLayout();
    connectToSharedMemory();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MonitorWindow::updateProfilerData);
    m_updateTimer->start(100);
}

void MonitorWindow::initLayout()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QVBoxLayout* leftLayout = new QVBoxLayout();

    m_statusLabel = new QLabel("Status: Pending connection...", this);
    m_statusLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: gray;");
    leftLayout->addWidget(m_statusLabel);

    m_infoLabel = new QLabel("Active allocations: 0 | Total operations: 0", this);
    m_infoLabel->setStyleSheet("color: #aaa; margin-bottom: 5px;");
    leftLayout->addWidget(m_infoLabel);

    m_memoryMap = new MemoryMapWidget(this);
    leftLayout->addWidget(m_memoryMap, 1);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* callstackTitle = new QLabel("Allocation call stack (Call Stack):", this);
    callstackTitle->setStyleSheet("font-weight: bold; color: #569CD6;");

    m_callstackList = new CallstackWidget(this);

    rightLayout->addWidget(callstackTitle);
    rightLayout->addWidget(m_callstackList);
    
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addLayout(rightLayout, 1);

    connect(m_memoryMap, &MemoryMapWidget::recordSelected, this, [this](int regIndex) {
        if (!m_payload) return;

        m_callstackList->clear();

        AllocationRecord& record = m_payload->records[regIndex];

        if (!record.active.load(std::memory_order_relaxed)) 
        {
            m_callstackList->showMessage("[The record has already been released by the target application]");
            return;
        }

        std::vector<ResolvedFrame> frames = m_resolver.Resolve(record.callstack, 12);

        QVector<CallstackFrameItem> widgetFrames;
        for (const auto& f : frames)
        {
            if (!f.functionName.empty())
            {
                widgetFrames.append({ f.functionName, f.fileName, f.lineNumber });
            }
        }
        m_callstackList->displayCallstack(record.address, record.size, widgetFrames);
        });
}

void MonitorWindow::connectToSharedMemory()
{
    m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Local\\CppMemoryProfilerShm");
    if (m_hMapFile == NULL)
    {
        m_statusLabel->setText("Status: Waiting for the target application to launch...");
        m_statusLabel->setStyleSheet("font-weight: bold; color: #D16969;");
        return;
    }

    m_payload = (SharedMemoryPayload*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemoryPayload));
    if (m_payload)
    {
        m_statusLabel->setText("Status: Profiling is running");
        m_statusLabel->setStyleSheet("font-weight: bold; color: #4EC9B0;");
    }
}

void MonitorWindow::updateProfilerData()
{
    if (!m_payload)
    {
        connectToSharedMemory();
        return;
    }

    uint32_t currentCount = m_payload->changeCounter;

    if (currentCount != m_lastChangeCount)
    {
        m_lastChangeCount = currentCount;

        size_t activeAllocations = 0;
        for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i) 
        {
            if (m_payload->records[i].active.load(std::memory_order_relaxed)) 
            {
                activeAllocations++;
            }
        }

        m_infoLabel->setText(QString("Active blocks in the heap: %1 | Total IPC operations: %2")
            .arg(activeAllocations)
            .arg(currentCount));

        m_memoryMap->updateData(m_payload->records, AllocationRegistry::MAX_RECORDS);
    }
}

MonitorWindow::~MonitorWindow()
{
    if (m_payload) UnmapViewOfFile(m_payload);
    if (m_hMapFile) CloseHandle(m_hMapFile);
}