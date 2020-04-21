#ifndef PROGRESS_H
#define PROGRESS_H

#include <QDialog>
#include <QString>
#include <QSerialPortInfo>
#include <QWidget>
#include <QProgressBar>
#include "ui_progress.h"

class Progress : public QDialog, protected Ui::Progress
{
	Q_OBJECT
	public:
		Progress(QString info, QSerialPortInfo portInfo, QWidget *parent=nullptr);
		~Progress();
		QProgressBar *getProgressBar();
};

#endif //PROGRESS_H
