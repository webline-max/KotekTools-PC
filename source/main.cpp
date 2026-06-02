#include <QApplication>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include "KoTeKTools.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSystemSemaphore semaphore("KoTeKTools_Semaphore", 1);
    semaphore.acquire();
    QSharedMemory sharedMem("KoTeKTools_SharedMemory");
    if (sharedMem.attach()) {
        semaphore.release();
        return 0;
    }
    sharedMem.create(1);
    semaphore.release();

    KoTeKTools window;
    window.show();
    int result = app.exec();
    sharedMem.detach();
    return result;
}