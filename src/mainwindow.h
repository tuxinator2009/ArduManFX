#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkReply>
//#include <angelscript.h>
#include "ui_mainwindow.h"
#include "ArduManFX.h"
#include "game.h"

#define DEBUG_MESSAGE "%s: %d\n", __FILE__, __LINE__
#define PRINT_DEBUG_MESSAGE() fprintf(stderr, DEBUG_MESSAGE);fflush(stderr);

class MainWindow : public QMainWindow, protected Ui::MainWindow
{
	Q_OBJECT
	public:
		MainWindow(QWidget *parent=nullptr);
		virtual ~MainWindow();
	protected slots:
		void on_cboxCategory_currentIndexChanged(int index);
		void on_cboxSort_currentIndexChanged(int index);
		void loadErwinRepo();
		void loadErwinRatings();
		void nextDownload();
		void downloadError(QNetworkReply::NetworkError code);
		void downloadRead();
		void downloadFinished();
		void on_btnRunTests_clicked();
		void uploadGame(QString title, QString hexLocation, QString dataLocation);
	private:
		struct DownloadFile
		{
			QUrl url;
			QString fileLocation;
			QString fileType;
			Game *game;
		};
		QString getETag(QString fileLocation);
		void saveETag(QString fileLocation, QString etag);
		QNetworkAccessManager *net;
		QNetworkReply *reply;
		QByteArray currentDownload;
		QQueue<DownloadFile> downloadQueue;
		QList<Game*> gameWidgets;
		QList<ArduManFX::FlashData> flashCart;
		QMap<QString, Game*> gamesByID;
		QStringList categories;
		ArduManFX arduboy;
		//static const char *categories[];
		static const int CATEGORY_ALL;
		static const int CATEGORY_SHOOTER;
		static const int CATEGORY_PUZZLE;
		static const int CATEGORY_ARCADE;
		static const int CATEGORY_APPLICATION;
		static const int CATEGORY_MISC;
		static const int CATEGORY_RACING;
		static const int CATEGORY_SPORTS;
		static const int CATEGORY_ACTION;
		static const int CATEGORY_RPG;
		static const int CATEGORY_PLATFORMER;
		static const int CATEGORY_DEMO;
		static const int SORT_TITLE_AZ;
		static const int SORT_TITLE_ZA;
		static const int SORT_RATING_HIGHLOW;
		static const int SORT_RATING_LOWHIGH;
		static const int SORT_DATE_NEWOLD;
		static const int SORT_DATE_OLDNEW;
		static const int SORT_RANDOM;
};

#endif //MAINWINDOW_H
