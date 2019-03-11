#include "parameterdock.h"

#include "ui_parameterdock.h"

using namespace TerrainViewer;

ParameterDock::ParameterDock(QWidget *parent)
	: QDockWidget(parent),
	ui(new Ui::ParameterDock)
{
	ui->setupUi(this);
	createActions();
}

ParameterDock::~ParameterDock()
{
	delete ui;
}

void ParameterDock::setParameters(const Parameters& parameters)
{
	ui->paletteComboBox->setCurrentIndex(static_cast<int>(parameters.palette));
	ui->shadingComboBox->setCurrentIndex(static_cast<int>(parameters.shading));
	ui->wireframeCheckBox->setChecked(parameters.wireFrame);
	ui->lodDoubleSpinBox->setValue(parameters.pixelsPerTriangleEdge);
}

Parameters ParameterDock::parameters() const
{
	return {
		static_cast<Palette>(ui->paletteComboBox->currentIndex()),
		static_cast<Shading>(ui->shadingComboBox->currentIndex()),
		ui->wireframeCheckBox->isChecked(),
		static_cast<float>(ui->lodDoubleSpinBox->value())
	};
}

void ParameterDock::createActions()
{
	connect(ui->paletteComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParameterDock::parameterChanged);
	connect(ui->shadingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ParameterDock::parameterChanged);
	connect(ui->wireframeCheckBox, &QCheckBox::stateChanged, this, &ParameterDock::parameterChanged);
	connect(ui->lodDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ParameterDock::parameterChanged);
}
