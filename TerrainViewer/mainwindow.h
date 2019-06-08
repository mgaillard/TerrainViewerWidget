#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "parameterdock.h"
#include "openterraindialog.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = Q_NULLPTR);

private slots:
	void loadFile();

	void exportNormalMap();

	void exportLightMap();

	void exportDemTexture();

private:
	void setupUi();
	void createActions();

	Ui::MainWindowClass ui;

	TerrainViewer::ParameterDock* m_parameterDock;
};

#endif // MAINWINDOW_H