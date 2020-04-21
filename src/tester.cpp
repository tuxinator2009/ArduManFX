#include <QtGui>
#include <QtCore>
#include <QMessageBox>
#include <QFileDialog>
#include "ArduManFX.h"
#include "tester.h"

Tester::Tester(QWidget *parent) : QDialog(parent)
{
	setupUi(this);
}

Tester::~Tester()
{
}

void Tester::on_btnPortScan_clicked()
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	tblPorts->clearContents();
	tblPorts->setRowCount(ports.count());
	for (int i = 0; i < ports.count(); ++i)
	{
		tblPorts->setItem(i, 0, new QTableWidgetItem(ports[i].portName()));
		tblPorts->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(ports[i].vendorIdentifier(), 4, 16, QChar('0'))));
		tblPorts->setItem(i, 2, new QTableWidgetItem(QString("%1").arg(ports[i].productIdentifier(), 4, 16, QChar('0'))));
		tblPorts->setItem(i, 3, new QTableWidgetItem(ports[i].manufacturer()));
		tblPorts->setItem(i, 4, new QTableWidgetItem(ports[i].description()));
	}
}

void Tester::on_btnCreateFX_clicked()
{
	QFile file;
	QList<ArduManFX::FlashData> flashData;
	QByteArray flashCart;
	QString inLocation, outLocation;
	inLocation = QFileDialog::getOpenFileName(this, "Choose Flash Cart Index File", QCoreApplication::applicationDirPath(), "CSV Files (*.csv)");
	if (inLocation.isNull())
		return;
	outLocation = QFileDialog::getSaveFileName(this, "Choose Flash Cart Image File", QCoreApplication::applicationDirPath(), "Image Files (*.bin)");
	if (outLocation.isNull())
		return;
	this->setEnabled(false);
	flashData = ArduManFX::parseCSV(inLocation);
	if (flashData.count() == 0)
	{
		QMessageBox::critical(this, "CSV Parse Error", QString("Failed to load the csv file.\nReason: %1").arg(ArduManFX::getErrorString()));
		this->setEnabled(true);
		return;
	}
	flashCart = ArduManFX::createFlashCart(flashData, chkFix1309->isChecked(), chkFixMicro->isChecked(), progressBar);
	if (flashCart.isNull())
	{
		QMessageBox::critical(this, "Cart Failed", QString("Failed to create a flash cart.\nReason: %1").arg(ArduManFX::getErrorString()));
		this->setEnabled(true);
		return;
	}
	file.setFileName(outLocation);
	if (file.open(QFile::WriteOnly))
	{
		file.write(flashCart);
		file.close();
	}
	else
		QMessageBox::critical(this, "Save Failed", QString("Failed to open output file for writing.\nReason: %1").arg(file.errorString()));
	this->setEnabled(true);
}

void Tester::on_btnReadFX_clicked()
{
	QFile file;
	QByteArray flashCart;
	QString outLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	outLocation = QFileDialog::getSaveFileName(this, "Choose Flash Cart Image File", QCoreApplication::applicationDirPath(), "Image Files (*.bin)");
	if (outLocation.isNull())
		return;
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	this->setEnabled(false);
	flashCart = arduboy.readFlashCart(progressBar);
	arduboy.disconnect();
	if (flashCart.isNull())
	{
		QMessageBox::critical(this, "Read Failed", QString("Failed to read flash cart.\nReason: %1").arg(ArduManFX::getErrorString()));
		this->setEnabled(true);
		return;
	}
	file.setFileName(outLocation);
	if (file.open(QFile::WriteOnly))
	{
		file.write(flashCart);
		file.close();
	}
	else
		QMessageBox::critical(this, "Save Failed", QString("Failed to save flash cart to file.\nReason: %1").arg(file.errorString()));
	this->setEnabled(true);
}

void Tester::on_btnWriteFX_clicked()
{
	QFile file;
	QByteArray flashCart;
	QString inLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	inLocation = QFileDialog::getOpenFileName(this, "Choose Flash Cart File", QCoreApplication::applicationDirPath(), "Supported Flash Carts (*.csv *.bin)");
	if (inLocation.isNull())
		return;
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	if (inLocation.endsWith(".csv", Qt::CaseInsensitive))
	{
		QList<ArduManFX::FlashData> flashData = ArduManFX::parseCSV(inLocation);
		if (flashData.count() == 0)
		{
			QMessageBox::critical(this, "CSV Parse Error", QString("Failed to load the csv file.\nReason: %1").arg(ArduManFX::getErrorString()));
			this->setEnabled(true);
			return;
		}
		flashCart = ArduManFX::createFlashCart(flashData, chkFix1309->isChecked(), chkFixMicro->isChecked(), progressBar);
		if (flashCart.isNull())
		{
			QMessageBox::critical(this, "Cart Failed", QString("Failed to create a flash cart.\nReason: %1").arg(ArduManFX::getErrorString()));
			this->setEnabled(true);
			return;
		}
	}
	else
	{
		file.setFileName(inLocation);
		if (!file.open(QFile::ReadOnly))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open file in read-only mode.\nReason: %1").arg(file.errorString()));
			return;
		}
		flashCart = file.readAll();
	}
	file.close();
	this->setEnabled(false);
	if (!arduboy.writeFlashCart(flashCart, chkVerify->isChecked(), progressBar))
	{
		this->setEnabled(true);
		QMessageBox::critical(this, "Upload Error", QString("Failed to write flash cart to device.\nReason: %1").arg(ArduManFX::getErrorString()));
	}
	arduboy.disconnect();
	this->setEnabled(true);
}

void Tester::on_btnReadEEPROM_clicked()
{
	QFile file;
	QByteArray eeprom;
	QString outLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	outLocation = QFileDialog::getSaveFileName(this, "Choose EEPROM File", QCoreApplication::applicationDirPath(), "EEPROM Files (*.eeprom)");
	if (outLocation.isNull())
		return;
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	this->setEnabled(false);
	eeprom = arduboy.readEEPROM();
	arduboy.disconnect();
	if (eeprom.isNull())
	{
		QMessageBox::critical(this, "Read Failed", QString("Failed to read EEPROM.\nReason: %1").arg(ArduManFX::getErrorString()));
		this->setEnabled(true);
		return;
	}
	file.setFileName(outLocation);
	if (file.open(QFile::WriteOnly))
	{
		file.write(eeprom);
		file.close();
	}
	else
		QMessageBox::critical(this, "Save Failed", QString("Failed to save EEPROM to file.\nReason: %1").arg(file.errorString()));
	this->setEnabled(true);
}

void Tester::on_btnWriteEEPROM_clicked()
{
	QFile file;
	QByteArray eeprom;
	QString inLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	inLocation = QFileDialog::getOpenFileName(this, "Choose EEPROM File", QCoreApplication::applicationDirPath(), "EEPROM Files (*.eeprom)");
	if (inLocation.isNull())
		return;
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	file.setFileName(inLocation);
	if (!file.open(QFile::ReadOnly))
	{
		QMessageBox::critical(this, "File I/O Error", QString("Failed to open file in read-only mode.\nReason: %1").arg(file.errorString()));
		return;
	}
	eeprom = file.readAll();
	file.close();
	this->setEnabled(false);
	if (!arduboy.writeEEPROM(eeprom, chkVerify->isChecked()))
		QMessageBox::critical(this, "Upload Error", QString("Failed to write EEPROM to device.\nReason: %1").arg(ArduManFX::getErrorString()));
	arduboy.disconnect();
	this->setEnabled(true);
}

void Tester::on_btnReadSketch_clicked()
{
	QFile file;
	QByteArray sketch;
	QString outLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	outLocation = QFileDialog::getSaveFileName(this, "Choose Sketch File", QCoreApplication::applicationDirPath(), "Sketch Binaries (*.bin)");
	if (outLocation.isNull())
		return;
	this->setEnabled(false);
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	sketch = arduboy.readSketch();
	arduboy.disconnect();
	if (sketch.isNull())
	{
		QMessageBox::critical(this, "Read Failed", QString("Failed to read sketch.\nReason: %1").arg(ArduManFX::getErrorString()));
		this->setEnabled(true);
		return;
	}
	file.setFileName(outLocation);
	if (file.open(QFile::WriteOnly))
	{
		file.write(sketch);
		file.close();
	}
	else
		QMessageBox::critical(this, "Save Failed", QString("Failed to save sketch to file.\nReason: %1").arg(file.errorString()));
	QMessageBox::information(this, "Save Finished", QString("Sketch size %1\n").arg(sketch.length()));
	this->setEnabled(true);
}

void Tester::on_btnWriteSketch_clicked()
{
	QFile file;
	QTextStream stream(&file);
	QString hex;
	QByteArray sketch;
	QString inLocation;
	QString vid, pid;
	QSerialPortInfo port;
	on_btnPortScan_clicked();
	inLocation = QFileDialog::getOpenFileName(this, "Choose Sketch File", QCoreApplication::applicationDirPath(), "Sketch Files (*.hex *.bin)");
	if (inLocation.isNull())
		return;
	if (!arduboy.connect())
	{
		QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
		return;
	}
	port = arduboy.getPortInfo();
	vid = QString("%1").arg(port.vendorIdentifier(), 4, 16, QChar('0'));
	pid = QString("%1").arg(port.productIdentifier(), 4, 16, QChar('0'));
	for (int i = 0; i < tblPorts->rowCount(); ++i)
	{
		if (port.portName() == tblPorts->item(i, 0)->text() && vid == tblPorts->item(i, 1)->text() && pid == tblPorts->item(i, 2)->text())
			tblPorts->selectRow(i);
	}
	file.setFileName(inLocation);
	if (inLocation.endsWith(".hex", Qt::CaseInsensitive))
	{
		if (!file.open(QFile::ReadOnly|QFile::Text))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open file in read-only text mode.\nReason: %1").arg(file.errorString()));
			return;
		}
		hex = stream.readAll();
		file.close();
		this->setEnabled(false);
		if (!arduboy.writeSketch(hex, chkVerify->isChecked(), chkFix1309->isChecked(), chkFixMicro->isChecked(), progressBar))
			QMessageBox::critical(this, "Upload Error", QString("Failed to write sketch to device.\nReason: %1").arg(ArduManFX::getErrorString()));
		arduboy.disconnect();
		this->setEnabled(true);
	}
	else
	{
		if (!file.open(QFile::ReadOnly))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open file in read-only mode.\nReason: %1").arg(file.errorString()));
			return;
		}
		sketch = file.readAll();
		file.close();
		this->setEnabled(false);
		if (!arduboy.writeSketch(sketch, chkVerify->isChecked(), chkFix1309->isChecked(), chkFixMicro->isChecked(), progressBar))
			QMessageBox::critical(this, "Upload Error", QString("Failed to write sketch to device.\nReason: %1").arg(ArduManFX::getErrorString()));
		arduboy.disconnect();
		this->setEnabled(true);
	}
}
