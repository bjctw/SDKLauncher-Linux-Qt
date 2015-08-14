//============================================================================
// Name        : sdkHelpler.cpp
// Author      : akira.1642@gmail.com
//This is free and unencumbered software released into the public domain.
//The detailed description is in UNLICENSE.
//============================================================================

#include "sdkHelper.h"

#include <ePub3/initialization.h>
#include <ePub3/archive.h>
#include <ePub3/container.h>
#include <ePub3/package.h>
#include <ePub3/property.h>
#include <ePub3/media-overlays_smil_model.h>	// ePub3::MediaOverlaysSmilModel
#include <ePub3/media-overlays_smil_data.h>		// ePub3::SMILData
#include <ePub3/utilities/byte_stream.h>

#include "json/json.h"
#include <string>
#include <vector>

#define  LOGD(...)  ((void)0)
#define  LOGE printf

#define	REDIRECT_PATH	"epubVirtual"

using namespace std;

ReadiumSdk::ReadiumSdk()
{
    ePub3::InitializeSdk();
    ePub3::PopulateFilterManager();
}

ReadiumSdk::~ReadiumSdk()
{
}

ePub3::Package* ReadiumSdk::getPackagePtr()
{
	return (&*m_pckg);
}

void ReadiumSdk::openEpubFile(char *file)
{
	//    ePub3::Container con;
	//	bool res = con.Open("/home/ben/workspace/SDKLauncher-Linux-Qt/res/test.epub");
	//	cout << "open book " << res << endl;

	m_cntnr = ePub3::Container::OpenContainer(file);
	m_pckg = m_cntnr->DefaultPackage();

    //dump package content
//    Json::Value root = packageToJason(&(*pkg));
//    Json::StyledWriter writer;
//    packStr = writer.write( root );
//    cout << packStr.c_str();
    //QString jsfunc = QString("openBook(%1)").arg(packStr.c_str());
}

static ePub3::string getProperty(ePub3::Package* package, char* name, char* pref, ePub3::PropertyHolder* forObject, bool lookupParents)
{
    LOGD("getProperty(): called for name='%s' pref='%s'", name, pref);
    auto propertyName = ePub3::string(name);
    auto prefix = ePub3::string(pref);
    auto iri = package->MakePropertyIRI(propertyName, prefix);

    auto prop = forObject->PropertyMatching(iri, lookupParents);

    if (prop != nullptr) {
        ePub3::string value(prop->Value());
        LOGD("getProperty(): returning '%s'", value.c_str());
        return value;
    }
    LOGD("getProperty(): returning EMPTY");
    return "";
}

static ePub3::string getPropertyWithOptionalPrefix(ePub3::Package *package, char *name, char *pref, ePub3::PropertyHolder *forObject, bool lookupParents)
{
    LOGD("getPropertyWithOptionalPrefix(): called for name='%s' pref='%s'", name, pref);
    auto value = getProperty(package, name, pref, forObject, lookupParents);

    if (value.length() == 0) {
        LOGD("getPropertyWithOptionalPrefix(): did not find with prefix, attempting with no prefix (name='%s' pref='%s')", name, pref);
        return getProperty(package, name, (char *) "", forObject, lookupParents);
    }

    return value;
}


static void populateJsonWithSmilParAudio(stringstream &stream, std::shared_ptr<const ePub3::SMILData::Audio> audio){

	stream << "{" << endl;
	stream << "\"nodeType\" : \"audio\"," << endl;
	stream << "\"src\" : \"" << audio->SrcFile() << "\"," << endl;

	double begin = (static_cast<double>(audio->ClipBeginMilliseconds())) / 1000;
	double end = (static_cast<double>(audio->ClipEndMilliseconds())) / 1000;

	stream << "\"clipBegin\" : " << begin << "," << endl;
	stream << "\"clipEnd\" : " << end << "" << endl;
	stream << "}" << endl;
}


static void populateJsonWithSmilParText(stringstream &stream, std::shared_ptr<const ePub3::SMILData::Text> text){

    auto srcFragmentId = text->SrcFragmentId();
    auto srcFile = text->SrcFile();

    stream << "{" << endl;
    stream << "\"nodeType\" : \"text\"," << endl;
    stream << "\"srcFile\" : \"" << srcFile << "\"," << endl;
    stream << "\"srcFragmentId\" : \"" << srcFragmentId << "\"," << endl;

    if(srcFragmentId.empty()){
    	stream << "\"src\" : \"" << srcFile << "\"" << endl;
    } else {
		stream << "\"src\" : \"" << srcFile << '#' << srcFragmentId << "\"" << endl;
    }

    stream << "}" << endl;
}

static void populateJsonWithSmilPar(stringstream &stream, std::shared_ptr<const ePub3::SMILData::Parallel> par){
	//TODO do we need this?
    //printf("CHECK SMIL DATA TREE TEXTREF FRAGID %s\n", par->_textref_fragmentID.c_str());

	if(nullptr == par){
		return;
	}

    stream << "{" << endl;
	stream << "\"epubtype\": \"" << par->Type() << "\" ," << endl;
	stream << "\"nodeType\": \"" << par->Name() << "\" ," << endl;

	stream << "\"children\" : [ " <<endl;
    if (nullptr != par->Text()){
        populateJsonWithSmilParText(stream, par->Text());
    }

    if (nullptr != par->Audio()){
    	stream << " , " << endl;
        populateJsonWithSmilParAudio(stream, par->Audio());
    }
    stream << "]" << endl;
	stream << "}" << endl;

}

static void populateJsonWithSmilSeq(stringstream &stream, std::shared_ptr<const ePub3::SMILData::Sequence> seqq){
	//TODO do we need this?
    //printf("CHECK SMIL DATA TREE TEXTREF FRAGID %s\n", seqq->_textref_fragmentID.c_str());

	if(nullptr == seqq){
        stream << "!! NULL SMIL SEQQ" << endl;
		return;
	}

    stream << "{" << endl;
    stream << "\"textref\": \"" << seqq->TextRefFile() << "\" ," << endl;
	stream << "\"epubtype\": \"" << seqq->Type() << "\" ," << endl;
	stream << "\"nodeType\": \"" << seqq->Name() << "\" ," << endl;

	stream << "\"children\": [" << endl;

	auto childrenCount = seqq->GetChildrenCount();

    for (int i = 0; i < childrenCount; ++i){

        std::shared_ptr<const ePub3::SMILData::TimeContainer> container = seqq->GetChild(i);

        if(nullptr == container){
            stream << "!! NULL SMIL CONTAINER" << endl;
        	continue;
        }


        //const ePub3::SMILData::Sequence *seq = dynamic_cast<const ePub3::SMILData::Sequence *>(container);
        //if (nullptr != seq){
        if (container->IsSequence()){
            auto seq = std::dynamic_pointer_cast<const ePub3::SMILData::Sequence>(container);
        	populateJsonWithSmilSeq(stream, seq);
            //continue;
        }

        //const ePub3::SMILData::Parallel *par = dynamic_cast<const ePub3::SMILData::Parallel *>(container);
        //if (nullptr != par){
        else if (container->IsParallel()){
            auto par = std::dynamic_pointer_cast<const ePub3::SMILData::Parallel>(container);
        	populateJsonWithSmilPar(stream, par);
            //continue;
        }
        else{
            stream << "!! SMIL CONTAINER TYPE???" << endl;
        }

        if(i != childrenCount-1){
        	stream << ',';
        }

    }
    stream << ']' << endl;
    stream << '}' << endl;

}

static void populateJsonWithSmilDatas(stringstream &stream, std::shared_ptr<ePub3::MediaOverlaysSmilModel> &smilDatas){

    if(nullptr == smilDatas){
    	return;
    }

    stream << "\"smil_models\" : [" << endl;

    int smilDataAmount = smilDatas->GetSmilCount();

    for (int i = 0; i < smilDataAmount; ++i){
        shared_ptr<ePub3::SMILData> smilData = smilDatas->GetSmil(i);

        if(nullptr == smilData){
        	continue;
        }

        stream << "{" << endl;

		//issue smilVersion
        stream << "\"smilVersion\" : \"3.0\"," << endl;

        //issue duration
        double duration = static_cast<double>(smilData->DurationMilliseconds_Metadata())/1000;
        stream << "\"duration\" : " << duration  << "," << endl;


        auto item = smilData->SmilManifestItem();

        string id;
        string href;
        if(nullptr == item){
        	id = "";
        	href = "fake.smil";
        } else{
        	id = item->Identifier().c_str();
        	href = item->Href().c_str();
        }

        stream << "\"id\" : \""<< id << "\"," << endl;
        stream << "\"href\" : \""<< href << "\"," << endl;

        stream << "\"spineItemId\" : \"" << smilData->XhtmlSpineItem()->Idref().c_str() << "\"," << endl;

        stream << "\"children\":[" << endl;

        auto seq = smilData->Body();
        //ePub3::SMILData::Sequence *seq = const_cast<ePub3::SMILData::Sequence *>(smilData->Body());
        populateJsonWithSmilSeq(stream, seq);

        stream << "]" << endl;

		stream << "}" << endl;

		//if we are not at the last index, set a comma
		if(i != smilDataAmount-1){
			stream << ",";
		}
    }

    stream << ']'; // smil_models [
}

static void populateJsonWithEscapeables(stringstream &stream, std::shared_ptr<ePub3::MediaOverlaysSmilModel> &smilDatas){
	stream << "\"escapables\" : [" << endl;

	auto amountOfEscapeables = smilDatas->GetEscapablesCount();

	for(std::vector<string>::size_type i = 0; i < amountOfEscapeables; ++i){
		stream << "\"" << smilDatas->GetEscapable(i) << "\"";
		if(i != amountOfEscapeables-1){
			stream << "," << endl;
		} else{
			stream << endl;
		}
	}

	stream << "]," << endl; //escapeables: [, the comma is there because we know that after this there will be skippables
}

static void populateJsonWithSkippables(stringstream &stream, std::shared_ptr<ePub3::MediaOverlaysSmilModel> &smilDatas){
	stream << "\"skippables\" : [" << endl;

	auto amountOfSkippables = smilDatas->GetSkippablesCount();

	for(std::vector<string>::size_type i = 0; i < amountOfSkippables; ++i){
		stream << "\"" << smilDatas->GetSkippable(i) << "\"";
		if(i != amountOfSkippables-1){
			stream << "," << endl;
		} else{
			stream << endl;
		}
	}

	stream << "]," << endl; //skippables: [, the comma is there because we know that after this there will be smil_models
}

static void populateJsonWithMediaOverlayContent(std::stringstream &stream, std::shared_ptr<ePub3::MediaOverlaysSmilModel> &model){
	//sanity check
	if(nullptr == model){
    	return;
    }

    double duration = static_cast<double>(model->DurationMilliseconds_Metadata())/1000;

    stream << "\"activeClass\": \"" << model->ActiveClass() << "\"," << endl;
    stream << "\"duration\": " << duration << "," << endl;
    stream << "\"narrator\": \"" << model->Narrator() << "\"," << endl;
    stream << "\"playbackActiveClass\": \"" << model->PlaybackActiveClass() << "\"," << endl;

    populateJsonWithEscapeables(stream, model);
    populateJsonWithSkippables(stream, model);
	populateJsonWithSmilDatas(stream, model);
}


Json::Value ReadiumSdk::packageToJason()
{
	ePub3::Package* package = &*m_pckg;

	Json::Value root;

	root["package"]["rootUrl"] = REDIRECT_PATH;
	root["package"]["rendition_layout"] = getProperty(package, "layout" ,"rendition" , package , true).c_str();
	root["package"]["rendition_flow"] = getProperty(package, "flow", "rendition", package , true).c_str();
	root["package"]["rendition_orientation"] = getProperty(package, "orientation", "rendition", package , true).c_str();
	root["package"]["rendition_spread"] = getProperty(package, "spread", "rendition", package , true).c_str();

    Json::Value spineArray(Json::arrayValue);
    Json::Value spineItem;
    auto spine = package->FirstSpineItem();
    do {
	    spineItem["href"] = spine->ManifestItem()->BaseHref().c_str();
	    spineItem["media_type"] = spine->ManifestItem()->MediaType().c_str();	//not used in spine_item.js
	    spineItem["page_spread"] = getPropertyWithOptionalPrefix(package, (char *) "page-spread", (char *) "rendition", (&*spine), false).c_str();
	    spineItem["idref"] = spine->Idref().c_str();
	    spineItem["rendition_layout"] = getProperty(package, (char *) "layout", (char *) "rendition", (&*spine), false).c_str();
	    spineItem["rendition_flow"] = getProperty(package, (char *) "flow", (char *) "rendition", (&*spine), false).c_str();	//not used in spine_item.js
	    spineItem["rendition_orientation"] = getProperty(package, (char *) "orientation", (char *) "rendition", (&*spine), false).c_str();	//not used in spine_item.js
	    spineItem["rendition_spread"] = getProperty(package, (char *) "spread", (char *) "rendition", (&*spine), false).c_str();	//not used in spine_item.js
	    spineItem["linear"] = spine->Linear() ? "yes" : "no";	//not used in spine_item.js
	    spineItem["media_overlay_id"] = spine->ManifestItem()->MediaOverlayID().c_str();
	    spineItem["title"] = spine->Title().c_str();	//not used in spine_item.js
	    spineArray.append(spineItem);
    } while ((spine = spine->Next()) != nullptr);

	Json::Value	spineJson;
	spineJson["items"] = spineArray;
	ePub3::PageProgression pageProgressionDirection = package->PageProgressionDirection();
	char *direction;
	switch (pageProgressionDirection) {
	case ePub3::PageProgression::LeftToRight:
		direction = (char *) "ltr";
		break;
	case ePub3::PageProgression::RightToLeft:
		direction = (char *) "rtl";
		break;
	case ePub3::PageProgression::Default:
	default:
		direction = (char *) "";
	}
	spineJson["direction"] = direction;

    root["package"]["spine"] = spineJson;

	std::stringstream stream;

	auto model = package->MediaOverlaysSmilModel();

// TODO: solve error:
//	msg:  "TypeError: 'undefined' is not an object (evaluating 'moDTO.smil_models.length')"
//	lineNumber:  22
//	sourceID:  "http://127.0.0.1/res/js/models/media_overlay.js"
//	stream << "{" << endl;
//
//	populateJsonWithMediaOverlayContent(stream, model);
//
//	stream << '}'; // media_overlay
//    root["package"]["media_overlay"] = stream.str();

	return root;
}

int ReadiumSdk::getContent(char *relativePath, char **buf)
{
	int nReadSize = 0;

	auto basePath = ePub3::string(m_pckg->BasePath());
	LOGD("Package.nativeInputStreamForRelativePath(): package base path '%s'", basePath.c_str());
    auto path = basePath.append(relativePath);
    LOGD("Package.nativeInputStreamForRelativePath(): final path '%s'", path.c_str());
    auto archive = m_pckg->GetContainer()->GetArchive();
    bool containsPath = archive->ContainsItem(path);
    if (!containsPath) {
        LOGE("Package.nativeInputStreamForRelativePath(): no archive found for path '%s'", path.c_str());
        return 0;
    }

    unique_ptr<ePub3::ByteStream> byteStream;

	ePub3::ConstManifestItemPtr manifestItem = m_pckg->ManifestItemAtRelativePath(ePub3::string(relativePath));
	if (manifestItem != nullptr) {
		auto rawInputbyteStream = m_pckg->ReadStreamForItemAtPath(path);
		ePub3::ManifestItemPtr m = std::const_pointer_cast<ePub3::ManifestItem>(manifestItem);
//		if (isRange == JNI_TRUE) {
//			byteStream = PCKG(pckgPtr)->GetFilterChainByteStreamRange(m, dynamic_cast<ePub3::SeekableByteStream *>(rawInputbyteStream.release()));
//		} else {
			byteStream = m_pckg->GetFilterChainByteStream(m, dynamic_cast<ePub3::SeekableByteStream *>(rawInputbyteStream.release()));
//		}
	} else {
		// In the rare case that the manifest item could not be resolved from the path,
		// fallback to a non-filtered byte stream read.
		byteStream = m_pckg->ReadStreamForItemAtPath(path);
		LOGD("Package.nativeInputStreamForRelativePath(): manifest item not found for relative path '%s'", relativePath);
	}

	nReadSize = byteStream->BytesAvailable();
	*buf = new char[nReadSize];
	byteStream->ReadBytes(*buf, nReadSize);

	printf("%s >> %d bytes read\n", __FUNCTION__, nReadSize);

	return nReadSize;
}
