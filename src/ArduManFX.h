#ifndef ARDUMANFX_H
#define ARDUMANFX_H

#include <QtCore>
#include <QtSerialPort>
#include <QtGui>
#include <QProgressBar>

#define PAGESIZE 256
#define BLOCKSIZE 65536

class ArduManFX
{
	public:
		struct FlashData
		{
			QImage titleImage;
			QString description;
			QString hexData;
			QByteArray gameData;
			QByteArray saveData;
			int listID;
		};
		ArduManFX();
		~ArduManFX();
		bool connect();
		void disconnect();
		bool startBootloader();
		bool isConnected();
		QByteArray readEEPROM();
		bool writeEEPROM(QByteArray data, bool verify);
		QByteArray readFlashCart(QProgressBar *progress);
		bool writeFlashCart(QByteArray data, bool verify, QProgressBar *progress);
		QByteArray readSketch();
		bool writeSketch(QString hexData, bool verify, bool fix1309, bool fixMicro, QProgressBar *progress);
		bool writeSketch(QByteArray data, const QVector<bool> &flashPageUsed, bool verify, bool fix1309, bool fixMicro, QProgressBar *progress, bool overwritesCatarina=false);
		QSerialPortInfo getPortInfo() {return arduboyPort;}
		static QByteArray createFlashCart(QList<FlashData> flashData, bool fix1309, bool fixMicro, QProgressBar *progress);
		static QByteArray convertImage(QImage image, bool encodeSize=false, int numFrames=1);
		static QString getErrorString() {return errorString;}
		static QList<FlashData> parseCSV(QString csv);
	private:
		static QByteArray defaultHeader();
		static QByteArray encodeHexFile(const QString &hexData, QVector<bool> &flashPageUsed, bool *overwritesCatarina=nullptr);
		static bool samePort(QSerialPortInfo port1, QSerialPortInfo port2);
		static void applyLcd1309Patch(QByteArray &data);
		static void applyMicroPolarityPatch(QByteArray &data);
		bool isCompatibleDevice(quint16 vid, quint16 pid);
		QSerialPortInfo getComPort();
		int getVersion();
		QByteArray getJedecID();
		bool waitWrite(int msecs = 3000);
		bool waitRead(int msecs = 3000);
		static const quint16 compatibleDevices[];
		static const quint8 manufacturerIDs[];
		static const char *manufacturerNames[];
		static const char *lcdBootProgram;
		static QString errorString;
		QSerialPort arduboy;
		QSerialPortInfo arduboyPort;
		bool bootloaderActive;
};

#endif //ARDUMANFX_H
