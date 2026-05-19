#include "../../include/ui/MonitorWindow.h"

#include "../../include/SymbolResolver.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

MonitorWindow::MonitorWindow(QWidget* parent) : QMainWindow(parent)
{
    initLayout();

    connectToSharedMemory();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MonitorWindow::updateProfilerData);
    m_updateTimer->start(100);
}

void MonitorWindow::initLayout()
{
    setWindowTitle("Core Memory Profiler v1.0");
    resize(1000, 600);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    m_statusLabel = new QLabel("Status: Pending connection...", this);
    m_statusLabel->setStyleSheet("font-weight: bold; color: gray;");
    leftLayout->addWidget(m_statusLabel);

    QLabel* mapPlaceholder = new QLabel("There will be a custom memory grid here (Heap Map)", this);
    mapPlaceholder->setAlignment(Qt::AlignCenter);
    mapPlaceholder->setStyleSheet("border: 2px dashed #444; background-color: #222; color: #aaa;");
    leftLayout->addWidget(mapPlaceholder, 1); 

    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* callstackTitle = new QLabel("Allocation call stack:", this);
    m_callstackList = new QListWidget(this);
    rightLayout->addWidget(callstackTitle);
    rightLayout->addWidget(m_callstackList);

    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addLayout(rightLayout, 1);
}

void MonitorWindow::connectToSharedMemory()
{
    m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Local\\CppMemoryProfilerShm");
    if (m_hMapFile == NULL)
    {
        m_statusLabel->setText("Status: Shared Memory not found. Launch the target application.");
        m_statusLabel->setStyleSheet("color: red;");
        return;
    }

    m_payload = (SharedMemoryPayload*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemoryPayload));
    if (m_payload)
    {
        m_statusLabel->setText("Status: Profiling is active (Enabled)");
        m_statusLabel->setStyleSheet("color: green;");
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

        m_statusLabel->setText(QString("Status: Active. Memory operations: %1").arg(currentCount));
    }
}

MonitorWindow::~MonitorWindow()
{
    if (m_payload) UnmapViewOfFile(m_payload);
    if (m_hMapFile) CloseHandle(m_hMapFile);
}