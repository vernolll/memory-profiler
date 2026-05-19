#include "MonitorWindow.h"

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
    QVBoxLayout* totalLayout = new QVBoxLayout(this);

    QHBoxLayout* topControlLayout = new QHBoxLayout();

    QLabel* filterLabel = new QLabel("Size Filter:", this);
    filterLabel->setStyleSheet("color: #fff; font-weight: bold;");

    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("All allocations", QVariant(0));
    m_filterCombo->addItem("> 32 bytes", QVariant(32));
    m_filterCombo->addItem("> 512 bytes", QVariant(512));
    m_filterCombo->addItem("> 4 KB (Page)", QVariant(4096));
    m_filterCombo->setStyleSheet("background-color: #3C3C3C; color: #fff; padding: 3px; border: 1px solid #555;");

    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MonitorWindow::applyFilter);

    m_reconnectButton = new QPushButton("Reconnect SHM", this);
    m_reconnectButton->setStyleSheet(
        "QPushButton { background-color: #0E639C; color: white; border: none; padding: 5px 10px; font-weight: bold; }"
        "QPushButton:hover { background-color: #1177BB; }"
        "QPushButton:pressed { background-color: #0C517F; }"
    );
    connect(m_reconnectButton, &QPushButton::clicked, this, &MonitorWindow::forceReconnect);

    topControlLayout->addWidget(filterLabel);
    topControlLayout->addWidget(m_filterCombo);
    topControlLayout->addSpacing(20);
    topControlLayout->addWidget(m_reconnectButton);
    topControlLayout->addStretch();

    totalLayout->addLayout(topControlLayout);

    m_statusLabel = new QLabel("Status: Pending connection...", this);
    m_statusLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: gray;");
    totalLayout->addWidget(m_statusLabel);

    m_infoLabel = new QLabel("Active blocks in the heap: 0 | Total IPC operations: 0", this);
    m_infoLabel->setStyleSheet("color: #aaa; margin-bottom: 5px;");
    totalLayout->addWidget(m_infoLabel);

    QHBoxLayout* workLayout = new QHBoxLayout();

    m_memoryMap = new MemoryMapWidget(this);
    workLayout->addWidget(m_memoryMap, 2);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* callstackTitle = new QLabel("Allocation call stack:", this);
    callstackTitle->setStyleSheet("font-weight: bold; color: #569CD6;");

    m_callstackList = new CallstackWidget(this);

    rightLayout->addWidget(callstackTitle);
    rightLayout->addWidget(m_callstackList);
    workLayout->addLayout(rightLayout, 1);

    totalLayout->addLayout(workLayout, 1); 

    connect(m_memoryMap, &MemoryMapWidget::recordSelected, this, [this](int regIndex) 
        {
        if (!m_payload) return;

        AllocationRecord& record = m_payload->records[regIndex];
        if (!record.active.load(std::memory_order_relaxed)) 
        {
            m_callstackList->showMessage("[The block has already been released by the target application]");
            return;
        }

        std::vector<ResolvedFrame> rawFrames = m_resolver.Resolve(record.callstack, 12);

        QVector<CallstackFrameItem> widgetFrames;
        for (const auto& f : rawFrames) 
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

        m_infoLabel->setText(QString("Active blocks in the heap: %1 | Total IPC operations: %2 | Filter threshold: %3 bytes")
            .arg(activeAllocations)
            .arg(currentCount)
            .arg(m_minSizeFilter));

        m_memoryMap->updateData(m_payload->records, AllocationRegistry::MAX_RECORDS, m_minSizeFilter);
    }
}

MonitorWindow::~MonitorWindow()
{
    if (m_payload) UnmapViewOfFile(m_payload);
    if (m_hMapFile) CloseHandle(m_hMapFile);
}

void MonitorWindow::applyFilter(int index)
{
    m_minSizeFilter = m_filterCombo->itemData(index).toULongLong();

    m_lastChangeCount = 0;
    updateProfilerData();
}

void MonitorWindow::forceReconnect()
{
    if (m_payload) UnmapViewOfFile(m_payload);
    if (m_hMapFile) CloseHandle(m_hMapFile);

    m_payload = nullptr;
    m_hMapFile = NULL;
    m_lastChangeCount = 0;

    m_callstackList->clear();
    connectToSharedMemory();
}