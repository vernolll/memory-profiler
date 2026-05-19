#pragma once

#include <QListWidget>
#include <QVector>
#include <string>

struct CallstackFrameItem
{
    std::string functionName;
    std::string fileName;
    unsigned int lineNumber;
};

class CallstackWidget : public QListWidget
{
public:
    explicit CallstackWidget(QWidget* parent = nullptr);
    ~CallstackWidget() = default;

    void displayCallstack(void* address, size_t size, const QVector<CallstackFrameItem>& frames);

    void showMessage(const QString& message);
};