#include <QApplication>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "MainWindow.h"
#include "AllocationRegistry.h"

extern AllocationRegistry g_Registry;

struct FakeAlloc
{
    void* fakeAddress;
    size_t size;
};

void simulateMemoryLoad() 
{
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::vector<FakeAlloc> activeAllocs;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uintptr_t> addrDist(0x10000000, 0x9FFFFFFF);
    std::uniform_int_distribution<int> typeDist(0, 3);

    while (true)
    {
        void* randomAddress = reinterpret_cast<void*>(addrDist(gen));

        size_t blockSize = 16;
        int type = typeDist(gen);
        if (type == 1) blockSize = 128;
        if (type == 2) blockSize = 1024;
        if (type == 3) blockSize = 5000;

        void* mockStack[3] = { (void*)0x555555, (void*)0x666666, (void*)0x777777 };

        g_Registry.Add(randomAddress, blockSize, mockStack, 3);
        activeAllocs.push_back({ randomAddress, blockSize });

        if (activeAllocs.size() > 450) 
        {
            for (int i = 0; i < 5; ++i)
            {
                FakeAlloc toRemove = activeAllocs.front();
                g_Registry.Remove(toRemove.fakeAddress);
                activeAllocs.erase(activeAllocs.begin());
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
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