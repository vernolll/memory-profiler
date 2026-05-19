#pragma once

#include <QWidget>
#include <QVector>
#include "../AllocationRegistry.h"

struct VisualRecord 
{
    void* address;
    size_t size;
    bool active;
    int registryIndex;
};

class MemoryMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemoryMapWidget(QWidget* parent = nullptr);
    ~MemoryMapWidget() = default;

    void updateData(AllocationRecord* records, size_t maxRecords, size_t minSizeFilter = 0);

signals:
    void recordSelected(int registryIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QVector<VisualRecord> m_visualRecords;

    const int m_boxSize = 12;
    const int m_boxSpacing = 2;
};