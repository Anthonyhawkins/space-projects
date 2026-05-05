
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <iostream>
#include <optional>

constexpr const char* TIME_FORMAT = "%Y-%m-%d %H:%M:%S UTC";

std::optional<std::tm> parseTime(const std::string& input){

    std::tm time {};
    std::istringstream tsStream(input);

    tsStream >> std::get_time(&time, TIME_FORMAT);
    if (tsStream.fail()){
        return std::nullopt;
    }

    return time; 
}

std::string parseValueFromLabel(const std::string& line, const std::string& label, const std::string& upToLabel){

    std::size_t startPos = line.find(label);
    std::size_t stopPos = line.size(); // default if empty 

    if (!upToLabel.empty()){
        stopPos = line.find(upToLabel, startPos + label.size()); // find ending label AFTER the starting label
    }    

    if (startPos == std::string::npos ||
        stopPos == std::string::npos 
        ){
        //bad start stop step line
        return "";
    }

    // Parse start
    std::string timeStamp = line.substr(
        startPos + std::string(label).size(), //find where the label we care about (label) begins, move forward by len of label 
        stopPos - (startPos + std::string(label).size()) //find the label we are going UP to, subtract the len of the label we care about.
    );

    return timeStamp;

}

void parseHeader(std::vector<std::string> lines){
    const std::string CREATED_LABEL = "created:";
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

    /**
     * Parse Created Time - i.e. created:2026-04-25 20:06:49 UTC
     */

    std::string createdTs = parseValueFromLabel(createdLine, CREATED_LABEL, "");
    if (createdTs.empty()) { 
        // bad parse of source line
        return;
    }

    std::optional<std::tm> createdAtTime = parseTime(createdTs);
    if (!createdAtTime){
        //bad created at time
        return;
    }

    /**
     * Parse start, stop, and step size - i.e. ephemeris_start:2026-04-25 19:38:42 UTC ephemeris_stop:2026-04-27 19:37:42 UTC step_size:60
     */

    // Parse start
    std::string startTs = parseValueFromLabel(startEndStepLine, START_LABEL, STOP_LABEL);
    std::optional<std::tm> startTime = parseTime(startTs);
    if (!startTime){
        // bad startTime 
        return;
    }

    //Parse stop
    std::string stopTs = parseValueFromLabel(startEndStepLine, STOP_LABEL, STEP_LABEL);
    std::optional<std::tm> stopTime = parseTime(stopTs);
    if (!stopTime){
        // bad stopTime
        return;
    }

    // Parse step
    std::string stepSize = parseValueFromLabel(startEndStepLine, STEP_LABEL, "");
    if (stepSize.empty()) {
        // bade step size
        return;
    }

    /**
     * Parse the Source - i.e. ephemeris_source:blend
     */
    std::string source = parseValueFromLabel(sourceLine, SOURCE_LABEL, "");
    if (source.empty()) {
        // bad parse of source line
        return;
    }

    // year stores year - 1900, months are 0 indexed - of course they would be in C++ land!
    std::cout << "Created at: " << std::put_time(&createdAtTime.value(), TIME_FORMAT) << "\n";
    std::cout << "start Time: " << std::put_time(&startTime.value(), TIME_FORMAT) << "\n";
    std::cout << "Stop Time: " << std::put_time(&stopTime.value(), TIME_FORMAT) << "\n";
    std::cout << "Step Size: " << stepSize << "\n";
    std::cout << "Source: " << source << "\n";
    std::cout << "Covariance: " << covarianceLine << "\n";

}


std::vector<std::string> split(const std::string& input, char delim){
    std::vector<std::string> parts;
    std::size_t start = 0;

    for (std::size_t i = 0; i < input.size(); ++i){
        if (input[i] == delim) {
            parts.push_back(input.substr(start, i - start));
            start = i + 1;
        }
    }
    
    parts.push_back(input.substr(start));
    return parts;
};

void parseEntry(std::vector<std::string> lines){

    /**
     * record
     * 
     * timestamp: 2026115100342.000 | YYYY DDD HHMMSS.sss
     * position: -1180.1537434667 6646.6792368759 -499.8923176295 | x y z | km
     * velocity: -4.6181676683 -0.3658921704 6.1230849038 | Vx Vy Vz | km/s
     * 
     * covariance-matrix-input 
     * NOTE: this needs to be unpacked into a full 6x6 matrix, only the upper right of the matrix is provided
     *          
     * 4.9529272549e-07 -3.8070394357e-07 7.5785149365e-07 -2.0956695395e-10 1.9229091390e-11 1.1718187001e-06 9.2611637446e-10
     * -9.1585099974e-10 -3.3495200581e-13 7.3311529102e-12 -4.6546576329e-10 4.0028940231e-10 3.2760977512e-13 -8.9320818832e-13
     * 6.0843867840e-13 -7.5580316602e-13 2.0288322327e-13 1.5750218736e-09 -4.6894423182e-16 8.6004590944e-16 5.5708243178e-12
     * 
     * 
     * uncertainty in [x y z vx vy vz] | [U V W Udot Vdot Wdot] | satellite is probably here ± uncertainty
     */



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