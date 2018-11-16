#include "mainwindow.h"

#include <QtWidgets/QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setVersion(4, 0);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow w;
	w.show();
	return a.exec();
}
