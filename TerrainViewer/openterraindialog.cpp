#include "openterraindialog.h"

#include "ui_openterraindialog.h"

using namespace TerrainViewer;

OpenTerrainDialog::OpenTerrainDialog(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::OpenTerrainDialog();
	ui->setupUi(this);
}

OpenTerrainDialog::~OpenTerrainDialog()
{
	delete ui;
}

float OpenTerrainDialog::sizeX() const
{
	return static_cast<float>(ui->sizeXDoubleSpinBox->value());
}

float OpenTerrainDialog::sizeY() const
{
	return static_cast<float>(ui->sizeYDoubleSpinBox->value());
}

float OpenTerrainDialog::maxAltitude() const
{
	return static_cast<float>(ui->maxAltitudeDoubleSpinBox->value());
}
