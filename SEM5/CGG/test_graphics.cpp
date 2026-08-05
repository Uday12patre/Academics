#include <graphics.h>
#include <iostream>

int main(int argc, char *argv[]) {
    // Initialize the graphics system
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    // Check if window initialization succeeded
    if (graphresult() != grOk) {
        std::cerr << "Graphics initialization failed!" << std::endl;
        return 1;
    }

    // Draw a blue rectangle
    setcolor(BLUE);
    rectangle(150, 150, 450, 350);

    // Draw a red circle inside it
    setcolor(RED);
    circle(300, 250, 80);

    // Render some white text
    setcolor(WHITE);
    outtextxy(200, 100, (char*)"WSL C++ Graphics Testing Success!");
    outtextxy(220, 400, (char*)"Press any key to close...");

    // Wait for a keypress, then shut down the graphics window
    getch();
    closegraph();
    
    return 0;
}
