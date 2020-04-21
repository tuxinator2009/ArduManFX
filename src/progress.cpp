#include <QProgressBar>
#include <QLabel>
#include <QSerialPortInfo>
#include "progress.h"

Progress::Progress(QString info, QSerialPortInfo portInfo, QWidget *parent) : QDialog(parent)
{
	setupUi(this);
	lblInfo->setText(info);
	lblPort->setText(portInfo.portName());
	lblVID->setText(QString("%1").arg(portInfo.vendorIdentifier(), 2, 16, QChar('0')));
	lblPID->setText(QString("%1").arg(portInfo.productIdentifier(), 2, 16, QChar('0')));
	lblManufacturer->setText(portInfo.manufacturer());
	lblDescription->setText(portInfo.description());
	resize(sizeHint());
	setMinimumSize(sizeHint());
	setMaximumSize(sizeHint());
}

Progress::~Progress()
{
}

QProgressBar *Progress::getProgressBar()
{
	return progressBar;
}
