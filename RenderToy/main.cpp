#include <winsdkver.h>
#include "MainWindow.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	MainWindow window(hInstance);

	window.resize(800, 600);
	window.setWindowTitle("��ĵ�һ���������");

	window.show();

	return 0;
}