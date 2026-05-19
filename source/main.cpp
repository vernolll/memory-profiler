#include <QApplication>
#include <thread>
#include <chrono>
#include "MainWindow.h"
#include "MemoryTracker.h"

extern AllocationRegistry g_Registry;

void simulateMemoryLoad() 
{
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::vector<void*> allocatedBuffers;
    size_t step = 0;

    while (true) 
    {
        size_t blockSize = 16;
        if (step % 4 == 1) blockSize = 128;
        if (step % 4 == 2) blockSize = 1024;
        if (step % 4 == 3) blockSize = 5000;

        void* fakeAddress = ::malloc(blockSize);

        void* mockStack[3] = { (void*)0x111111, (void*)0x222222, (void*)0x333333 };

        g_Registry.Add(fakeAddress, blockSize, mockStack, 3);
        allocatedBuffers.push_back(fakeAddress);

        if (allocatedBuffers.size() > 150)
        {
            for (size_t i = 0; i < 40; ++i)
            {
                void* ptrToRemove = allocatedBuffers[i];
                g_Registry.Remove(ptrToRemove);
                ::free(ptrToRemove);
            }
            allocatedBuffers.erase(allocatedBuffers.begin(), allocatedBuffers.begin() + 40);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        step++;
    }
}

int main(int argc, char* argv[])
{
    std::thread loadThread(simulateMemoryLoad);
    loadThread.detach();

    QApplication app(argc, argv);

    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}