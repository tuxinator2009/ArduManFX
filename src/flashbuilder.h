#ifndef FLASHBUILDER_H
#define FLASHBUILDER_H

#include <QTreeWidget>
#include "ArduManFX.h"

class FlashBuilder : public QTreeWidget
{
	Q_OBJECT
	public:
		FlashBuilder(QWidget *parent=nullptr);
		~FlashBuilder();
		void addCategory(QImage image, QString description);
	private:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dropEvent(QDropEvent *event);
};

#endif //FLASHBUILDER_H
