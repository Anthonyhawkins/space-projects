#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <curl/curl.h>


// class Ephemeris 
class Ephemeris {};
    //func constructor(sting file)
    // - from a file, construct an eph <-- DONT DO THIS

    /**
     * 
     * fileName 
     * 
     * //Usually Mean Equator Mean Equinox, an Earth-centered inertial reference frame used for orbital state vectors
     * referenceFrame
     * 
     * //Unique tracking ID used by organizations like Space-Track and CelesTrak for this object
     * noradId
     * 
     * /The spacecraft’s constellation identifier within the Starlink network
     * starlinkId
     *
     * //Likely a SpaceX internal identifier for manufacturing, mission planning, or ephemeris generation
     * internalId
     * 
     * //Indicates the spacecraft is in active service rather than transfer, disposal, drift, or parking
     * status
     * 
     * //Likely a unique internal version ID for this specific ephemeris file generation
     * internalExportId
     * 
     * //Indicates the file is approved for public release
     * classification
     */

     /**
      * //raw unparsed body
      * rawBody
      */


struct Config {
    std::string loader;
    std::string dir;
    std::string baseUrl;
    int interval;
    int workers;
};


// class Satellite

    //func constructor(Ephemeris eph)
    //  - parse eph instantiate sat from eph

    /**
     * norad-id
     * starlink-id 
     * status
     * export-id
     * classification
     */

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



// Interface Loader
    // func Load() returns ephs[]
class Loader {
    public:
        virtual ~Loader() = default;
        virtual std::vector<Ephemeris> load() = 0; // important, makes it 'interface-like'
};


// Class WebLoader
class WebLoader : public Loader {
    public:

        WebLoader(Config& config) : config(config){

        };

        std::vector<Ephemeris> load(){
            //Get Manifest Page, Check For 200
            //Parse Manifest, return []files
            //For Each File
            //GET raw Eph content
            //Create Eph
            //Append
            // Return ephs
        };

        Config& config;

        //store created ephs from content
        std::vector<Ephemeris> ephemerides;

};
// Class FileLoader
class FileLoader : public Loader {
    public:
        FileLoader(Config& config) : config(config){

        };

        std::vector<Ephemeris> load() {
            std::vector<Ephemeris> ephs;
            std::filesystem::path dir = config.dir;
        
            // Does Dir Exist?
            if (!(std::filesystem::exists(dir) && std::filesystem::is_directory(dir))){
                // Directory Does Not Exist
                // TODO - create a loadResult struct which has {ephs, ok, error}
            }
            // Determine num_files
            std::size_t count = 0;
            for (const auto& entry : std::filesystem::directory_iterator(dir)){
                if (entry.is_regular_file()){
                    count++;
                }
            }


            // For Each File in Files
            // Readfile
            //TODO - Each file contains 11k lines, we do not want to read the
            // entire file, then parse. A good starting method is to prob begin reading the file
            // assert it is a valid record, instantiate an eph
            // then as we progress down the file, continue to build out the state of the eph
            // at the end of file, our eph will be loaded.

            // Create Eph
            // Append
            // return ephs
            return ephs;
        };

        Config& config;
        /**
         * baseDir - baseDir to read files from
         * workers - num worker threads to spawn
         */
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


    // ephs = loader.load()

    
    return 0;
}