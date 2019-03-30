#ifndef OPENTERRAINDIALOG_H
#define OPENTERRAINDIALOG_H

#include <QDialog>

namespace Ui {
	class OpenTerrainDialog;
};

namespace TerrainViewer
{

class OpenTerrainDialog : public QDialog
{
	Q_OBJECT

public:
	explicit OpenTerrainDialog(QWidget *parent = Q_NULLPTR);
	~OpenTerrainDialog();
	
	/**
	 * \brief Return the size of the terrain along the x axis
	 * \return The size of the terrain along the x axis
	 */
	float sizeX() const;

	/**
	 * \brief Return the size of the terrain along the y axis
	 * \return The size of the terrain along the y axis
	 */
	float sizeY() const;

	/**
	 * \brief Return the maximum altitude of the terrain
	 * \return The maximum altitude of the terrain
	 */
	float maxAltitude() const;

private:
	Ui::OpenTerrainDialog *ui;
};

}

#endif // OPENTERRAINDIALOG_H