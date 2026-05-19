#include "CallstackWidget.h"

#include <QFont>
#include <QFileInfo>

CallstackWidget::CallstackWidget(QWidget* parent) : QListWidget(parent)
{
    setStyleSheet
    (
        "QListWidget {"
        "   background-color: #1E1E1E;"
        "   color: #D4D4D4;"
        "   border: 1px solid #333333;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 4px;"
        "   border-bottom: 1px solid #2D2D2D;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #2A2D2E;"
        "}"
    );

    QFont monoFont("Consolas", 10);
    if (!monoFont.exactMatch()) 
    {
        monoFont.setFamily("Courier New");
    }
    setFont(monoFont);
}

void CallstackWidget::displayCallstack(void* address, size_t size, const QVector<CallstackFrameItem>& frames)
{
    clear();

    QString headerText = QString("Block: %1 bytes\n Address: 0x%2")
        .arg(size)
        .arg(QString::number(reinterpret_cast<quintptr>(address), 16).toUpper());

    QListWidgetItem* headerItem = new QListWidgetItem(headerText, this);
    headerItem->setForeground(QColor("#569CD6"));
    QFont headerFont = font();
    headerFont.setBold(true);
    headerItem->setFont(headerFont);
    addItem(headerItem);
    this->setWordWrap(true);

    if (frames.isEmpty()) 
    {
        QListWidgetItem* emptyItem = new QListWidgetItem("  [There is no data about the call stack]", this);
        emptyItem->setForeground(QColor("#888888"));
        addItem(emptyItem);
        return;
    }

    for (const auto& frame : frames)
    {
        if (frame.functionName.empty()) continue;

        QString fullPath = QString::fromStdString(frame.fileName);
        QString fileNameOnly = QFileInfo(fullPath).fileName();

        if (fileNameOnly.isEmpty()) 
        {
            fileNameOnly = "The system module / CRT";
        }

        QString frameText = QString(" %1\n    └─ %2 : string %3")
            .arg(QString::fromStdString(frame.functionName))
            .arg(fileNameOnly)
            .arg(frame.lineNumber);

        QListWidgetItem* item = new QListWidgetItem(frameText, this);

        if (frame.lineNumber > 0)
        {
            item->setForeground(QColor("#4EC9B0"));
        }
        else 
        {
            item->setForeground(QColor("#888888")); 
        }

        addItem(item);
    }
}

void CallstackWidget::showMessage(const QString& message)
{
    clear();
    QListWidgetItem* item = new QListWidgetItem(message, this);
    item->setForeground(QColor("#E06C75")); 
    addItem(item);
}