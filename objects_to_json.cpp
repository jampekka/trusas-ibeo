#include <ibeosdk/ecu.hpp>
#include <ibeosdk/devices/IdcFile.hpp>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include <chrono>

const ibeosdk::Version::MajorVersion majorVersion(5);
const ibeosdk::Version::MinorVersion minorVersion(0);
const ibeosdk::Version::Revision revision(4);
const ibeosdk::Version::PatchLevel patchLevel;
const ibeosdk::Version::Build build;
const std::string info = "objects_to_json";

ibeosdk::Version appVersion(majorVersion, minorVersion, revision, patchLevel, build, info);
ibeosdk::IbeoSDK ibeoSDK;

using nlohmann::json;

namespace ibeosdk {
	void to_json(json &j, const ObjectListEcu& obj) {
		j = json{
			{"ntptime", obj.getTimestamp().getTime()},
			{"objects", obj.getObjects()}
		};
	}

	void to_json(json &j, const ObjectEcu& obj) {
		j = json{
			{"id", obj.getObjectId()},
			{"contour", obj.getContourPoints()}
		};
	}

	void to_json(json &j, const Point2dFloat& p) {
		j = json{
			{"x", p.getX()},
			{"y", p.getY()}
		};
	}
}

class Listener : public ibeosdk::DataListener<ibeosdk::ObjectListEcu> {
	void onData(const ibeosdk::ObjectListEcu* objs) {
		for(auto obj : objs->getObjects()) {
			std::cout << obj.getObjectId() << std::endl;
		}
	}
};

static unsigned long getTimeSinceEpochMicros() {
    return static_cast<unsigned long>
        (std::chrono::duration_cast<std::chrono::microseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count());
}

void dump_objects(const ibeosdk::IbeoDataHeader hdr,
		const ibeosdk::ObjectListEcu& objects, std::ostream& output=std::cout) {
	// TODO: USE THE HEADER TIMESTAMP!!
	json j = json::array({
		{{"ts", getTimeSinceEpochMicros()/1e6}},
		objects
		});
	output << j << std::endl;
	output.flush();
}

int main(int argc, char** argv) {
	const ibeosdk::LogLevel ll = ibeosdk::logLevelFromString("Warning");
	ibeosdk::LogFile::setLogLevel(ll);
	ibeosdk::LogFileManager logFileManager;
	logFileManager.start();
	
	std::ifstream input;
	input.open("/dev/stdin", std::ifstream::in | std::ifstream::binary);
	
	ibeosdk::IbeoDataHeader hdr;
	ibeosdk::ObjectListEcu objList;
	while(!input.eof()) {
		hdr.deserialize(input);
		if(hdr.getDataType() == objList.getDataType()) {
			objList.deserialize(input, hdr);
			dump_objects(hdr, objList);
		} else {
			input.ignore(hdr.getMessageSize());
		}
	}
	
	return 0;
}
