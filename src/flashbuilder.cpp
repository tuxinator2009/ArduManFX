#include "ArduManFX.h"
#include "flashbuilder.h"

FlashBuilder::FlashBuilder(QWidget *parent) : QTreeWidget(parent)
{
}

FlashBuilder::~FlashBuilder()
{
}

void FlashBuilder::addCategory(QImage image, QString description)
{
}

void FlashBuilder::dragEnterEvent(QDragEnterEvent *event)
{
	printf("drag enter event\n");
}

void FlashBuilder::dragMoveEvent(QDragMoveEvent *event)
{
	printf("drag move event\n");
}

void FlashBuilder::dropEvent(QDropEvent *event)
{
	printf("drop event\n");
}
