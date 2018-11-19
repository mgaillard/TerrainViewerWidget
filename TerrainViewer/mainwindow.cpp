#include "mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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
		const auto image = cv::imread(fileName.toStdString(), cv::ImreadModes::IMREAD_ANYDEPTH);
		Terrain terrain(10.0f, 10.0f, 1.0f);
		if (terrain.loadFromImage(image))
		{
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
