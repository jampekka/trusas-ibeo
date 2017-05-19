.PHONY : all

all: idc_recorder objects_to_json

idc_recorder: idc_recorder.cpp
	g++ --std=c++14 $^ -I sdk/src -pthread -L sdk/libs -static -lboost_system -Wl,--whole-archive -libeosdk -Wl,--no-whole-archive -lboost_thread -lboost_program_options -o $@
	strip $@

objects_to_json: objects_to_json.cpp
	g++ --std=c++14 $^ -I sdk/src -pthread -L sdk/libs -static -lboost_system -Wl,--whole-archive -libeosdk -Wl,--no-whole-archive -lboost_thread -lboost_program_options -o $@
	strip $@
