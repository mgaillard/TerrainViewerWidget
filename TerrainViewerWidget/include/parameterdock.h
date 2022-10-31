#ifndef PARAMETERDOCK_H
#define PARAMETERDOCK_H

#include <QDockWidget>

#include "terrainviewerparameters.h"

namespace Ui {
	class ParameterDock;
}

namespace TerrainViewer
{

class ParameterDock : public QDockWidget
{
	Q_OBJECT

public:
	explicit ParameterDock(QWidget *parent = Q_NULLPTR);
	virtual ~ParameterDock();
	
	/**
	 * \brief Set the parameters
	 * \return The parameters
	 */
	void setParameters(const Parameters& parameters);

	/**
	 * \brief Return the parameters specified by the user
	 * \return The parameters
	 */
	Parameters parameters() const;

signals:
	/**
	 * \brief Emitted when a parameter has been changed
	 */
	void parameterChanged();

private:
	void createActions();

	Ui::ParameterDock* ui;
};

}

#endif // PARAMETERDOCK_H