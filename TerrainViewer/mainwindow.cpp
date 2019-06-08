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
	setupUi();
	createActions();
}

void MainWindow::loadFile()
{
	// Ask the user for a file to import
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load a terrain"), "", tr("Images (*.png *.jpg)"));

	// Check if file exists
	if (QFileInfo::exists(fileName))
	{
		// Ask the user for the size of the terrain
		auto dialog = new TerrainViewer::OpenTerrainDialog(this);
		const int returnCode = dialog->exec();

		if (returnCode == QDialog::Accepted)
		{
			const auto image = cv::imread(fileName.toStdString(), cv::ImreadModes::IMREAD_ANYDEPTH);

			TerrainViewer::Terrain terrain(dialog->sizeX(), dialog->sizeY(), dialog->maxAltitude());
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
}

void MainWindow::exportNormalMap()
{
	const QString filename = QFileDialog::getSaveFileName(this, tr("Save normal map"), "", tr("Images (*.png *.xpm *.jpg)"));

	if (!filename.isEmpty())
	{
		const bool saved = ui.terrainViewerWidget->normalTexture().save(filename);

		if (!saved)
		{
			QMessageBox::critical(this, tr("Error while saving"), tr("Impossible to save the normal map"));
		}
	}
}

void MainWindow::exportLightMap()
{
	const QString filename = QFileDialog::getSaveFileName(this, tr("Save light map"), "", tr("Images (*.png *.xpm *.jpg)"));

	if (!filename.isEmpty())
	{
		const bool saved = ui.terrainViewerWidget->lightMapTexture().save(filename);

		if (!saved)
		{
			QMessageBox::critical(this, tr("Error while saving"), tr("Impossible to save the normal map"));
		}
	}
}

void MainWindow::exportDemTexture()
{
	const QString filename = QFileDialog::getSaveFileName(this, tr("Save DEM texure"), "", tr("Images (*.png *.xpm *.jpg)"));

	if (!filename.isEmpty())
	{
		const bool saved = ui.terrainViewerWidget->demTexture().save(filename);

		if (!saved)
		{
			QMessageBox::critical(this, tr("Error while saving"), tr("Impossible to save the DEM texture"));
		}
	}
}

void MainWindow::setupUi()
{
	ui.setupUi(this);

	m_parameterDock = new TerrainViewer::ParameterDock(this);
	m_parameterDock->setParameters(TerrainViewer::TerrainViewerWidget::default_parameters);
	addDockWidget(Qt::RightDockWidgetArea, m_parameterDock);
	ui.menuWindow->addAction(m_parameterDock->toggleViewAction());
}

void MainWindow::createActions()
{
	connect(ui.actionLoad, &QAction::triggered, this, &MainWindow::loadFile);
	connect(ui.actionExport_normal_map, &QAction::triggered, this, &MainWindow::exportNormalMap);
	connect(ui.actionExport_light_map, &QAction::triggered, this, &MainWindow::exportLightMap);
	connect(ui.actionExport_DEM_texture, &QAction::triggered, this, &MainWindow::exportDemTexture);
	connect(m_parameterDock, &TerrainViewer::ParameterDock::parameterChanged, [=]() {
		ui.terrainViewerWidget->setParameters(m_parameterDock->parameters());
	});
}
