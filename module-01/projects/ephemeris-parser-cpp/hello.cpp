#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

// Obtain a List of ephemeris from MANIFEST URL
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size *nmemb);
    return size *nmemb;
}

struct httpResponse {
    CURLcode code;
    bool ok;
    std::string body;
    std::string error;
};

httpResponse httpGet(std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return httpResponse{
        CURLE_FAILED_INIT, false, "", "Unable to intialize curl"
    };

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res;
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) return httpResponse{
        res, false, "", "None 200 code."
    };

    return httpResponse {
        res, true, readBuffer, "success"
    };
}

// Holds Attributes and Methods for a Single Ephemeris
class Ephemeris {
    public:
                                //pass by ref vs copying, good for long strings, more efic
                                //              initializer list  used to directly init member vars
        Ephemeris(const std::string& fileName, const std::string&baseUrl) : fileName(fileName), baseUrl(baseUrl){ 
            // parse the filename into known parts
            parseFileName();
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

        std::string baseUrl;
        
        // the raw output from the eph url
        std::string rawBody;

        void print() const {
            std::cout   << "fileName: " << fileName << "\n"
                        << "\treferenceFrame: " << referenceFrame << "\n"
                        << "\tstarlinkId: " << starlinkId << "\n"
                        << "\tstatus: " << status << "\n"
                        << std::endl;
        }

        void saveToFIle(const std::string& path) {
            std::ofstream out(path + "/" + fileName);
            if (!out) {
                std::cerr << "Faild to open " << path + "/" + fileName << "\n";
                return;
            }
            out << rawBody;
        }

        // fetch obtains the contents for this ephemeris
        void fetch() {
            std::string url = baseUrl + fileName;
            httpResponse res;
            res = httpGet(url);
            if (res.ok) {
                rawBody = res.body;
            }
        }

    private:
        void parseFileName(){
            std::vector<std::string> tokens;
            std::istringstream iss(fileName);
            std::string token;

            char deliminator = '_';
            while(std::getline(iss, token, deliminator)) {
                tokens.push_back(token);
            }

            referenceFrame = tokens[0];
            noradId = tokens[1];
            starlinkId = tokens[2];
            internalId = tokens[3];
            status = tokens[4];
            internalExportId = tokens[5];
            classification = tokens[6];
        }
};


class StarlinkEphemeridesClient {
    public:
         StarlinkEphemeridesClient(const std::string& baseUrl) : baseUrl(baseUrl){
            getManifest();
    
        }

        void printAll() {
            /**
             *  can also write a loop in modern way
             *  for (const auto& eph : ephemerides) {
             *      eph.print();
             *  }
             */
            for (size_t i = 0; i < ephemerides.size(); ++i){
                ephemerides[i].print();
            }
        }

        // fetchAll obtains the contents for all ephemerides and parses them into structured types
        void fetchAll() {
            int count = 1;
            for (auto& eph : ephemerides) {
                std::cout << "Fetching and Saving... " << eph.fileName << std::endl; 
                eph.fetch();
                eph.saveToFIle("local-eph/cache");

                if (count == 3) {
                    break;
                }
                ++count;
            }
        }

        // Hosting Site
        std::string baseUrl;
        // List of Ephemeris
        std::vector<Ephemeris> ephemerides;

    private:
        void getManifest() {
            std::string url = baseUrl + "MANIFEST.txt";
            httpResponse res;
            res = httpGet(url);
            if (res.ok) {
                getManifestEntries(res.body);
            }

        }

        void getManifestEntries(std::string manifest) {
            std::istringstream iss(manifest);
            std::string line;
            while (std::getline(iss, line)) {
                ephemerides.emplace_back(line, baseUrl);
            }
        }
};


int main(int argc, char* argv[]){

    std::string endpoint = "https://api.starlink.com";
    const std::string path = "/public-files/ephemerides/";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--local") {
            endpoint = "http://localhost:8080";
        }
    }

    std::cout << "Using Endpoint: " << endpoint << std::endl;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    StarlinkEphemeridesClient client(endpoint+path);
    //client.printAll();
    client.fetchAll();
    curl_global_cleanup();

    std::cout << "total ephemerides " << client.ephemerides.size() << std::endl;
    return 0;
}