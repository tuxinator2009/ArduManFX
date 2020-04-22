#ifndef ARDUBOYRECOVERY_H
#define ARDUBOYRECOVERY_H

#include "ArduManFX.h"
#include "ui_arduboyrecovery.h"

class ArduboyRecovery : public QDialog, protected Ui::ArduboyRecovery
{
	Q_OBJECT
	public:
		ArduboyRecovery(QWidget *parent=nullptr);
		~ArduboyRecovery();
	protected slots:
		void scanForArduboy();
	private:
		ArduManFX arduboy;
		QTimer timer;
		QString hex;
};

#endif //ARDUBOYRECOVERY_H
