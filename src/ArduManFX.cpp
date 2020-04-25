//The following code is translated from Mr.Blinky's Arduboy-Python-Utilities
//https://github.com/MrBlinky/Arduboy-Python-Utilities

#include <QtCore>
#include <QtSerialPort>
#include <QtGui>
#include "ArduManFX.h"
#include "mainwindow.h"

const quint16 ArduManFX::compatibleDevices[] =
{
	//Arduboy Leonardo
	0x2341,0x0036, 0x2341,0x8036,
	0x2A03,0x0036, 0x2A03,0x8036,
	//Arduboy Micro
	0x2341,0x0037, 0x2341,0x8037,
	0x2A03,0x0037, 0x2A03,0x8037,
	//Genuino Micro
	0x2341,0x0237, 0x2341,0x8237,
	//Sparkfun Pro Micro 5V
	0x1B4F,0x9205, 0x1B4F,0x9206,
	//Adafruit ItsyBitsy 5V
	0x239A,0x000E, 0x239A,0x800E,
};

const quint8 ArduManFX::manufacturerIDs[] = {0x01,0x14,0x1C,0x1F,0x20,0x37,0x9D,0xC2,0xC8,0xBF,0xEF};
const char *ArduManFX::manufacturerNames[]
{
	"Spansion",
	"Cypress",
	"EON",
	"Adesto(Atmel)",
	"Micron",
	"AMIC",
	"ISSI",
	"General Plus",
	"Giga Device",
	"Microchip",
	"Winbond"
};

const char *ArduManFX::lcdBootProgram = "\xD5\xF0\x8D\x14\xA1\xC8\x81\xCF\xD9\xF1\xAF\x20\x00";

QString ArduManFX::errorString = "";

ArduManFX::ArduManFX()
{
	arduboy.setReadBufferSize(0);
	bootloaderActive = false;
}

ArduManFX::~ArduManFX()
{
	if (arduboy.isOpen())
	{
		arduboy.write("E", 1);
		waitWrite();
		waitRead();
		arduboy.read(1);
		arduboy.close();
	}
}

bool ArduManFX::connect()
{
	if (isConnected())
		return true;
	arduboyPort = getComPort();
	bootloaderActive = false;
	if (arduboyPort.isNull())
	{
		errorString = "Failed to find an Arduboy.";
		return false;
	}
	arduboy.setPort(arduboyPort);
	arduboy.setBaudRate(1200);
	if (!arduboy.open(QIODevice::ReadWrite))
	{
		errorString = QString("Failed to open Arduboy port %1.\nReason: %2").arg(arduboyPort.portName()).arg(arduboy.errorString());
		return false;
	}
	return true;
}

void ArduManFX::disconnect()
{
	if (isConnected())
	{
		if (bootloaderActive)
			exitBootloader();
		arduboy.close();
	}
}

bool ArduManFX::startBootloader()
{
	int attempt = 0;
	bootloaderActive = false;
	if (!isConnected())
	{
		if (!connect())
			return false;
	}
	if (!bootloaderActive)
	{
		arduboy.setDataTerminalReady(false);
		QThread::msleep(500);
		arduboy.close();
		//wait for disconnect and reconnect in bootloader mode
		while (samePort(getComPort(), arduboyPort) && !bootloaderActive)
		{
			++attempt;
			if (attempt > 50)
			{
				errorString = QString("Failed to restart Arduboy on port %1.").arg(arduboyPort.portName());
				return false;
			}
			QThread::msleep(100);
		}
		arduboyPort = QSerialPortInfo();
		attempt = 0;
		while (arduboyPort.isNull())
		{
			arduboyPort = getComPort();
			++attempt;
			if (attempt > 50)
			{
				errorString = QString("Failed to connect to Arduboy on port %1.").arg(arduboyPort.portName());
				return false;
			}
			QThread::msleep(100);
		}
		QThread::msleep(100);
		arduboy.setPort(arduboyPort);
		arduboy.setBaudRate(57600);
		if (!arduboy.open(QIODevice::ReadWrite))
		{
			errorString = QString("Failed to open Arduboy port %1.\nReason: %2").arg(arduboyPort.portName()).arg(arduboy.errorString());
			return false;
		}
		QThread::msleep(500);
		bootloaderActive = true;
	}
	return true;
}

void ArduManFX::exitBootloader()
{
	if (isConnected() && bootloaderActive)
	{
		arduboy.write("E", 1);
		waitWrite();
		waitRead();
		arduboy.read(1);
		bootloaderActive = false;
	}
}

bool ArduManFX::isConnected()
{
	if (arduboy.isOpen())
		return true;
	return false;
}

QByteArray ArduManFX::readEEPROM()
{
	QByteArray eeprom;
	if (!startBootloader())
		return QByteArray();
	arduboy.write("A\x00\x00", 3);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	arduboy.read(1);
	arduboy.write("g\x04\x00\x45", 4);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	eeprom = arduboy.read(1024);
	return eeprom;
}

bool ArduManFX::writeEEPROM(QByteArray data, bool verify)
{
	if (data.length() != 1024)
	{
		errorString = "Error: EEPROM data must be exactly 1024 bytes.";
		return false;
	}
	if (!startBootloader())
		return false;
	arduboy.write("A\x00\x00", 3);
	if (!waitWrite())
		return false;
	if (!waitRead())
		return false;
	arduboy.read(1);
	arduboy.write("B\x04\x00\x45", 4);
	if (!waitWrite())
		return false;
	if (!waitRead())
		return false;
	arduboy.write(data);
	if (!waitWrite())
		return false;
	if (!waitRead())
		return false;
	arduboy.read(1);
	if (verify)
	{
		QByteArray eeprom = readEEPROM();
		if (eeprom.length() == 0)
			return false;
		else if (eeprom != data)
		{
			errorString = "Failed to upload EEPROM.\nValues read back don't match values written.";
			return false;
		}
	}
	return true;
}

QByteArray ArduManFX::readFlashCart(QProgressBar *progress)
{
	QByteArray flashCart, jedecID;
	unsigned long capacity, blocks;
	if (!startBootloader())
		return QByteArray();
	if (getVersion() < 13)
	{
		errorString = "Bootloader has no flash cart support";
		return QByteArray();
	}
	jedecID = getJedecID();
	capacity = 1 << jedecID[2];
	blocks = capacity / BLOCKSIZE;
	if (progress != nullptr)
	{
		progress->setMaximum(blocks);
		progress->setValue(0);
	}
	QCoreApplication::processEvents();
	for (unsigned long block = 0; block < blocks; ++block)
	{
		QByteArray data(3, 0xFF);
		unsigned long blockaddr = block * BLOCKSIZE / PAGESIZE;
		if (block & 1)
			arduboy.write("x\xC0", 2);
		else
			arduboy.write("x\xC1", 2);
		if (!waitWrite())
			return QByteArray();
		if (!waitRead())
			return QByteArray();
		arduboy.read(1);
		data[0] = 'A';
		data[1] = blockaddr >> 8;
		data[2] = blockaddr & 0xFF;
		arduboy.write(data);
		if (!waitWrite())
			return QByteArray();
		if (!waitRead())
			return QByteArray();
		arduboy.read(1);
		arduboy.write("g\x00\x00\x43", 4);
		if (!waitWrite())
			return QByteArray();
		if (!waitRead())
			return QByteArray();
		flashCart += arduboy.read(BLOCKSIZE);
		if (progress != nullptr)
			progress->setValue(block + 1);
		QCoreApplication::processEvents();
	}
	arduboy.write("x\x44", 2);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	arduboy.read(1);
	return flashCart;
}

bool ArduManFX::writeFlashCart(QByteArray data, bool verify, QProgressBar *progress)
{
	QByteArray jedecID;
	long blocks;
	long capacity = 1l;
	if (!startBootloader())
		return false;
	if (data.length() % PAGESIZE != 0)
		data += QByteArray(PAGESIZE - (data.length() % PAGESIZE), 0xFF);
	if (getVersion() < 13)
	{
		errorString = "Bootloader has no flash cart support.";
		return false;
	}
	jedecID = getJedecID();
	if (jedecID.isNull())
		return false;
	capacity <<= jedecID[2];
    printf("JEDEC ID: %02X %02X %02X\n", (unsigned char)jedecID.at(0), (unsigned char)jedecID.at(1), (unsigned char)jedecID.at(2));
	if (data.length() > capacity)
	{
		errorString = QString("Flash data too large (%1 %2 %3).\nFlash Data Size: %4 bytes\nFlash Ram Capacity: %5 bytes").arg(jedecID.at(0), 2, 16, QChar('0')).arg(jedecID.at(1), 2, 16, QChar('0')).arg(jedecID.at(2), 2, 16, QChar('0')).arg(data.length()).arg(capacity);
		return false;
	}
	blocks = data.length() / BLOCKSIZE;
	if (progress != nullptr)
	{
		progress->setMaximum(blocks);
		progress->setValue(0);
	}
	QCoreApplication::processEvents();
	for (long block = 0; block < blocks; ++block)
	{
		QByteArray cmd(3, 0xFF);
		long blockaddr = block * BLOCKSIZE / PAGESIZE;
		long blocklen = BLOCKSIZE;
		if (block & 1)
			arduboy.write("x\xC0", 2);
		else
			arduboy.write("x\xC2", 2);
		if (!waitWrite())
			return false;
		if (!waitRead())
			return false;
		arduboy.read(1);
		cmd[0] = 'A';
		cmd[1] = blockaddr >> 8;
		cmd[2] = blockaddr & 0xFF;
		arduboy.write(cmd);
		if (!waitWrite())
			return false;
		if (!waitRead())
			return false;
		arduboy.read(1);
		cmd[0] = 'B';
		cmd[1] = (blocklen >> 8) & 0xFF;
		cmd[2] = blocklen & 0xFF;
		cmd += 'C';
		arduboy.write(cmd);
		arduboy.write(data.mid(block * BLOCKSIZE, blocklen));
		if (!waitWrite())
			return false;
		if (!waitRead())
			return false;
		arduboy.read(1);
		if (verify)
		{
			cmd = QByteArray(3, 0xFF);
			cmd[0] = 'A';
			cmd[1] = blockaddr >> 8;
			cmd[2] = blockaddr & 0xFF;
			arduboy.write(cmd);
			if (!waitWrite())
				return false;
			if (!waitRead())
				return false;
			arduboy.read(1);
			cmd[0] = 'g';
			cmd[1] = (blocklen >> 8) & 0xFF;
			cmd[2] = blocklen & 0xFF;
			cmd += 'C';
			arduboy.write(cmd);
			if (!waitWrite())
				return false;
			if (!waitRead())
				return false;
			if (arduboy.read(blocklen) != data.mid(block * BLOCKSIZE, blocklen))
			{
				errorString = "Verify failed!";
				return false;
			}
		}
		if (progress != nullptr)
			progress->setValue(block + 1);
		QCoreApplication::processEvents();
	}
	arduboy.write("x\x44", 2);
	if (!waitWrite())
		return false;
	if (!waitRead())
		return false;
	arduboy.read(1);
	return true;
}

QByteArray ArduManFX::readSketch()
{
	QByteArray sketch;
	if (!startBootloader())
		return QByteArray();
	if (!arduboy.isOpen())
	{
		errorString = "Arduboy not connected.";
		return QByteArray();
	}
	arduboy.write("A\x00\x00", 3);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	arduboy.read(1);
	arduboy.write("g\x70\x00\x46", 4);
	if (!waitWrite())
		return QByteArray();
	while (sketch.length() < 0x7000)
	{
		if (!waitRead())
			return QByteArray();
		sketch += arduboy.readAll();
	}
	return sketch;
}

bool ArduManFX::writeSketch(QString hexData, bool verify, bool fix1309, bool fixMicro, QProgressBar *progress)
{
	QVector<bool> flashPageUsed(256, false);
	bool overwritesCatarina = false;
	QByteArray data = encodeHexFile(hexData, flashPageUsed, &overwritesCatarina);
	if (data.isNull())
		return false;
	return writeSketch(data, flashPageUsed, verify, fix1309, fixMicro, progress, overwritesCatarina);
}

bool ArduManFX::writeSketch(QByteArray data, const QVector<bool> &flashPageUsed, bool verify, bool fix1309, bool fixMicro, QProgressBar *progress, bool overwritesCatarina)
{
	if (fix1309)
		applyLcd1309Patch(data);
	if (fixMicro)
		applyMicroPolarityPatch(data);
	if (!startBootloader())
		return false;
	if (getVersion() == 10)
	{
		arduboy.write("r", 1);
		if (!waitWrite())
			return false;
		if (!waitRead())
			return false;
		if ((arduboy.read(1)[0] & 0x10) != 0 && overwritesCatarina)
		{
			errorString = "This upload will most likely corrupt the bootloader.";
			return false;
		}
	}
	if (progress != nullptr)
	{
		progress->setMaximum(data.length() / 128);
		progress->setValue(0);
	}
	QCoreApplication::processEvents();
	for (int i = 0; i < data.length() / 128; ++i)
	{
		if (flashPageUsed[i])
		{
			QByteArray cmd(3, 0xFF);
			QByteArray page = data.mid(i * 128, 128);
			QByteArray data;
			cmd[0] = 'A';
			cmd[1] = i >> 2;
			cmd[2] = (i & 3) << 6;
			arduboy.write(cmd);
			if (!waitWrite())
				return false;
			if (!waitRead())
				return false;
			arduboy.read(1);
			arduboy.write("B\x00\x80\x46", 4);
			arduboy.write(page);
			if (!waitWrite())
				return false;
			if (!waitRead())
				return false;
			arduboy.read(1);
		}
		if (progress != nullptr)
			progress->setValue(i + 1);
		QCoreApplication::processEvents();
	}
	if (verify)
	{
		QByteArray sketch = readSketch();
		for (int i = 0; i < data.length() / 128; ++i)
		{
			if (flashPageUsed[i])
			{
				QByteArray pageWritten = data.mid(i * 128, 128);
				QByteArray pageRead = sketch.mid(i * 128, 128);
				if (pageWritten != pageRead)
				{
					errorString = QString("Verify failed at address %1.\nUpload Failed.").arg(i * 128);
					return false;
				}
			}
		}
	}
	return true;
}

QByteArray ArduManFX::createFlashCart(QList<FlashData> flashData, bool fix1309, bool fixMicro, QProgressBar *progress)
{
	QVector<bool> flashPageUsed(256, false);
	QByteArray flashCart;
	unsigned short previousPage = 0xFFFF;
	unsigned short currentPage = 0;
	unsigned short nextPage = 0;
	if (progress != nullptr)
	{
		progress->setMaximum(flashData.count());
		progress->setValue(0);
	}
	QCoreApplication::processEvents();
	for (int i = 0; i < flashData.count(); ++i)
	{
		QByteArray header = defaultHeader();
		QByteArray title = convertImage(flashData[i].titleImage);
		QByteArray program = encodeHexFile(flashData[i].hexData, flashPageUsed);
		QByteArray gameData = flashData[i].gameData + QByteArray((256 - flashData[i].gameData.length()) % 256, 0xFF);
		int programSize = program.length();
		int dataSize = gameData.length();
		int slotSize = ((programSize + dataSize) >> 8) + 5;
		int programPage = currentPage + 5;
		int dataPage = programPage + (programSize >> 8);
		if (fix1309)
			applyLcd1309Patch(program);
		if (fixMicro)
			applyMicroPolarityPatch(program);
		nextPage += slotSize;
		header[7] = flashData[i].listID;
		header[8] = previousPage >> 8;
		header[9] = previousPage & 0xFF;
		header[10] = nextPage >> 8;
		header[11] = nextPage & 0xFF;
		header[12] = slotSize >> 8;
		header[13] = slotSize & 0xFF;
		header[14] = programSize >> 7;
		if (programSize > 0)
		{
			header[15] = programPage >> 8;
			header[16] = programPage & 0xFF;
			if (dataSize > 0)
			{
				program[0x14] = 0x18;
				program[0x15] = 0x95;
				program[0x16] = dataPage >> 8;
				program[0x17] = dataPage & 0xFF;
			}
		}
		if (dataSize > 0)
		{
			header[17] = dataPage >> 8;
			header[18] = dataPage & 0xFF;
		}
		flashCart += header;
		flashCart += title;
		flashCart += program;
		flashCart += gameData;
		previousPage = currentPage;
		currentPage = nextPage;
		if (progress != nullptr)
			progress->setValue(i + 1);
		QCoreApplication::processEvents();
	}
	return flashCart;
}

QByteArray ArduManFX::convertImage(QImage image, bool encodeSize, int numFrames)
{
	QByteArray data;
	QRgb pixel;
	int frameWidth = image.width();
	int frameHeight = image.height() / numFrames;
	if (encodeSize)
	{
		data += frameWidth;
		data += frameHeight;
	}
	for (int y = 0; y < image.height(); y += 8)
	{
		for (int x = 0; x < image.width(); ++x)
		{
			char byte = 0x00;
			for (int bit = 0; bit < 8; ++bit)
			{
				pixel = image.pixel(x, y + bit);
				if (qRed(pixel) != 0 || qGreen(pixel) != 0 || qBlue(pixel) != 0)
					byte += 1 << bit;
			}
			data += byte;
		}
	}
	return data;
}

QList<ArduManFX::FlashData> ArduManFX::parseCSV(QString csvLocation)
{
	QList<FlashData> flashData;
	QFile file(csvLocation);
	QTextStream stream(&file);
	QStringList rows;
	QString basePath = QFileInfo(csvLocation).absolutePath();
	if (!file.open(QFile::ReadOnly|QFile::Text))
	{
		errorString = file.errorString();
		return flashData;
	}
	rows = stream.readAll().split('\n', QString::SkipEmptyParts);
	file.close();
	for (int row = 1; row < rows.count(); ++row)
	{
		QStringList rowData = rows[row].split(';');
		FlashData data;
		while (rowData.count() < 6)
			rowData += "";
		data.listID = rowData[0].toInt();
		data.description = rowData[1];
		fflush(stdout);
		if (rowData[2].isEmpty())
		{
			data.titleImage = QImage(128, 64, QImage::Format_RGB32);
			data.titleImage.fill(qRgb(0, 0, 0));
		}
		else
			data.titleImage = QImage(basePath + "/" + rowData[2]).convertToFormat(QImage::Format_RGB32);
		if (!rowData[3].isEmpty())
		{
			file.setFileName(basePath + "/" + rowData[3]);
			if (file.open(QFile::ReadOnly|QFile::Text))
			{
				data.hexData = stream.readAll();
				file.close();
			}
		}
		if (!rowData[4].isEmpty())
		{
			file.setFileName(basePath + "/" + rowData[4]);
			if (file.open(QFile::ReadOnly))
			{
				data.gameData = file.readAll();
				file.close();
			}
		}
		if (!rowData[5].isEmpty())
		{
			file.setFileName(basePath + "/" + rowData[5]);
			if (file.open(QFile::ReadOnly))
			{
				data.saveData = file.readAll();
				file.close();
			}
		}
		flashData += data;
	}
	errorString = "Empty/Invalid CSV file";
	return flashData;
}

QByteArray ArduManFX::defaultHeader()
{
	QByteArray header("ARDUBOY");
	header.append(249, 0xFF);
	return header;
}

QByteArray ArduManFX::encodeHexFile(const QString &hexData, QVector<bool> &flashPageUsed, bool *overwritesCatarina)
{
	QStringList records = hexData.split('\n', QString::SkipEmptyParts);
	QByteArray data(32768, 0xFF);
	int flashEnd = 0;
	for (int i = 0; i < records.count(); ++i)
	{
		QString rcd = records[i];
		if (rcd == ":00000001FF")
			break;
		if (rcd[0] == ':')
		{
			int rcd_len = rcd.mid(1, 2).toInt(nullptr, 16);
			int rcd_typ = rcd.mid(7, 2).toInt(nullptr, 16);
			int rcd_addr = rcd.mid(3, 4).toInt(nullptr, 16);
			int rcd_sum = rcd.mid(9+rcd_len*2, 2).toInt(nullptr, 16);
			if (rcd_typ == 0 && rcd_len > 0)
			{
				int flash_addr = rcd_addr;
				int checksum = rcd_sum;
				flashPageUsed[rcd_addr / 128] = true;
				flashPageUsed[(rcd_addr + rcd_len - 1) / 128] = true;
				for (int j = 1; j < 9 + rcd_len * 2; j += 2)
				{
					int byte = rcd.mid(j, 2).toInt(nullptr, 16);
					checksum = (checksum + byte) & 0xFF;
					if (j >= 9)
					{
						data[flash_addr] = byte;
						++flash_addr;
						if (flash_addr > flashEnd)
							flashEnd = flash_addr;
					}
				}
				if (checksum != 0)
				{
					errorString = "Error: Hex data contains errors.";
					return QByteArray();
				}
			}
		}
	}
	if (overwritesCatarina != nullptr)
	{
		int flashPageCount = 0;
		*overwritesCatarina = false;
		for (int i = 0; i < 256; ++i)
		{
			if (flashPageUsed[i])
			{
				++flashPageCount;
				if (i >= 224)
					*overwritesCatarina = true;
			}
		}
	}
	flashEnd = ((flashEnd + 255) / 256) * 256;
	return data.left(flashEnd);
}

bool ArduManFX::samePort(QSerialPortInfo port1, QSerialPortInfo port2)
{
	if (port1.vendorIdentifier() != port2.vendorIdentifier())
		return false;
	if (port1.productIdentifier() != port2.productIdentifier())
		return false;
	if (port1.portName() != port2.portName())
		return false;
	return true;
}

void ArduManFX::applyLcd1309Patch(QByteArray &data)
{
	int lcdBootProgram_addr = 0;
	while (lcdBootProgram_addr >= 0)
	{
		lcdBootProgram_addr = data.indexOf(lcdBootProgram, lcdBootProgram_addr);
		if (lcdBootProgram_addr >= 0)
		{
			data[lcdBootProgram_addr+2] = 0xE3;
			data[lcdBootProgram_addr+3] = 0xE3;
		}
	}
}

void ArduManFX::applyMicroPolarityPatch(QByteArray &data)
{
	for (int i = 0; i < data.length(); i += 2)
	{
		if (data[i] == '\x28' && data[i+1] == '\x98')
			data[i+1] = '\x9a';
		else if (data[i] == '\x28' && data[i+1] == '\x9a')
			data[i+1] = '\x98';
		else if (data[i] == '\x5d' && data[i+1] == '\x98')
			data[i+1] = '\x9a';
		else if (data[i] == '\x5d' && data[i+1] == '\x9a')
			data[i+1] = '\x98';
		else if (data[i] == '\x81' && data[i+1] == '\xef' && data[i+2] == '\x85' && data[i+3] == '\xb9')
			data[i] = '\x80';
		else if (data[i] == '\x84' && data[i+1] == '\xe2' && data[i+2] == '\x8b' && data[i+3] == '\xb9')
			data[i+1] = '\xe0';
	}
}

bool ArduManFX::isCompatibleDevice(quint16 vid, quint16 pid)
{
	for (size_t i = 0; i < sizeof(compatibleDevices) / sizeof(compatibleDevices[0]); i += 2)
	{
		if (compatibleDevices[i] == vid && compatibleDevices[i+1] == pid)
		{
			bootloaderActive = ((i / 2) & 1) == 0;
			return true;
		}
	}
	return false;
}

QSerialPortInfo ArduManFX::getComPort()
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	for (int i = 0; i < ports.count(); ++i)
	{
		if (ports[i].hasProductIdentifier() && ports[i].hasVendorIdentifier())
		{
			if (isCompatibleDevice(ports[i].vendorIdentifier(), ports[i].productIdentifier()))
				return ports[i];
		}
	}
	return QSerialPortInfo();
}

int ArduManFX::getVersion()
{
	if (!arduboy.isOpen())
	{
		errorString = "Arduboy not connected";
		return -1;
	}
	arduboy.write("V", 1);
	if (!waitWrite())
		return -1;
	if (!waitRead())
		return -1;
	return QString(arduboy.read(2)).toInt();
}

QByteArray ArduManFX::getJedecID()
{
	QByteArray jedec1, jedec2;
	if (!arduboy.isOpen())
	{
		errorString = "Arduboy not connected";
		return QByteArray();
	}
	arduboy.write("j", 1);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	jedec1 = arduboy.read(3);
	QThread::msleep(500);
	arduboy.write("j", 1);
	if (!waitWrite())
		return QByteArray();
	if (!waitRead())
		return QByteArray();
	jedec2 = arduboy.read(3);
	if (jedec1[0] != jedec2[0] || jedec1[1] != jedec2[1] || jedec1[2] != jedec2[2])
	{
		errorString = "No flash cart detected.";
		return QByteArray();
	}
	return jedec1;
}

bool ArduManFX::waitWrite(int msecs)
{
	if (arduboy.waitForBytesWritten(msecs))
		return true;
	errorString = QString("Failed to communicate with arduboy.\nReason: %1").arg(arduboy.errorString());
	return false;
}

bool ArduManFX::waitRead(int msecs)
{
	if (arduboy.waitForReadyRead(msecs))
		return true;
	errorString = QString("Failed to communicate with arduboy.\nReason: %1").arg(arduboy.errorString());
	return false;
}
