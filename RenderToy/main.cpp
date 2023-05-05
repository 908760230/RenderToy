#include <winsdkver.h>
#include "MainWindow.h"
#include "Engine.h"
#include <iostream>

int  main (int argc, char* agrv[]) {
	
	MainWindow window;
	window.setWindowTitle("Toy");
	window.show();

	Engine engine;
	engine.setMainWindow(&window);
	engine.init();
	engine.run();

	return 0;
}