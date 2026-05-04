#include <filesystem>
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <curl/curl.h>

std::mutex coutMutex;

/**
 * UTILS
 */
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


struct Config {
    std::string loader;
    std::string dir;
    std::string baseUrl;
    int interval;
    int workers;
};


class Satellite{
    public:
        Satellite() : fileName(""), error(""){}
        Satellite(const std::string& fileName) : fileName(fileName), error("") {

            if (fileName.empty()) {
                error = "Filenames must not be empty strings.";
                return;
            }

            std::string nameAndExt;
            std::vector<std::string> pathParts = split(fileName, '/');
            if (pathParts.size() > 0) {
                nameAndExt = pathParts.back();
            }

            std::vector<std::string> nameAndExtParts = split(nameAndExt, '.');
            if (nameAndExtParts.empty()) {
                error = "Unable to parsefilename, doesn't look like a .<ext> file. Missing .";
                return;
            }

            std::vector<std::string> nameParts = split(fileName, '_');
            if (nameParts.size() != 7) {
                error = "Filename was split into more parts than expected.  Looking for 7, found " 
                + std::to_string(nameParts.size());
                return;
            }

            referenceFrame = nameParts[0];
            noradId = nameParts[1];
            starlinkId = nameParts[3];
            internalId = nameParts[4];
            status = nameParts[5];
            internalExportId = nameParts[6];
            classification = nameParts[7];

        }

        std::string fileName;
        //Usually Mean Equator Mean Equinox, an Earth-centered inertial reference frame used for orbital state vectors        
        std::string referenceFrame;
        //Unique tracking ID used by organizations like Space-Track and CelesTrak for this object
        std::string noradId;
        //The spacecraft’s constellation identifier within the Starlink network
        std::string starlinkId;
        //Likely a SpaceX internal identifier for manufacturing, mission planning, or ephemeris generation
        std::string internalId;
        //Indicates the spacecraft is in active service rather than transfer, disposal, drift, or parking
        std::string status;
        //Likely a unique internal version ID for this specific ephemeris file generation
        std::string internalExportId;
        //Indicates the file is approved for public release
        std::string classification;

        //Ephemeris file fields
        std::string createdAt; 

        std::string error;
    
    private:
        void parseFile(){
            std::ifstream file(fileName);
            if (!file.is_open()){
                error = "Unable to open file: " + fileName;
                return;
            }

            std::string line;
            std::vector<std::string> entryLines;

            //collect header lines, first 4 lines then parse.
            std::size_t i = 1;
            while (std::getline(file, line)){
                if (i < 5){
                    entryLines.push_back(line);
                }
                if (i == 4){
                    parseHeaders(entryLines);
                    entryLines.clear();
                    break;
                }
            }

            //collect every set of 4 lines, parse as an entry
            i = 1;
            while(std::getline(file, line)){
                if (i < 5) {
                    entryLines.push_back(line);
                }
                if ( i == 4) {
                    parseEntry(entryLines);
                    entryLines.clear();
                    i = 1;
                    break; // TODO - remove this after algo is working.
                    //continue;
                }
                ++i;
            }

            if (!file.eof()) {
                error = "Read Error while reading file: " + fileName;
                return;
            }
            return;
        }

        /**
         * createdAt created:2026-04-25 12:03:33 UTC
         * 
         * ephemeris_start:2026-04-25 10:03:42 UTC ephemeris_stop:2026-04-28 10:03:42 UTC step_size:60
         * startTime
         * endTime
         * stepTime
         * 
         * source ephemeris_source:blend
         * 
         * covariance UVW
         * 
         * U = radial (away from Earth)
         * V = along-track (direction of motion)
         * W = cross-track (normal to orbital plane)
         * 
         */

        // parse the first 4 lines of an ephemeris 
        void parseHeaders(std::vector<std::string> headerLines){
          return;
        }


        // FULL RECORD EXAMPLE
        //2026115100342.000 -1180.1537434667 6646.6792368759 -499.8923176295 -4.6181676683 -0.3658921704 6.1230849038
        //4.9529272549e-07 -3.8070394357e-07 7.5785149365e-07 -2.0956695395e-10 1.9229091390e-11 1.1718187001e-06 9.2611637446e-10
        //-9.1585099974e-10 -3.3495200581e-13 7.3311529102e-12 -4.6546576329e-10 4.0028940231e-10 3.2760977512e-13 -8.9320818832e-13
        //6.0843867840e-13 -7.5580316602e-13 2.0288322327e-13 1.5750218736e-09 -4.6894423182e-16 8.6004590944e-16 5.5708243178e-12


        /**
         * record
         * 
         * timestamp: 2026115100342.000 | YYYY DDD HHMMSS.sss
         * position: -1180.1537434667 6646.6792368759 -499.8923176295 | x y z | km
         * velocity: -4.6181676683 -0.36589
         * 21704 6.1230849038 | Vx Vy Vz | km/s
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


        // Parse the 4 lines which make up an ephemeris entry
        void parseEntry(std::vector<std::string> entryLines){
            return;
        }

    
};



// Interface Loader
    // func Load() returns ephs[]
class Loader {
    public:
        virtual ~Loader() = default;
        virtual std::vector<Satellite> load() = 0; // important, makes it 'interface-like'
};


// Class WebLoader
class WebLoader : public Loader {
    public:

        WebLoader(Config& config) : config(config){

        };

        std::vector<Satellite> load(){
            //Get Manifest Page, Check For 200
            //Parse Manifest, return []files
            //For Each File
            //GET raw Eph content
            //Create Eph
            //Append
            // Return ephs
            return {};
        }

        Config& config;

        //store created ephs from content
        std::vector<Satellite> satellites;

};
// Class FileLoader
class FileLoader : public Loader {
    public:
        FileLoader(Config& config) : config(config){

        };

        std::vector<Satellite> load() {
        
            // Does Dir Exist?
            std::filesystem::path dir = config.dir;
            if (!(std::filesystem::exists(dir) && std::filesystem::is_directory(dir))){
                // Directory Does Not Exist
                // TODO - create a loadResult struct which has {sats, ok, error}
            }
            // Load filenames
            std::vector<std::filesystem::path> files;
            for (const auto& entry : std::filesystem::directory_iterator(dir)){
                if (entry.is_regular_file()){
                    // we can prob parse the filename here
                    // check if file is .txt
                    files.push_back(entry.path());
                }
            }

            //ceiling division to ensure we round up, not down. no file left behind!
            std::size_t numFilesPerWorker = (files.size() + config.workers - 1) / config.workers;

            std::vector<Satellite> sats(files.size());

            // Assign each worker a begin and end point 
            std::vector<std::thread> threads;
            for (std::size_t w = 0; w < config.workers; ++w){
                std::size_t begin = w * numFilesPerWorker;
                std::size_t end = std::min(begin + numFilesPerWorker, files.size());
                threads.emplace_back(
                    &FileLoader::worker,
                    this,
                    w,
                    begin,
                    end,
                    std::ref(files),
                    std::ref(sats)
                );
            }

            for (auto& t : threads){
                t.join();
            }

            std::cout << "Starlink Nodes: " << std::to_string(sats.size()) << "\n"; 

            for (const auto& sat : sats) {
                std::cout << "========================================\n";
                std::cout << "Starlink: " + sat.starlinkId + "\n";
                std::cout << "Status: " + sat.status + "\n\n";
            }

            return sats;
        }

        Config& config;

    private:
        void worker(
            int worker, 
            std::size_t begin, std::size_t end,
            std::vector<std::filesystem::path>& files, 
            std::vector<Satellite>& sats
        ){
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "Worker: " << worker << "\n\tBegin-End: " 
                          << begin << "-" << end << "\n";
            }
            
            for (std::size_t i = begin; i < end; ++i){

                //TODO - this was a debug try catch, need better error handling for this.
                try {
                    Satellite sat(files[i]);
                    sats[i] = sat;

                } catch (const std::exception& e){
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "Worker " << worker
                              << " failed on file: " << files[i]
                              << "\nerror: "  << e.what() << "\n";
                }
                // Readfile
                //TODO - Each file contains 11k lines, we do not want to read the
                // entire file, then parse. A good starting method is to prob begin reading the file
                // assert it is a valid record, instantiate an eph
                // then as we progress down the file, continue to build out the state of the eph
                // at the end of file, our eph will be loaded.

                // Create Eph
                // Append
            }

            return;
        };
};

struct SetupResult {
    bool ok;
    std::string error;
};

SetupResult setup(Config& config, int argc, char* argv[]){

    SetupResult ret = {
        .ok = true,
        .error = ""
    };

    for (int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        //TODO - This parsing is good enough for now.
        if (arg == "--from" && i + 1 < argc) 
        {
            config.loader = argv[i+1];
        } 
        else if (config.loader != "file" && config.loader != "web") 
        {
            ret.ok = false;
            ret.error = "Loader: " + config.loader + " Not Supporter\n";
            return ret;
        } 
        else if (config.loader == "file" && arg == "--dir" && i + 1 < argc) 
        {
            config.dir = argv[i+1];
        } 
        else if (config.loader == "web" && arg == "--base-url" && i + 1 < argc) 
        {
            config.baseUrl = argv[i+1];
        } 
        else if (config.loader == "web" && arg == "--interval" && i + 1 < argc) 
        {
            try {
                config.interval = std::stoi(argv[i+1]); 
            } catch(std::exception& e){
                ret.ok = false;
                ret.error = "Invalid Argument: --interval must be an integer\n";
                return ret;
            }
        } 
        else if (arg == "--workers" && i + 1 < argc) 
        {
            try {
                config.workers = std::stoi(argv[i+1]); 
            } catch(std::exception& e){
                ret.ok = false;
                ret.error = "Invalid Argument: --workers must be an integer\n";
                return ret;
            }        
        }
    }

    return ret;
}



int main(int argc, char* argv[]){

    Config c = Config{
        .loader = "file",
        .dir = "",
        .baseUrl = "http://localhost",
        .interval = 1,
        .workers = 1
    };

    SetupResult result = setup(c, argc, argv);
    
    if (!result.ok) {
        std::cout << result.error << std::endl;
        return 1;
    }

    std::unique_ptr<Loader> loader;

    if (c.loader == "file"){
        loader = std::make_unique<FileLoader>(c);
    } else {
        loader = std::make_unique<WebLoader>(c);
    }

    std::vector<Satellite> sats = loader->load();

    // ephs = loader.load()

    
    return 0;
}
