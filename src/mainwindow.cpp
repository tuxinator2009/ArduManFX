#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QNetworkReply>
#include <QSslError>
#include <QSslConfiguration>
#include <QSslSocket>
#include <algorithm>
#include <random>
#include <cstdio>
#include <cstdlib>
#include "ArduManFX.h"
#include "game.h"
#include "mainwindow.h"
#include "progress.h"
#include "tester.h"

/*const char *MainWindow::categories[] =
{
	"ALL",
	"Shooter",
	"Puzzle",
	"Arcade",
	"Application",
	"Misc",
	"Racing",
	"Sports",
	"Action",
	"RPG",
	"Platformer",
	"Demo"
};
const int MainWindow::CATEGORY_ALL = 0;
const int MainWindow::CATEGORY_SHOOTER = 1;
const int MainWindow::CATEGORY_PUZZLE = 2;
const int MainWindow::CATEGORY_ARCADE = 3;
const int MainWindow::CATEGORY_APPLICATION = 4;
const int MainWindow::CATEGORY_MISC = 5;
const int MainWindow::CATEGORY_RACING = 6;
const int MainWindow::CATEGORY_SPORTS = 7;
const int MainWindow::CATEGORY_ACTION = 8;
const int MainWindow::CATEGORY_RPG = 9;
const int MainWindow::CATEGORY_PLATFORMER = 10;
const int MainWindow::CATEGORY_DEMO = 11;*/
const int MainWindow::SORT_TITLE_AZ = 0;
const int MainWindow::SORT_TITLE_ZA = 1;
const int MainWindow::SORT_RATING_HIGHLOW = 2;
const int MainWindow::SORT_RATING_LOWHIGH = 3;
const int MainWindow::SORT_DATE_NEWOLD = 4;
const int MainWindow::SORT_DATE_OLDNEW = 5;
const int MainWindow::SORT_RANDOM = 6;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	QNetworkRequest request(QUrl("https://arduboy.ried.cl/repo.json"));
	QString fileLocation = QCoreApplication::applicationDirPath() + "/erwinRepo.json";
	if (QFile::exists(fileLocation))
		request.setHeader(QNetworkRequest::IfNoneMatchHeader, getETag(fileLocation));
	setupUi(this);
	treeFlashCartBuilder->invisibleRootItem()->setFlags(Qt::ItemIsEnabled);
	net = new QNetworkAccessManager(this);
	reply = net->get(request);
	connect(reply, SIGNAL(readyRead()), this, SLOT(downloadRead()));
	connect(reply, SIGNAL(finished()), this, SLOT(loadErwinRepo()));
	Ui_MainWindow::statusBar->showMessage("Downloading Erwin's \"Unofficial\" Repo from https://arduboy.ried.cl/repo.json");
	this->setEnabled(false);
	//Uncomment these two lines for a release build
	btnRunTests->setVisible(false);
	wFlashCartBuilder->setEnabled(false);
	wFilter->setVisible(false);
	btnManageLocalRepo->setVisible(false);
	lblRepo->setVisible(false);
	cboxRepo->setVisible(false);
	//TODO: Load local repo
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_cboxCategory_currentIndexChanged(int index)
{
	for (int i = 0; i < gameWidgets.count(); ++i)
	{
		if (index == 0)
			gameWidgets[i]->setVisible(true);
		else
			gameWidgets[i]->setVisible(gameWidgets[i]->getGenre() == categories[index-1]);
	}
}

void MainWindow::on_cboxSort_currentIndexChanged(int index)
{
	QVBoxLayout *gameLayout = dynamic_cast<QVBoxLayout*>(wGameList->layout());
	std::random_device rd;

	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> dist(0, 1);
	std::sort(gameWidgets.begin(), gameWidgets.end(), [index,&dist,&mt](Game *a, Game *b)
	{
		if (index == SORT_TITLE_AZ)
			return a->getTitle() < b->getTitle();
		else if (index == SORT_TITLE_ZA)
			return a->getTitle() >= b->getTitle();
		else if (index == SORT_RATING_HIGHLOW)
		{
			if (a->getRating() > b->getRating())
				return true;
			else if (a->getRating() == b->getRating())
				return a->getNumVotes() > b->getNumVotes();
			return false;
		}
		else if (index == SORT_RATING_LOWHIGH)
		{
			if (a->getRating() < b->getRating())
				return true;
			else if (a->getRating() == b->getRating())
				return a->getNumVotes() < b->getNumVotes();
			return false;
		}
		else if (index == SORT_DATE_NEWOLD)
			return a->getDate() >= b->getDate();
		else if (index == SORT_DATE_OLDNEW)
			return a->getDate() < b->getDate();
		return dist(mt) == 1;
	});
	for (int i = 0; i < gameWidgets.count(); ++i)
	{
		gameLayout->removeWidget(gameWidgets[i]);
		gameLayout->insertWidget(i, gameWidgets[i]);
	}
}

void MainWindow::loadErwinRepo()
{
	QDir erwinRepo(QCoreApplication::applicationDirPath());
	QJsonParseError parseError;
	QJsonDocument doc;
	QJsonArray items;
	QString ratingURLString = "https://app.widgetpack.com/widget/rating/bootstrap?id=14914&chan=";
	Ui_MainWindow::statusBar->showMessage("Parsing Erwin's \"Unofficial\" Repo from https://arduboy.ried.cl/repo.json");
	QCoreApplication::processEvents();
	currentDownload += reply->readAll();
	saveETag(QCoreApplication::applicationDirPath() + "/erwinRepo.json", reply->header(QNetworkRequest::ETagHeader).toString());
	doc = QJsonDocument::fromJson(currentDownload, &parseError);
	items = doc["items"].toArray();
	if (!erwinRepo.exists("erwinRepo"))
		erwinRepo.mkdir("erwinRepo");
	erwinRepo.cd("erwinRepo");
	for (int i = 0; i < items.count(); ++i)
	{
		Game *gameWidget;
		DownloadFile download;
		Game::GameInfo info;
		QJsonObject game = items[i].toObject();
		QJsonObject binary;
		QJsonArray binaries = game["binaries"].toArray();
		QJsonArray screenshotArray = game["screenshots"].toArray();
		QString hexLocation;
		QString dataLocation;
		QStringList screenshots;
		info.title = game["title"].toString();
		info.author = game["author"].toString();
		info.description = game["description"].toString();
		info.date = game["date"].toString();
		info.url = QUrl(game["url"].toString(), QUrl::StrictMode);
		info.genre = game["genre"].toString();
		info.license = game["license"].toString();
		info.gameID = game["id"].toString();
		info.rating = 0.0;
		info.numVotes = -1;
		ratingURLString += info.gameID;
		if (i < items.count() - 1)
			ratingURLString += "%2C";
		if (!categories.contains(info.genre))
			categories += info.genre;
		//Search for Arduboy sketch
		for (int i = 0; i < binaries.count() && hexLocation.isEmpty(); ++i)
		{
			binary = binaries[i].toObject();
			if (binary["device"].toString().contains("Arduboy", Qt::CaseInsensitive))
				hexLocation = binary["filename"].toString();
		}
		//Search for ArduboyFX data
		for (int i = 0; i < binaries.count() && dataLocation.isEmpty(); ++i)
		{
			binary = binaries[i].toObject();
			if (binary["device"].toString().contains("ArduboyFX", Qt::CaseInsensitive))
			{
				dataLocation = binary["filename"].toString();
				info.fx = true;
			}
		}
		info.numScreenshots = screenshotArray.count();
		for (int i = 0; i < screenshotArray.count(); ++i)
			screenshots += screenshotArray[i].toString();
		if (!erwinRepo.exists(info.gameID))
			erwinRepo.mkdir(info.gameID);
		erwinRepo.cd(info.gameID);
		gameWidget = new Game(info);
		connect(gameWidget, SIGNAL(upload(QString, QString, QString)), this, SLOT(uploadGame(QString, QString, QString)));
		wGameList->layout()->addWidget(gameWidget);
		gameWidgets += gameWidget;
		download.game = gameWidget;
		gamesByID.insert(info.gameID, gameWidget);
		download.url = QUrl(game["banner"].toString(), QUrl::StrictMode);
		download.fileLocation = QString("%1/title.%2").arg(erwinRepo.path()).arg(download.url.fileName().section(".", -1));
		download.fileType = "title";
		if (!download.url.isValid())
			gameWidget->setTitleImage("");
		else
			downloadQueue.enqueue(download);
		if (QFile::exists(download.fileLocation))
			gameWidget->setTitleImage(download.fileLocation);
		download.url = QUrl(hexLocation, QUrl::StrictMode);
		download.fileLocation = erwinRepo.path() + "/sketch.hex";
		download.fileType = "sketch";
		if (QFile::exists(download.fileLocation))
			gameWidget->setHexLocation(download.fileLocation);
		downloadQueue.enqueue(download);
		if (info.fx)
		{
			download.url = QUrl(dataLocation, QUrl::StrictMode);
			download.fileLocation = erwinRepo.path() + "/data.bin";
			download.fileType = "data";
			if (QFile::exists(download.fileLocation))
				gameWidget->setDataLocation(download.fileLocation);
			downloadQueue.enqueue(download);
		}
		for (int i = 0; i < screenshots.count(); ++i)
		{
			download.url = QUrl(screenshots[i], QUrl::StrictMode);
			download.fileLocation = QString("%1/screenshot%2.%3").arg(erwinRepo.path()).arg(i).arg(download.url.fileName().section(".", -1));
			download.fileType = "screenshot";
			if (QFile::exists(download.fileLocation))
				gameWidget->addScreenshot(download.fileLocation);
			downloadQueue.enqueue(download);
		}
		erwinRepo.cdUp();
	}
	std::sort(categories.begin(), categories.end());
	for (int i = 0; i < categories.count(); ++i)
		cboxCategory->addItem(categories[i]);
	reply->close();
	reply->deleteLater();
	currentDownload.clear();
	reply = net->get(QNetworkRequest(QUrl(ratingURLString)));
	connect(reply, SIGNAL(readyRead()), this, SLOT(downloadRead()));
	connect(reply, SIGNAL(finished()), this, SLOT(loadErwinRatings()));
	Ui_MainWindow::statusBar->showMessage("Downloading Ratings from Erwin's \"Unofficial\" Repo");
}

void MainWindow::loadErwinRatings()
{
	QJsonParseError parseError;
	QJsonDocument doc;
	QJsonArray scores;
	Ui_MainWindow::statusBar->showMessage("Parsing Ratings from Erwin's \"Unofficial\" Repo");
	QCoreApplication::processEvents();
	currentDownload += reply->readAll();
	doc = QJsonDocument::fromJson(currentDownload, &parseError);
	scores = doc["score"].toArray();
	for (int i = 0; i < scores.count(); ++i)
	{
		QJsonObject score = scores[i].toObject();
		gameWidgets[i]->setRating(score["sum"].toInt(), score["count"].toInt());
	}
	if (downloadQueue.count() > 0)
		nextDownload();
	else
		Ui_MainWindow::statusBar->clearMessage();
	this->setEnabled(true);
}

void MainWindow::nextDownload()
{
	DownloadFile download = downloadQueue.head();
	QNetworkRequest request(download.url);
	currentDownload.clear();
	Ui_MainWindow::statusBar->showMessage(QString("Downloading %1").arg(download.url.fileName()));
	if (QFile::exists(download.fileLocation))
		request.setHeader(QNetworkRequest::IfNoneMatchHeader, getETag(download.fileLocation));
	reply = net->get(request);
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(readyRead()), this, SLOT(downloadRead()));
	connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void MainWindow::downloadError(QNetworkReply::NetworkError code)
{
	Ui_MainWindow::statusBar->showMessage(QString("Network Error: %1").arg(code));
	reply->abort();
}

void MainWindow::downloadRead()
{
	currentDownload += reply->readAll();
}

void MainWindow::downloadFinished()
{
	DownloadFile download = downloadQueue.dequeue();
	QFile file(download.fileLocation);
	QTextStream stream(&file);
	QIODevice::OpenMode mode = QIODevice::WriteOnly;
	saveETag(download.fileLocation, reply->header(QNetworkRequest::ETagHeader).toString());
	if (reply->error())
	{
		fprintf(stdout, "Download error %d: %s\n", reply->error(), download.url.toString().toLocal8Bit().data());
		fflush(stdout);
		return;
	}
	if (!currentDownload.isEmpty())
	{
		if (download.fileType == "sketch")
			mode |= QIODevice::Text;
		if (file.open(mode))
		{
			if (download.fileType == "sketch")
				stream << currentDownload;
			else
				file.write(currentDownload);
			file.close();
		}
		else
			QMessageBox::critical(this, "Save Failed", QString("Failed to save file: %1\nReason: %2").arg(download.fileLocation).arg(file.errorString()));
		if (QFile::exists(download.fileLocation))
		{
			if (download.fileType == "title")
				download.game->setTitleImage(download.fileLocation);
			else if (download.fileType == "sketch")
				download.game->setHexLocation(download.fileLocation);
			else if (download.fileType == "data")
				download.game->setDataLocation(download.fileLocation);
			else if (download.fileType == "screenshot")
				download.game->addScreenshot(download.fileLocation);
		}
	}
	reply->close();
	reply->deleteLater();
	if (downloadQueue.count() > 0)
		nextDownload();
	else
		Ui_MainWindow::statusBar->clearMessage();
}

void MainWindow::on_btnRunTests_clicked()
{
	Tester *tester = new Tester(this);
	tester->exec();
	tester->deleteLater();
}

void MainWindow::uploadGame(QString title, QString hexLocation, QString dataLocation)
{
	Progress *progress;
	QFile file;
	QTextStream stream(&file);
	QString hex;
	QByteArray data;
	if (!dataLocation.isEmpty())
	{
		QList<ArduManFX::FlashData> flashCart;
		ArduManFX::FlashData flashData;
		flashData.listID = 0;
		flashData.titleImage = QImage(128,64, QImage::Format_RGB32);
		flashData.titleImage.fill(qRgb(0, 0, 0));
		flashData.description = title;
		file.setFileName(hexLocation);
		if (!file.open(QFile::ReadOnly|QFile::Text))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open \"%1\" in read-only mode.\nReason: %2").arg(hexLocation).arg(file.errorString()));
			return;
		}
		flashData.hexData = stream.readAll();
		file.close();
		file.setFileName(dataLocation);
		if (!file.open(QFile::ReadOnly|QFile::Text))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open \"%1\" in read-only mode.\nReason: %2").arg(dataLocation).arg(file.errorString()));
			return;
		}
		flashData.gameData = file.readAll();
		file.close();
		flashCart += flashData;
		data = ArduManFX::createFlashCart(flashCart, chkFix1309->isChecked(), chkFixMicro->isChecked(), nullptr);
		if (!arduboy.connect())
		{
			QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
			return;
		}
		progress = new Progress(QString("Uploading %1").arg(title), arduboy.getPortInfo(), this);
		progress->show();
		if (!arduboy.writeFlashCart(data, chkVerify->isChecked(), progress->getProgressBar()))
			QMessageBox::critical(this, "Upload Error", QString("Failed to write data to device.\nReason: %1").arg(ArduManFX::getErrorString()));
		progress->close();
		progress->deleteLater();
	}
	else
	{
		file.setFileName(hexLocation);
		if (!file.open(QFile::ReadOnly|QFile::Text))
		{
			QMessageBox::critical(this, "File I/O Error", QString("Failed to open \"%1\" in read-only mode.\nReason: %2").arg(hexLocation).arg(file.errorString()));
			return;
		}
		hex = stream.readAll();
		file.close();
		if (!arduboy.connect())
		{
			QMessageBox::critical(this, "Connection Failed", ArduManFX::getErrorString());
			return;
		}
		progress = new Progress(QString("Uploading %1").arg(title), arduboy.getPortInfo(), this);
		progress->show();
		if (!arduboy.writeSketch(hex, chkVerify->isChecked(), chkFix1309->isChecked(), chkFixMicro->isChecked(), progress->getProgressBar()))
			QMessageBox::critical(this, "Upload Error", QString("Failed to write sketch to device.\nReason: %1").arg(ArduManFX::getErrorString()));
		arduboy.disconnect();
		progress->close();
		progress->deleteLater();
	}
}

QString MainWindow::getETag(QString fileLocation)
{
	QFile file(fileLocation + ".etag");
	QTextStream stream(&file);
	QString etag;
	if (file.open(QFile::ReadOnly|QFile::Text))
	{
		etag = stream.readAll();
		file.close();
	}
	return etag;
}

void MainWindow::saveETag(QString fileLocation, QString etag)
{
	QFile file(fileLocation + ".etag");
	QTextStream stream(&file);
	if (file.open(QFile::WriteOnly|QFile::Text))
	{
		stream << etag;
		file.close();
	}
}
