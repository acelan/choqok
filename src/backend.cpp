//
// C++ Implementation: backend
//
// Description: the Backend of choqoK
//
//
// Author:  Mehrdad Momeny (C) 2008
// Email Address: mehrdad.momeny[AT]gmail.com
// Copyright: GNU GPL v3
//
//
#include "backend.h"

#include <KDE/KLocale>
#include <QDomDocument>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurl.h>
#include "settings.h"

Backend::Backend(QObject* parent): QObject(parent)
{
	kDebug();
	urls[HomeTimeLine] = "http://twitter.com/statuses/friends_timeline.xml";
	urls[ReplyTimeLine] = "http://twitter.com/statuses/replies.xml";
	urls[UserTimeLine] = "http://twitter.com/statuses/user_timeline.xml";
	login();	
	
}

Backend::~Backend()
{
	kDebug();
	logout();
}

void Backend::postNewStatus(const QString & statusMessage, uint replyToStatusId)
{
	kDebug();
	KUrl url("http://twitter.com/statuses/update.xml");
	url.setUser(Settings::username());
	url.setPass(Settings::password());
	QByteArray data = "status=";
	data += QUrl::toPercentEncoding(statusMessage);
	if(replyToStatusId!=0)
		data += "&in_reply_to_status_id=" + QString::number(replyToStatusId);
	data += "&source=choqok";
	KIO::TransferJob *job = KIO::http_post(url, data, KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
	
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotPostNewStatusFinished(KJob*)) );
	
	job->start();
}

void Backend::login()
{
	
}

void Backend::logout()
{
}

void Backend::requestTimeLine(TimeLineType type, int page)
{
	kDebug();
	KUrl url(urls[type]);
	url.setUser(Settings::username());
	url.setPass(Settings::password());
	url.setQuery(Settings::latestStatusId() ? "?since_id=" + QString::number(Settings::latestStatusId()) : QString());
	kDebug()<<"Latest status Id: "<<Settings::latestStatusId();
	

	KIO::TransferJob *job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http GET request!";
		QString errMsg = i18n("Cannot create a http GET request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	mRequestTimelineMap[job] = type;
	mRequestTimelineBuffer[job] = QByteArray();
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestTimelineFinished(KJob*)));
	connect( job, SIGNAL(data( KIO::Job *, const QByteArray &)), this, SLOT(slotRequestTimelineData(KIO::Job*, const QByteArray&)));
	job->start();
}

QDateTime Backend::dateFromString(const QString &date)
{
	QDateTime datetime = QDateTime::fromString(date, "ddd MMM dd hh:mm:ss '+0000' yyyy");
	datetime.setTimeSpec(Qt::UTC);
	return datetime.toLocalTime();
}

QList<Status> * Backend::readTimeLineFromXml(const QByteArray & buffer)
{
	kDebug();
	QDomDocument document;
	QList<Status> *statusList = new QList<Status>;
	
	document.setContent(buffer);
	
	QDomElement root = document.documentElement();
	
	if (root.tagName() != "statuses") {
		QString err = i18n("Data returned from server corrupted!");
		kDebug()<<"there's no statuses tag in XML\t the XML is: \n"<<buffer.data();
		mLatestErrorString = err;
		return 0;
	}
		QDomNode node = root.firstChild();
		QString timeStr;
		while (!node.isNull()) {
			if (node.toElement().tagName() != "status") {
				kDebug()<<"there's no status tag in XML, maybe there is no new status!";
				return statusList;
			}
				QDomNode node2 = node.firstChild();
				Status status;
				while (!node2.isNull()) {
					if(node2.toElement().tagName() == "created_at")
						timeStr = node2.toElement().text();
					else if(node2.toElement().tagName() == "text")
						status.content = node2.toElement().text();
					else if(node2.toElement().tagName() == "id")
						status.statusId = node2.toElement().text().toInt();
					else if(node2.toElement().tagName() == "in_reply_to_status_id")
						status.replyToStatusId = node2.toElement().text().toULongLong();
					else if(node2.toElement().tagName() == "in_reply_to_user_id")
						status.replyToUserId = node2.toElement().text().toULongLong();
					else if(node2.toElement().tagName() == "in_reply_to_screen_name")
						status.replyToUserScreenName = node2.toElement().text();
					else if(node2.toElement().tagName() == "source")
						status.source = node2.toElement().text();
					else if(node2.toElement().tagName() == "truncated")
						status.isTruncated = (node2.toElement().text() == "true")? true : false;
					else if(node2.toElement().tagName() == "favorited")
						status.isFavorited = (node2.toElement().text() == "true")? true : false;
					else if(node2.toElement().tagName() == "user"){
						QDomNode node3 = node2.firstChild();
						while (!node3.isNull()) {
							if (node3.toElement().tagName() == "screen_name") {
								status.user.screenName = node3.toElement().text();
							} else if (node3.toElement().tagName() == "profile_image_url") {
								status.user.profileImageUrl = node3.toElement().text();
							} else if (node3.toElement().tagName() == "id") {
								status.user.userId = node3.toElement().text().toUInt();
							} else if (node3.toElement().tagName() == "name") {
								status.user.name = node3.toElement().text();
							}
							node3 = node3.nextSibling();
						}
					}
					node2 = node2.nextSibling();
				}
				node = node.nextSibling();
				status.creationDateTime = dateFromString(timeStr);
// 				 = QDateTime(time.date(), time.time(), Qt::UTC);
				statusList->insert(0, status);
			}
	return statusList;
}

void Backend::abortPostNewStatus()
{
	kDebug()<<"Not implemented yet!";
// 	statusHttp.abort();
}

QString& Backend::latestErrorString()
{
	return mLatestErrorString;
}

void Backend::requestFavorited(uint statusId, bool isFavorite)
{
	kDebug();
	KUrl url;
	if(isFavorite){
		url.setUrl("http://twitter.com/favorites/create/"+QString::number(statusId)+".xml");
	
	} else {
		url.setUrl("http://twitter.com/favorites/destroy/"+QString::number(statusId)+".xml");
	}
	url.setUser(Settings::username());
	url.setPass(Settings::password());
	
	KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestFavoritedFinished(KJob*)) );
	
	job->start();
}

void Backend::requestDestroy(uint statusId)
{
	kDebug();
	KUrl url("http://twitter.com/statuses/destroy/"+QString::number(statusId)+".xml");
	
	url.setUser(Settings::username());
	url.setPass(Settings::password());
	
	KIO::TransferJob *job = KIO::http_post(url, QByteArray(), KIO::HideProgressInfo) ;
	if(!job){
		kDebug()<<"Cannot create a http POST request!";
		QString errMsg = i18n("Cannot create a http POST request, please check your internet connection.");
		emit sigError(errMsg);
		return;
	}
	
	connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestDestroyFinished(KJob*)) );
	
	job->start();
}

void Backend::quiting()
{
}

void Backend::slotPostNewStatusFinished(KJob * job)
{
	kDebug();
	if(job->error()==0){//No error occured
		emit sigPostNewStatusDone(false);
	} else {
		mLatestErrorString = job->errorText();
		emit sigPostNewStatusDone(true);
	}
}

void Backend::slotRequestTimelineFinished(KJob *job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer";
		return;
	}
	if(job->error()){
		mLatestErrorString = job->errorText();
		kDebug()<<mLatestErrorString;
		emit sigError(mLatestErrorString);
		return;
	}
	QList<Status> *ptr = readTimeLineFromXml(mRequestTimelineBuffer[ job ].data());
	switch(mRequestTimelineMap.value(job)){
	case HomeTimeLine:
		if(ptr){
			emit homeTimeLineRecived(*ptr);
		} else {
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
		}
		break;
	case ReplyTimeLine:
		if(ptr)
			emit replyTimeLineRecived(*ptr);
		else
			kDebug()<<"Null returned from Backend::readTimeLineFromXml()";
		break;
	default:
		kDebug()<<"The returned job isn't in Map!";
		break;
	};
}

void Backend::slotRequestTimelineData(KIO::Job * job, const QByteArray & data)
{
	kDebug();
	if( !job ) {
		kError() << "Job is a null pointer.";
		return;
	}
	unsigned int oldSize = mRequestTimelineBuffer[ job ].size();
	mRequestTimelineBuffer[ job ].resize( oldSize + data.size() );
	memcpy( mRequestTimelineBuffer[ job ].data() + oldSize, data.data(), data.size() );
}

void Backend::slotRequestFavoritedFinished(KJob * job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer.";
		return;
	}
	if(job->error()){
			mLatestErrorString = job->errorText();
			emit sigFavoritedDone(true);
			return;
		} else
			emit sigFavoritedDone(false);
}

void Backend::slotRequestDestroyFinished(KJob * job)
{
	kDebug();
	if(!job){
		kDebug()<<"Job is null pointer.";
		return;
	}
	if(job->error()){
		mLatestErrorString = job->errorText();
		emit sigDestroyDone(true);
		return;
	} else
		emit sigDestroyDone(false);
}


#include "backend.moc"
