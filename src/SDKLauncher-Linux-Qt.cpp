//============================================================================
// Name        : SDKLauncher-Linux-Qt.cpp
// Author      : akira.1642@gmail.com
// Description : Testbed of readium SDK
//This is free and unencumbered software released into the public domain.
//The detailed description is in UNLICENSE.
//============================================================================

#include "DREpubInterface.h"
#include "sdkHelper.h"

#include <cstddef>

#include <QApplication>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QWebSettings>
#include <QWebInspector>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QDebug>
#include <string>
#include <string.h>
using namespace std;


#define READER_SKELETON	"http://127.0.0.1/res/host_app/myReader.html"
//#define READER_SKELETON	"res/myReader.html"
//#define TEST_EPUB	"res/test_ja.epub"
//#define TEST_EPUB	"res/test_en.epub"
#define TEST_EPUB	"res/page-blanche.epub"
//#define TEST_EPUB	"res/haruko-html-jpeg.epub"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	//readium-sdk init
	ReadiumSdk *sdk = new ReadiumSdk();

    QCoreApplication::setOrganizationName("asus");
    QCoreApplication::setApplicationName("eeereader");

    QWebView *view = new QWebView();

    DRWebPage *page = new DRWebPage(sdk, 0);

    DRNetworkAccessManager *netManager = new DRNetworkAccessManager(sdk, 0);
    page->setNetworkAccessManager(netManager);

    qDebug() << "storageLocation:" << QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    qDebug() << "displayName:" << QDesktopServices::displayName(QDesktopServices::DataLocation);

    view->settings()->enablePersistentStorage();

//    QWebInspector *inspector = new QWebInspector;
//    inspector->setPage(page);
//    inspector->show();

    (page->mainFrame())->setUrl(QUrl(READER_SKELETON));
    view->setPage(page);
    view->show();

    DREpubInterface *pEpubInterface = new DREpubInterface(sdk, page->mainFrame());

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    app.exec();

	delete pEpubInterface;
	delete page;
    delete view;
    //delete inspector;
    delete sdk;

//	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

	return 0;
}
