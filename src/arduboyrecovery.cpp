#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <cstdio>
#include <cstdlib>
#include "arduboyrecovery.h"
#include "ArduManFX.h"
#include "progress.h"

ArduboyRecovery::ArduboyRecovery(QWidget *parent) : QDialog(parent)
{
	QFile hexFile(":/recovered.hex");
	QTextStream stream(&hexFile);
	setupUi(this);
	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(scanForArduboy()));
	timer.start(100);
	hexFile.open(QFile::ReadOnly|QFile::Text);
	hex = stream.readAll();
	hexFile.close();
	resize(sizeHint());
	setMinimumSize(sizeHint());
	setMaximumSize(sizeHint());
}

ArduboyRecovery::~ArduboyRecovery()
{
}

void ArduboyRecovery::scanForArduboy()
{
	if (arduboy.connect())
	{
		timer.stop();
		if (!arduboy.writeSketch(hex, true, false, false, progressBar))
			QMessageBox::critical(this, "Upload Failed", QString("Failed to upload recovery sketch to Arduboy\n\n%1").arg(arduboy.getErrorString()));
		arduboy.disconnect();
		accept();
	}
}
