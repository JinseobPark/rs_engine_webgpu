#include "FluidDemoApp.h"

int main() {
    FluidDemoApp app;
    if (app.init()) {
        app.run();
    }
    return 0;
}