#include <QApplication>
#include "../include/ui/MonitorWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    MonitorWindow window;
    window.show();

    return app.exec();
}