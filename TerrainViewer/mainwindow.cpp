#include "mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "terrain.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	createActions();
}

void MainWindow::loadFile()
{
	// Ask the user for a file to import
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load a terrain"), "", tr("Images (*.png *.jpg)"));

	// Check if file exists
	if (QFileInfo::exists(fileName))
	{
		QImage image(fileName);

		if (!image.isNull())
		{
			Terrain terrain(12.8f, 12.8f, 1.0f);
			terrain.loadFromImage(image);

			ui.terrainViewerWidget->loadTerrain(terrain);
		}
		else
		{
			QMessageBox::critical(this, tr("Impossible to import"), tr("It is not a valid image file"));
		}
	}
}

void MainWindow::createActions()
{
	connect(ui.actionLoad, &QAction::triggered, this, &MainWindow::loadFile);
}
