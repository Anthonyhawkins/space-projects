
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <iostream>

void parseHeader(std::vector<std::string> lines){

    constexpr const char* TIME_FORMAT = "%Y-%m-%d %H:%M:%S UTC";
    const std::string START_LABEL = "ephemeris_start:";
    const std::string STOP_LABEL = "ephemeris_stop:";
    const std::string STEP_LABEL = "step_size:";
    const std::string SOURCE_LABEL = "ephemeris_source:";

    if (lines.size() < 4) {
        //bad header, expecting 4 lines
        return;
    }

    std::string createdLine = lines[0]; // created:2026-04-25 20:06:49 UTC
    std::string startEndStepLine = lines[1]; // ephemeris_start:2026-04-25 19:38:42 UTC ephemeris_stop:2026-04-27 19:37:42 UTC step_size:60
    std::string sourceLine = lines[2]; // ephemeris_source:blend
    std::string covarianceLine = lines[3]; //UVW

    // parse out the created time
    std::string timeStamp = createdLine.substr(createdLine.find(':') + 1);
    if (timeStamp.empty()){
        //bad timestamp, expecting created:2026-04-25 20:06:49 UTC
        return;
    }

    std::tm createdAtTime {};
    std::istringstream tsStream(timeStamp);
    tsStream >> std::get_time(&createdAtTime, TIME_FORMAT);

    // parse out the start, stop times, and the step value
    std::size_t startPos = startEndStepLine.find(START_LABEL);
    std::size_t stopPos = startEndStepLine.find(STOP_LABEL);
    std::size_t stepPos = startEndStepLine.find(STEP_LABEL);

    //check for find failures
    if (startPos == std::string::npos ||
        stopPos == std::string::npos ||
        stepPos == std::string::npos
    ){
        //bad start stop step line
        return;
    }

    std::string startTs = startEndStepLine.substr(
        startPos + std::string(START_LABEL).size(), //find where ephemeris_start: begins, move forward by len of label 
        stopPos - (startPos + std::string(START_LABEL).size()) // find where ephemeris_stop begins, subtract len of start label
    );

    std::tm startTime {};
    std::istringstream startStream(startTs);
    startStream >> std::get_time(&startTime, TIME_FORMAT);

    std::string stopTs = startEndStepLine.substr(
        stopPos + std::string(STOP_LABEL).size(), 
        stepPos - (stopPos + std::string(STOP_LABEL).size()) 
    );

    std::tm stopTime {};
    std::istringstream stopStream(stopTs);
    stopStream >> std::get_time(&stopTime, TIME_FORMAT);

    std::string stepStr = startEndStepLine.substr(
        stepPos + std::string(STEP_LABEL).size()
    );


    std::size_t sourcePos = sourceLine.find(SOURCE_LABEL);
    if (sourcePos == std::string::npos) {
        // bad parse of source line
        return;
    }

    std::string source = sourceLine.substr(sourcePos + std::string(SOURCE_LABEL).size());

    // year stores year - 1900, months are 0 indexed - of course they would be in C++ land!
    std::cout << "Created at: " << std::put_time(&createdAtTime, TIME_FORMAT) << "\n";
    std::cout << "start Time: " << std::put_time(&startTime, TIME_FORMAT) << "\n";
    std::cout << "Stop Time: " << std::put_time(&stopTime, TIME_FORMAT) << "\n";
    std::cout << "Step Size: " << stepStr << "\n";
    std::cout << "Source: " << source << "\n";
    std::cout << "Covariance: " << covarianceLine << "\n";

}


void parseEntry(std::vector<std::string> lines){

}

int main(){

    std::vector<std::string> headerLines = {
        "created:2026-04-25 20:06:49 UTC",
        "ephemeris_start:2026-04-25 19:38:42 UTC ephemeris_stop:2026-04-27 19:37:42 UTC step_size:60",
        "ephemeris_source:blend",
        "UVW"
    };

    parseHeader(headerLines);

    std::vector<std::string> entryLines = {
        "2026115193842.000 6635.5873794006 -1314.9233475698 767.6623381257 0.2150855768 4.6689530736 6.0628927473",
        "5.5855971992e-07 -3.8801739923e-07 8.4142959103e-07 -2.6670037434e-10 -6.9141528227e-11 1.1396391417e-06 9.0536323529e-10",
        "-9.7342871640e-10 -2.5991796971e-14 2.0853462498e-12 -4.9927489044e-10 3.8179168198e-10 -7.2699903612e-14 -8.2567562639e-13",
        "5.2402563002e-13 -1.4420018928e-12 5.8054690161e-13 1.4972764222e-09 -9.2703561585e-16 7.4021517557e-16 6.1054060826e-12"
    };

    parseEntry(entryLines);

}