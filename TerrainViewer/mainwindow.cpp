#include "mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "terrain.h"
#include "terrainimages.h"
#include "openterraindialog.h"

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
		const auto& terrain = ui.terrainViewerWidget->terrain();
		const auto image = TerrainViewer::normalTextureImage(terrain);

		if (!image.save(filename))
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
		const auto& terrain = ui.terrainViewerWidget->terrain();
		const auto& parameters = ui.terrainViewerWidget->parameters();
		const auto image = lightMapTextureImage(terrain, parameters);

		if (!image.save(filename))
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
		const auto& terrain = ui.terrainViewerWidget->terrain();
		const auto& parameters = ui.terrainViewerWidget->parameters();
		const auto image = demTextureImage(terrain, parameters);

		if (!image.save(filename))
		{
			QMessageBox::critical(this, tr("Error while saving"), tr("Impossible to save the DEM texture"));
		}
	}
}

void MainWindow::resetViewerWidget()
{
	const auto camera = ui.terrainViewerWidget->camera();
	const auto terrain = ui.terrainViewerWidget->terrain();

	// Delete current viewer widget
	ui.terrainViewerWidget->cleanup();
	ui.terrainViewerWidget->deleteLater();

	// Create a new viewer widget
	ui.terrainViewerWidget = new TerrainViewer::TerrainViewerWidget(ui.centralWidget);
	ui.verticalLayout->addWidget(ui.terrainViewerWidget);

	// Keep the same parameters in the new widget
	QTimer::singleShot(0, this, [this, camera, terrain]() {
		ui.terrainViewerWidget->setCamera(camera);
		ui.terrainViewerWidget->loadTerrain(terrain);
		ui.terrainViewerWidget->setParameters(m_parameterDock->parameters());
	});
}

void MainWindow::initWaterSimulation()
{
	ui.terrainViewerWidget->startWaterSimulation();
}

void MainWindow::pauseWaterSimulation()
{
	ui.terrainViewerWidget->pauseWaterSimulation();
}

void MainWindow::resumeWaterSimulation()
{
	ui.terrainViewerWidget->resumeWaterSimulation();
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
	connect(ui.actionInitialize_water, &QAction::triggered, this, &MainWindow::initWaterSimulation);
	connect(ui.actionPauseSimulation, &QAction::triggered, this, &MainWindow::pauseWaterSimulation);
	connect(ui.actionResumeSimulation, &QAction::triggered, this, &MainWindow::resumeWaterSimulation);
	connect(m_parameterDock, &TerrainViewer::ParameterDock::parameterChanged, [=]() {
		ui.terrainViewerWidget->setParameters(m_parameterDock->parameters());
	});
}
