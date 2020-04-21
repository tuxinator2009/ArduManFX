#include <QtGui>
#include <QtCore>
#include "ArduManFX.h"
#include "game.h"

Game::Game(const GameInfo &info, QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	lblTitle->setText(info.title);
	lblAuthor->setText(info.author);
	lblDescription->setText(info.description);
	lblLicense->setText(info.license);
	lblDate->setText(info.date);
	lblGenre->setText(info.genre);
	gameID = info.gameID;
	url = info.url;
	rating = info.rating;
	numVotes = info.numVotes;
	numScreenshots = info.numScreenshots;
	fx = info.fx;
	lblRating->setMinimumWidth((int)(rating / 5.0 * 80));
	lblRating->setMaximumWidth((int)(rating / 5.0 * 80));
	lblRating->setToolTip(QString("%1 / 5.0 - %2 %3").arg(rating, 0, 'g', 2).arg(numVotes).arg((numVotes == 1) ? "vote":"votes"));
	if (numVotes != -1)
		lblNotImplemented->setVisible(false);
	lblFX->setVisible(info.fx);
	if (info.fx)
		btnUpload->setText("UploadFX");
	btnMoreInfo->setEnabled(url.isValid());
	btnScreenshots->setEnabled(false);
	btnUpload->setEnabled(false);
}

Game::~Game()
{
}

void Game::setHexLocation(QString location)
{
	hexLocation = location;
	if (!fx)
		btnUpload->setEnabled(true);
}

void Game::setDataLocation(QString location)
{
	dataLocation = location;
	btnUpload->setEnabled(true);
}

void Game::setTitleImage(QString imageLocation)
{
	lblTitleImage->setPixmap(QPixmap(imageLocation).scaled(256, 128, Qt::KeepAspectRatio));
}

void Game::addScreenshot(QString imageLocation)
{
	screenshots += imageLocation;
	btnScreenshots->setEnabled(numScreenshots == screenshots.count());
}

void Game::setRating(int total, int votes)
{
	rating = (float)total / (float)votes;
	numVotes = votes;
	lblRating->setMinimumWidth((int)(rating / 5.0 * 80));
	lblRating->setMaximumWidth((int)(rating / 5.0 * 80));
	lblRating->setToolTip(QString("%1 / 5.0 - %2 %3").arg(rating, 0, 'g', 2).arg(numVotes).arg((numVotes == 1) ? "vote":"votes"));
	if (numVotes != -1)
		lblNotImplemented->setVisible(false);
}

void Game::on_btnMoreInfo_clicked()
{
	QDesktopServices::openUrl(url);
}

void Game::on_btnScreenshots_clicked()
{
	//TODO: Launch screenshot viewer with list of screenshots
}

void Game::on_btnUpload_clicked()
{
	emit upload(lblTitle->text(), hexLocation, dataLocation);
}

void Game::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		dragStartPosition = event->pos();
}

void Game::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
		return;
	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	QLabel *label = new QLabel(lblTitle->text(), nullptr);
	QPixmap pixmap;
	QPoint hotSpot(2, 2);
	label->setAutoFillBackground(true);
	label->setFrameShape(QFrame::Panel);
	label->setFrameShadow(QFrame::Raised);
	pixmap = QPixmap(label->sizeHint());
	label->render(&pixmap);
	label->deleteLater();
	mimeData->setText(lblTitle->text());
	mimeData->setImageData(lblTitleImage->pixmap()->toImage());
	mimeData->setData("ardumanfx-title", lblTitle->text().toLocal8Bit());
	mimeData->setData("ardumanfx-gameID", gameID.toLocal8Bit());
	mimeData->setData("ardumanfx-hex", hexLocation.toLocal8Bit());
	mimeData->setData("ardumanfx-bin", dataLocation.toLocal8Bit());
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);
	drag->setHotSpot(hotSpot);
	drag->exec(Qt::CopyAction);
	drag->deleteLater();
}
