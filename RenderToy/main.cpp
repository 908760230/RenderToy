#include <winsdkver.h>
#include "MainWindow.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	MainWindow window(hInstance);

	window.resize(800, 600);
	window.setWindowTitle("你的第一个窗体程序");

	window.show();

	return 0;
}