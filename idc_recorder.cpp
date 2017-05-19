#include <ibeosdk/trackingbox.hpp>
#include <ibeosdk/ecu.hpp>
#include <ibeosdk/lux.hpp>
#include <ibeosdk/devices/IdcFile.hpp>
#include <iostream>
#include <stdlib.h>

const ibeosdk::Version::MajorVersion majorVersion(5);
const ibeosdk::Version::MinorVersion minorVersion(0);
const ibeosdk::Version::Revision revision(4);
const ibeosdk::Version::PatchLevel patchLevel;
const ibeosdk::Version::Build build;
const std::string info = "idc_recorder";

ibeosdk::Version appVersion(majorVersion, minorVersion, revision, patchLevel, build, info);
ibeosdk::IbeoSDK ibeoSDK;

struct FrameDumper :
		public ibeosdk::DataStreamer
		 {
	//ibeosdk::IdcFile writer;
	std::ostream &output;
	bool firstSample = true;
	
	FrameDumper(std::ostream& output=std::cout) :output(output) {
	}

	virtual void onData(const ibeosdk::IbeoDataHeader& dh, const char* const data) {
		if(firstSample) {
			ibeosdk::IbeoDataHeader ndh(dh);
			ndh.setPreviousMessageSize(0);
			ndh.serialize(output);
			firstSample = false;
		} else {
			dh.serialize(output);
		}
		
		output.write(data, dh.getMessageSize());
		output.flush();
	}

	/*virtual void onData(const ibeosdk::LogMessageError* const logMsg) {
		std::cerr << "Error! " << logMsg->getMessage() << std::endl;
	}*/
};

int main(const int argc, char **argv) {
	const ibeosdk::LogLevel ll = ibeosdk::logLevelFromString("Warning");
	ibeosdk::LogFile::setLogLevel(ll);
	ibeosdk::LogFileManager logFileManager;
	logFileManager.start();

	FrameDumper dumper;

	ibeosdk::IbeoTrackingBox device(argv[1]);
	device.setLogFileManager(&logFileManager);
	device.registerStreamer(&dumper);
	
	// We seem to need a sleep before sending commands :(
	ibeosdk::IbeoDevice<ibeosdk::IbeoTrackingBox>* baseDevice = &device;
	baseDevice->getConnected();
	sleep(1);

	device.getConnected();
	while(device.isConnected()) {
		sleep(1);
	}
}
