//============================================================================
// Name        : DREpubInterface.h
// Author      : akira.1642@gmail.com
//This is free and unencumbered software released into the public domain.
//The detailed description is in UNLICENSE.
//============================================================================

#ifndef DREPUBINTERFACE_H_
#define DREPUBINTERFACE_H_

#include <QObject>
#include <QWebPage>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class QBuffer;
class ReadiumSdk;

namespace ePub3{
	class Package;
}

class QWebFrame;

class DREpubInterface : public QObject
{
	Q_OBJECT

public Q_SLOTS:
	void AddJSObject();
	//void CallJS();

public:
	DREpubInterface(ReadiumSdk *sdk, QWebFrame *pWebFrame);
	Q_INVOKABLE	void onPaginationChanged(QString str);
	Q_INVOKABLE	void LoadFile();
	void CallJS_OpenBook(std::string& pckgJson);

private:
	QWebFrame *m_pWebFrame;
	ReadiumSdk *m_pSdk;
};

class DRWebPage : public QWebPage
{
	Q_OBJECT

public:
	DRWebPage(ReadiumSdk *sdk, QObject *parent = 0);
	virtual void	javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID );

//protected:
//    virtual bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

public Q_SLOTS:
    bool shouldInterruptJavaScript();

private:
	ReadiumSdk *m_pSdk;
};

class DRNetworkAccessManager : public QNetworkAccessManager
{
	Q_OBJECT

	//TODO: remove default constructor
public:
	DRNetworkAccessManager(ReadiumSdk *sdk, QObject *parent = 0);

protected:
	//all public functions (get, post, sendCustomRequest) call this function to process
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                         QIODevice *outgoingData = 0);
private:
	ReadiumSdk *m_pSdk;
};

class DRNetworkReply: public QNetworkReply
{
    Q_OBJECT

public:
    DRNetworkReply( QObject *parent=0 );
    ~DRNetworkReply();

//    void setHttpStatusCode( int code, const QByteArray &statusText = QByteArray() );
    void setHeader( QNetworkRequest::KnownHeaders header, const QVariant &value );
    void setContentType( const QByteArray &contentType );

    void setContent( const QString &content );
    void setContent( const QByteArray &content );

    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char *data, qint64 maxSize);

private:
    struct DRNetworkReplyPrivate *d;
};

#endif /* DREPUBINTERFACE_H_ */
