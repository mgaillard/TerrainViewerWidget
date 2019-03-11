#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "parameterdock.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = Q_NULLPTR);

private slots:
	void loadFile();

private:
	void setupUi();
	void createActions();

	Ui::MainWindowClass ui;

	TerrainViewer::ParameterDock* m_parameterDock;
};
