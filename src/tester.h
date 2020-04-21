#ifndef TESTER_H
#define TESTER_H

#include "ArduManFX.h"
#include "ui_tester.h"

class Tester : public QDialog, protected Ui::Tester
{
	Q_OBJECT
	public:
		Tester(QWidget *parent=nullptr);
		~Tester();
	protected slots:
		void on_btnPortScan_clicked();
		void on_btnCreateFX_clicked();
		void on_btnReadFX_clicked();
		void on_btnWriteFX_clicked();
		void on_btnReadEEPROM_clicked();
		void on_btnWriteEEPROM_clicked();
		void on_btnReadSketch_clicked();
		void on_btnWriteSketch_clicked();
	private:
		ArduManFX arduboy;
};

#endif //TESTER_H
