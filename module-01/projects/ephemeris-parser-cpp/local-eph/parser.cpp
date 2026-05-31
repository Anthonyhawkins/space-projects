
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <iostream>
#include <optional>
#include <tuple>
#include <charconv>

constexpr const char* TIME_FORMAT = "%Y-%m-%d %H:%M:%S UTC";


std::optional<float> parseFloat(const std::string& s) {
    float value{};
    
    // parse from start to last char + 1
    std::from_chars_result res = std::from_chars(s.data(), s.data() + s.size(), value);

    if (res.ec != std::errc() || res.ptr != s.data() + s.size()) {
        return std::nullopt; // invalid or trailing garbage
    }

    return value;
}

std::optional<float> parseDouble(const std::string& s) {
    double value{};

    // parse from start to last char + 1
    std::from_chars_result res = std::from_chars(s.data(), s.data() + s.size(), value);

    if (res.ec != std::errc() || res.ptr != s.data() + s.size()) {
        return std::nullopt;
    }

    return value;

}

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


struct Metadata {
    std::string created;
    std::string start;
    std::string stop;
    std::string step;
    std::string source;
};

struct ParseHeaderResult {
    std::string error;
    Metadata metadata;
};

//TODO return Parse Result
void parseHeader(std::vector<std::string> lines){

    ParseHeaderResult result;

    const std::string CREATED_LABEL = "created:";
    const std::string START_LABEL = "ephemeris_start:";
    const std::string STOP_LABEL = "ephemeris_stop:";
    const std::string STEP_LABEL = "step_size:";
    const std::string SOURCE_LABEL = "ephemeris_source:";

    if (lines.size() < 4) {
        result.error = "Unable to parse header. Expected 4 lines, Got " + std::to_string(lines.size());
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
        result.error = "Unable to parse timestamp from label: '" + CREATED_LABEL + "' on line: " + createdLine;
        return;
    }

    std::optional<std::tm> createdAtTime = parseTime(createdTs);
    if (!createdAtTime){
        result.error = "Unable to create time from label on line: " + createdLine;
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

struct PositionVec {
    float x;
    float y;
    float z;
};

struct VelocityVec {
    float vx;
    float vy;
    float vz;
};


struct TimeStep {
    std::optional<std::tm> time;
    PositionVec position;
    VelocityVec velocity;
    std::vector<std::vector<double>> covariance;
};


struct ParseEntryResult {
    std::string error;
    TimeStep timestep;
};

//TODO return Parse Result
void parseEntry(std::vector<std::string> lines){

    ParseEntryResult result; 

    if (lines.size() < 4) {
        result.error = "Failed to parse Entry, expected exactly 4 lines. Got " + std::to_string(lines.size()) + " lines.";
        return;
    }

    /** record
     * 
     * timestamp: 2026115100342.000 | YYYY DDD HHMMSS.sss
     * position: -1180.1537434667 6646.6792368759 -499.8923176295 | x y z | km
     * velocity: -4.6181676683 -0.3658921704 6.1230849038 | Vx Vy Vz | km/s
     * 
     */

    std::string positionAndVelocity = lines[0];
    std::vector<std::string> positionAndVelocityParts = split(positionAndVelocity, ' ');

    if (positionAndVelocityParts.size() != 7){
        result.error = "Failed to parse Entry, Expected 1 timestamp, 3 positions, and 3 velocity values.  Got " + std::to_string(positionAndVelocityParts.size()) + " values.";
        return;
    }

    std::string timeStamp = positionAndVelocityParts[0];

    std::string xStr = positionAndVelocityParts[1];
    std::string yStr = positionAndVelocityParts[2];
    std::string zStr = positionAndVelocityParts[3];
    std::string vxStr = positionAndVelocityParts[4];
    std::string vyStr = positionAndVelocityParts[5];
    std::string vzStr = positionAndVelocityParts[6];
    
    PositionVec xyz;
    auto x = parseFloat(xStr);
    if (!x){
        result.error = "Failed to parse X for Position Vector. Expected parsable float.  Got " + xStr + " at x _ _";
        return;
    }

    auto y = parseFloat(yStr);
    if (!y){
        result.error = "Failed to parse Y for Position Vector. Expected parsable float.  Got " + yStr + " at _ y _";
        return;
    }

    auto z = parseFloat(zStr);
    if (!z){
        result.error = "Failed to parse Y for Position Vector. Expected parsable float.  Got " + zStr + " at _ _ z";
        return;
    }

    VelocityVec vxyz;
    auto vx = parseFloat(vxStr);
    if (!vx){
        result.error = "Failed to parse X for Velocity Vector. Expected parsable float.  Got " + vxStr + " at vx _ _";
        return;
    }

    auto vy = parseFloat(vyStr);
    if (!vy){
        result.error = "Failed to parse Y for Velocity Vector. Expected parsable float.  Got " + vyStr + " at _ vy _";
        return;
    }

    auto vz = parseFloat(vzStr);
    if (!vz){
        result.error = "Failed to parse Y for Velocity Vector. Expected parsable float.  Got " + vzStr + " at _ _ vz";
        return;
    }


    /**

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

    std::vector<std::string> seed = split(lines[1] + " " + lines[2] + " " + lines[3], ' ');
    std::vector<std::vector<std::string>> covarianceMatrix(6, std::vector<std::string>(6));

    if (seed.size() != 21){
        result.error = "Unable to construct Covariance matrix, expected 21 seed values. Got " + std::to_string(seed.size());
        return;
        // not enough values to unpack
    }

    double value = 0;
    std::size_t k = 0;
    for (std::size_t i = 0; i < 6; ++i){
        for (std::size_t j = i; j < 6; ++j){
            
            auto val = parseDouble(seed[k]);
            if (!val){
                result.error = "Failed to parse seed value while constructing Covariance matrix, expected parsable double.  Got " + seed[k] + ".";
                return;
            }

            covarianceMatrix[i][j] = value;
            covarianceMatrix[j][i] = value;
            ++k;
        }
    }

    std::cout << "\nCovariance Matrix (6x6)\n\n";

    for (std::size_t i = 0; i < 6; ++i){
        for (std::size_t j = 0; j < 6; ++j){
            std::cout << std::setw(3) << covarianceMatrix[i][j]  << " ";
        }
        std::cout << "\n";
    }

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