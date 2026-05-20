#include "MonitorWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <vector>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>

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
    filterLabel->setStyleSheet("color: #000; font-weight: bold;");

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

    QPushButton* exportButton = new QPushButton("Export Report", this);
    exportButton->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; border: none; padding: 5px 10px; font-weight: bold; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:pressed { background-color: #1e7e34; }"
    );

    topControlLayout->addWidget(exportButton);

    connect(exportButton, &QPushButton::clicked, this, &MonitorWindow::exportCurrentReport);

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

    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* callstackTitle = new QLabel("Allocation call stack:", this);
    callstackTitle->setStyleSheet("font-weight: bold; color: #569CD6; margin-bottom: 2px;");

    titleLayout->addStretch(3);
    titleLayout->addWidget(callstackTitle, 2);
    totalLayout->addLayout(titleLayout);

    m_historyChart = new MemoryHistoryWidget(this);
    totalLayout->addWidget(m_historyChart);

    QHBoxLayout* workLayout = new QHBoxLayout();

    m_memoryMap = new MemoryMapWidget(this);

    workLayout->addWidget(m_memoryMap, 3);

    m_callstackList = new CallstackWidget(this);

    workLayout->addWidget(m_callstackList, 2);

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

    size_t activeAllocations = 0;
    size_t totalBytesInHeap = 0;

    for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i)
    {
        if (m_payload->records[i].active.load(std::memory_order_relaxed))
        {
            activeAllocations++;
            totalBytesInHeap += m_payload->records[i].size;
        }
    }

    double megabytes = static_cast<double>(totalBytesInHeap) / (1024.0 * 1024.0);
    if (m_historyChart)
    {
        m_historyChart->addSample(megabytes);
    }

    if (currentCount != m_lastChangeCount)
    {
        m_lastChangeCount = currentCount;

        long totalProbes = m_payload->totalProbesCount;
        long totalAllocs = m_payload->totalAllocsCount;

        double collisionRate = 0.0;
        if (totalAllocs > 0 && totalProbes >= totalAllocs)
        {
            collisionRate = (static_cast<double>(totalProbes - totalAllocs) / totalProbes) * 100.0;
        }

        m_infoLabel->setText(QString("Active blocks: %1 | Total IPC ops: %2 | Hash Collision Rate: %3%")
            .arg(activeAllocations)
            .arg(currentCount)
            .arg(collisionRate, 0, 'f', 1));

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

void MonitorWindow::exportCurrentReport()
{
    if (!m_payload) 
    {
        QMessageBox::warning(this, "Export", "No Shared Memory payload available.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Memory Profiler Report", "", "Text Files (*.txt);;CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
    {
        QMessageBox::critical(this, "Error", "Could not open or create file.");
        return;
    }

    QTextStream out(&file);

    if (fileName.endsWith(".csv")) 
    {
        out << "Index;Address;Size(Bytes);Status\n";
        for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i) 
        {
            if (m_payload->records[i].active.load(std::memory_order_relaxed)) 
            {
                out << i << ";"
                    << QString("0x%1").arg(reinterpret_cast<uintptr_t>(m_payload->records[i].address), 0, 16) << ";"
                    << m_payload->records[i].size << ";"
                    << "Allocated\n";
            }
        }
    }
    else 
    {
        out << "          CORE MEMORY PROFILER REPORT            \n";

        int activeCount = 0;
        for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i) 
        {
            if (m_payload->records[i].active.load(std::memory_order_relaxed))
            {
                activeCount++;
                out << "Block #" << activeCount << "\n";
                out << "  Registry Index: " << i << "\n";
                out << "  Memory Address: " << QString("0x%1").arg(reinterpret_cast<uintptr_t>(m_payload->records[i].address), 0, 16) << "\n";
                out << "  Size:           " << m_payload->records[i].size << " bytes\n";
            }
        }
        out << "\nTotal active blocks logged: " << activeCount << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Report successfully exported!");
}


MemoryHistoryWidget::MemoryHistoryWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(60);
}

void MemoryHistoryWidget::addSample(double megabytes)
{
    m_history.append(megabytes);
    if (m_history.size() > 200)
    {
        m_history.removeFirst();
    }
    update();
}

void MemoryHistoryWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#252526"));

    if (m_history.isEmpty()) return;

    double maxVal = 1.0;
    for (double v : m_history) { if (v > maxVal) maxVal = v; }

    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QColor("#333333"));
    for (int y = 0; y < height(); y += 20)
    {
        painter.drawLine(0, y, width(), y);
    }

    QPainterPath path;
    double stepX = static_cast<double>(width()) / 200.0;

    path.moveTo(0, height());
    for (int i = 0; i < m_history.size(); ++i)
    {
        double x = i * stepX;
        double y = height() - (m_history[i] / maxVal) * (height() - 10);
        path.lineTo(x, y);
    }
    path.lineTo((m_history.size() - 1) * stepX, height());

    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(78, 201, 176, 100));
    gradient.setColorAt(1, QColor(78, 201, 176, 0));
    painter.fillPath(path, gradient);

    painter.setPen(QPen(QColor("#4EC9B0"), 2));

    for (int i = 1; i < m_history.size(); ++i)
    {
        double x1 = (i - 1) * stepX;
        double y1 = height() - (m_history[i - 1] / maxVal) * (height() - 10);
        double x2 = i * stepX;
        double y2 = height() - (m_history[i] / maxVal) * (height() - 10);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    painter.setPen(QColor("#fff"));
    painter.drawText(10, 20, QString("RAM Load: %1 MB").arg(m_history.last(), 0, 'f', 2));
}