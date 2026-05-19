#include "../../include/ui/MemoryMapWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

MemoryMapWidget::MemoryMapWidget(QWidget* parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(200, 200);
}

void MemoryMapWidget::updateData(AllocationRecord* records, size_t maxRecords)
{
    m_visualRecords.clear();
    m_visualRecords.reserve(static_cast<int>(maxRecords));

    for (size_t i = 0; i < maxRecords; ++i)
    {
        bool isActive = records[i].active.load(std::memory_order_relaxed);

        m_visualRecords.append
        (
            {
            records[i].address,
            records[i].size,
            isActive,
            static_cast<int>(i)
            }
        );
    }

    update();
}

void MemoryMapWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, false);

    if (m_visualRecords.isEmpty())
    {
        painter.setPen(QColor("#888888"));
        painter.drawText(rect(), Qt::AlignCenter, "Waiting for data from Shared Memory...");
        return;
    }

    int widthInBoxes = width() / (m_boxSize + m_boxSpacing);
    if (widthInBoxes <= 0) widthInBoxes = 1;

    for (int i = 0; i < m_visualRecords.size(); ++i)
    {
        int row = i / widthInBoxes;
        int col = i % widthInBoxes;

        int x = col * (m_boxSize + m_boxSpacing);
        int y = row * (m_boxSize + m_boxSpacing);

        if (y > height()) break;

        QRect boxRect(x, y, m_boxSize, m_boxSize);

        if (m_visualRecords[i].active)
        {
            size_t size = m_visualRecords[i].size;
            QColor allocColor;

            if (size <= 32)
            {
                allocColor = QColor("#4EC9B0");
            }
            else if (size <= 512)
            {
                allocColor = QColor("#569CD6");
            }
            else if (size <= 4096) 
            {
                allocColor = QColor("#DCDCAA");
            }
            else 
            {
                allocColor = QColor("#E06C75");
            }

            painter.fillRect(boxRect, allocColor);
        }
        else
        {
            painter.fillRect(boxRect, QColor("#2D2D2D"));
        }
    }
}

void MemoryMapWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_visualRecords.isEmpty()) return;

    int widthInBoxes = width() / (m_boxSize + m_boxSpacing);
    if (widthInBoxes <= 0) widthInBoxes = 1;

    int col = event->x() / (m_boxSize + m_boxSpacing);
    int row = event->y() / (m_boxSize + m_boxSpacing);

    int clickedIndex = row * widthInBoxes + col;

    if (clickedIndex >= 0 && clickedIndex < m_visualRecords.size())
    {
        if (m_visualRecords[clickedIndex].active)
        {
            emit recordSelected(m_visualRecords[clickedIndex].registryIndex);
        }
    }
}