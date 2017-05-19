#include <ibeosdk/trackingbox.hpp>
#include <ibeosdk/ecu.hpp>
#include <ibeosdk/lux.hpp>
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
	std::ostream &output;
	bool firstSample = true;
	
	FrameDumper(std::ostream& output=std::cout) :output(output) {
	}

	virtual void onData(const ibeosdk::IbeoDataHeader& dh, const char* const data) {
		ibeosdk::IbeoDataHeader oh(dh);
		if(firstSample) {
			oh.setPreviousMessageSize(0);
			firstSample = false;
		}
		// Shouldn't probably do this, but rewrite the
		// header timestamp so we don't have to sync the
		// clocks.
		oh.setTimestamp(ibeosdk::Time::universalTime());
		oh.serialize(output);
		output.write(data, oh.getMessageSize());
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
