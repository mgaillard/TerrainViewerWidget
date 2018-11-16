#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = Q_NULLPTR);

private slots:
	void loadFile();

private:
	void createActions();

	Ui::MainWindowClass ui;
};
