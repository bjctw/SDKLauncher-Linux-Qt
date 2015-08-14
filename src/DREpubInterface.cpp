//============================================================================
// Name        : DREpubInterface.cpp
// Author      : akira.1642@gmail.com
//This is free and unencumbered software released into the public domain.
//The detailed description is in UNLICENSE.
//============================================================================

#include "DREpubInterface.h"
#include "sdkHelper.h"

#include <QTimer>
#include <QBuffer>
#include <QNetworkRequest>
#include <QString>
#include <QWebFrame>
#include <QFileDialog>
#include <QDebug>

#define	REDIRECT_PATH	"epubVirtual"

DREpubInterface::DREpubInterface(ReadiumSdk *sdk, QWebFrame *pWebFrame)
{
	m_pSdk = sdk;
	m_pWebFrame = pWebFrame;
	connect(pWebFrame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(AddJSObject()));
	//res = connect(pWebFrame, SIGNAL(loadFinished(bool)), this, SLOT(CallJS()));
	//qDebug() << "Connect" << res;
}

void DREpubInterface::onPaginationChanged(QString str)
{
	qDebug() << "onPaginationChanged. Received data:\n" << str;
}

void DREpubInterface::LoadFile()
{
	QString file = QFileDialog::getOpenFileName(0, "Open ePub File", "", "ePub Files (*.epub)");
	if(file.isNull())	//cancel
		return;

	m_pSdk->openEpubFile(file.toUtf8().data());

    Json::Value root = m_pSdk->packageToJason();
    Json::StyledWriter writer;
    std::string  pckgJson = writer.write( root );
    //cout << packStr.c_str();

    CallJS_OpenBook(pckgJson);

}

void DREpubInterface::AddJSObject()
{
	m_pWebFrame->addToJavaScriptWindowObject("DREpubInterface", this);
}

void DREpubInterface::CallJS_OpenBook(std::string& pckgJson)
{
    QString jsfunc = QString("openBook(%1)").arg(pckgJson.c_str());

	//m_pWebFrame->evaluateJavaScript("testFunc()");
	m_pWebFrame->evaluateJavaScript(jsfunc);
}

DRWebPage::DRWebPage(ReadiumSdk *sdk, QObject *parent)
	:QWebPage(parent)
{
	m_pSdk = sdk;
}

void DRWebPage::javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID)
{
	qDebug() << "msg: " << message;
	qDebug() << "lineNumber: " << lineNumber;
	qDebug() << "sourceID: " << sourceID;
}

bool DRWebPage::shouldInterruptJavaScript()
{
	return false;	//avoid timeout pop-up message
}

//bool DRWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
//{
//	return true;	//redirect handle by network manager
//
//	if(m_package == nullptr)
//	{
//		printf("[ERROR] Package pointer is not set.\n");
//		return true;
//	}
//
//	qDebug() << "URL: " << request.url() << "rawHeaderList: " << request.rawHeaderList() << "type: " << type;
//
//	QString path = request.url().path();
//	int nIndex = path.indexOf(REDIRECT_PATH);
//	if(nIndex != -1)
//	{
//		//redirect url
//		qDebug() << "REDIRECT!";
//		//TODO: async
//		QString relativePath = path.right(path.length()-(nIndex+strlen(REDIRECT_PATH)+1));
//		qDebug() << "relative path: " << relativePath;
//		char *buf;
//		int nSize = getContent(m_package, relativePath.toUtf8().data(), &buf);
//
//		QByteArray data = QByteArray::fromRawData(buf, nSize);
//		frame->setContent(data, QString("application/xhtml+xml"));
//	}
//	else
//		return true;	//normal url
//
//	return false;
//}

DRNetworkAccessManager::DRNetworkAccessManager(ReadiumSdk *sdk, QObject *parent)
	:QNetworkAccessManager(parent)
{
	m_pSdk = sdk;
}

QNetworkReply *DRNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
	qDebug() << "op" << op << "request url" << request.url();

	ePub3::Package* package = m_pSdk->getPackagePtr();

	if(package == nullptr)
	{
		qDebug() << __FUNCTION__ << ">> Package pointer is not set!";
	}

	QString path = request.url().path();
    if ( !path.contains(REDIRECT_PATH) &&
    		(package == nullptr	//
    		|| path.startsWith("/res/host_app/") )
    		)
    {
    	//TODO: use more specified path name
    	// host skeleton and js files
    	qDebug() << "Forwarding...";
        QNetworkReply *reply = QNetworkAccessManager::createRequest( op, request, outgoingData );
        return reply;
    }
    else
    {
    	qDebug() << "Extracting...";
    	//TODO: cache?
    	// files in epub
    	DRNetworkReply *reply = new DRNetworkReply();

    	//find MIME type
    	QString mimeType;
    	//TODO: should ask Readium?
    	if(request.url().path().endsWith("html"))
    		mimeType = "application/xhtml+xml";
    	else if(request.url().path().endsWith("css"))
    		mimeType = "text/css";
    	else if(request.url().path().endsWith("jpeg") || request.url().path().endsWith("jpg") )
    		mimeType = "image/jpeg";
    	else if(request.url().path().endsWith("png"))
    		mimeType = "image/png";
    	else
    	{
    		qDebug() << "Cannot find MIME type! URL: " << request.url();
    	}

        reply->setContentType(mimeType.toUtf8());


		//TODO: async
        QString relativePath;
		int nIndex = path.indexOf(REDIRECT_PATH);
		if(nIndex != -1)
			relativePath = path.right(path.length()-(nIndex+strlen(REDIRECT_PATH)+1));
		else
			relativePath = path.right(path.length()-4);	//remove "/res/"


		char *buf;
		qDebug() << "relative path: " << relativePath;
		int nSize = m_pSdk->getContent(relativePath.toUtf8().data(), &buf);

		QByteArray data = QByteArray::fromRawData(buf, nSize);
        reply->setContent(data);

        return reply;
    }

}

struct DRNetworkReplyPrivate
{
    QByteArray content;
    qint64 offset;
};

DRNetworkReply::DRNetworkReply( QObject *parent )
    : QNetworkReply(parent)
{
    d = new DRNetworkReplyPrivate;
}

DRNetworkReply::~DRNetworkReply()
{
    delete d;
}

//void DRNetworkReply::setHttpStatusCode( int code, const QByteArray &statusText )
//{
//    setAttribute( QNetworkRequest::HttpStatusCodeAttribute, code );
//    if ( statusText.isNull() )
//        return;
//
//    setAttribute( QNetworkRequest::HttpReasonPhraseAttribute, statusText );
//}

void DRNetworkReply::setHeader( QNetworkRequest::KnownHeaders header, const QVariant &value )
{
    QNetworkReply::setHeader( header, value );
}

void DRNetworkReply::setContentType( const QByteArray &contentType )
{
    setHeader(QNetworkRequest::ContentTypeHeader, contentType);
}

void DRNetworkReply::setContent( const QString &content )
{
    setContent(content.toUtf8());
}

void DRNetworkReply::setContent( const QByteArray &content )
{
    d->content = content;
    d->offset = 0;

    open(ReadOnly | Unbuffered);
    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(content.size()));

    QTimer::singleShot( 0, this, SIGNAL(readyRead()) );
    QTimer::singleShot( 0, this, SIGNAL(finished()) );
}

void DRNetworkReply::abort()
{
    // Do nothing
}


qint64 DRNetworkReply::bytesAvailable() const
{
    return d->content.size() - d->offset + QIODevice::bytesAvailable();
}

bool DRNetworkReply::isSequential() const
{
    return true;
}


qint64 DRNetworkReply::readData(char *data, qint64 maxSize)
{
    if (d->offset >= d->content.size())
        return -1;

    qint64 number = qMin(maxSize, d->content.size() - d->offset);
    memcpy(data, d->content.constData() + d->offset, number);
    d->offset += number;

    return number;
}

