//============================================================================
// Name        : sdkHelpler.h
// Author      : akira.1642@gmail.com
//This is free and unencumbered software released into the public domain.
//The detailed description is in UNLICENSE.
//============================================================================

#ifndef SDKHELPLER_H_
#define SDKHELPLER_H_

#include <ePub3/Forward.h>
#include "json/json.h"

namespace ePub3{
	class Package;
}

class ReadiumSdk
{
public:
	ReadiumSdk();
	~ReadiumSdk();

	void openEpubFile(char *file);
	ePub3::Package* getPackagePtr();
	Json::Value packageToJason();
	int getContent(char *relativePath, char **buf);

private:
	ePub3::ContainerPtr m_cntnr;
	ePub3::PackagePtr m_pckg;
};


#endif /* SDKHELPLER_H_ */
