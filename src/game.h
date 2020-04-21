#ifndef GAME_H
#define GAME_H

#include <QtGui>
#include <QtCore>
#include "ui_game.h"

class Game : public QWidget, protected Ui::Game
{
	Q_OBJECT
	public:
		struct GameInfo
		{
			QString title;
			QString description;
			QString author;
			QString license;
			QString date;
			QString genre;
			QString gameID;
			QUrl url;
			float rating;
			int numVotes;
			int numScreenshots;
			bool fx;
		};
		Game(const GameInfo &info, QWidget *parent=nullptr);
		~Game();
		void setHexLocation(QString location);
		void setDataLocation(QString location);
		void setTitleImage(QString imageLocation);
		void addScreenshot(QString imageLocation);
		void setRating(int total, int votes);
		QString getTitle() {return lblTitle->text();}
		float getRating() {return rating;}
		int getNumVotes() {return numVotes;}
		QString getDate() {return lblDate->text();}
		QString getGenre() {return lblGenre->text();}
		QString getGameID() {return gameID;}
	signals:
		void upload(QString title, QString hexLocation, QString dataLocation);
	protected slots:
		void on_btnMoreInfo_clicked();
		void on_btnScreenshots_clicked();
		void on_btnUpload_clicked();
	private:
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		QUrl url;
		QStringList screenshots;
		QString hexLocation;
		QString dataLocation;
		QString gameID;
		QPoint dragStartPosition;
		float rating;
		int numVotes;
		int numScreenshots;
		bool fx;
};

#endif //GAME_H
