
#include "processmanager.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ProcessManager manager;
    manager.show();
    return app.exec();
}
