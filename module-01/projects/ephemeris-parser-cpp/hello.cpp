#include <iostream>
#include <curl/curl.h>
#include <vector>
#include <sstream>
#include <string>


// Holds Attributes and Methods for a Single Ephemeris
class Ephemeris {
    public:
        Ephemeris(const std::string& fileName) : fileName(fileName){ 
            ParseFileName();
        }

        std::string fileName;
        //Usually Mean Equator Mean Equinox, an Earth-centered inertial reference frame used for orbital state vectors
        std::string referenceFrame;
        //Unique tracking ID used by organizations like Space-Track and CelesTrak for this object
        std::string noradId;
        //The spacecraft’s constellation identifier within the Starlink network
        std::string starLinkId;
        //Likely a SpaceX internal identifier for manufacturing, mission planning, or ephemeris generation
        std::string internalId;
        //Indicates the spacecraft is in active service rather than transfer, disposal, drift, or parking
        std::string status;
        //Likely a unique internal version ID for this specific ephemeris file generation
        std::string internalExportId;
        //Indicates the file is approved for public release
        std::string classification;

    private:

        void ParseFileName(){
            std::vector<std::string> tokens;
            std::istringstream iss(fileName);
            std::string token;

            char deliminator = '_';
            while(std::getline(iss, token, deliminator)) {
                tokens.push_back(token);
            }

            referenceFrame = tokens[0];
            noradId = tokens[1];
            starLinkId = tokens[2];
            internalId = tokens[3];
            status = tokens[4];
            internalExportId = tokens[5];
            classification = tokens[6];
        }

};


// Obtain a List of ephemeris from MANIFEST URL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size *nmemb);
    return size *nmemb;
}

class StarlinkEphemeridesClient {
    public:
         StarlinkEphemeridesClient(){
            // Obtain Manifest
            curl_global_init(CURL_GLOBAL_DEFAULT);
            GetManifest();
            curl_global_cleanup();

            // For Each Ephemeris
            for (size_t i = 0; i < ephemerides.size(); ++i){
                // Get Ephemeris
                Ephemeris e(ephemerides[i]);
                std::cout << e.fileName << std::endl;
                std::cout << e.starLinkId << std::endl;

            }
        }

    private:
        void GetManifest() {
            CURL* curl = curl_easy_init();
            if (!curl) return;

            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.starlink.com/public-files/ephemerides/MANIFEST.txt");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            CURLcode res;
            res = curl_easy_perform(curl);

            if (res != CURLE_OK) return;
            GetManifestEntries(readBuffer);
        }

        void GetManifestEntries(std::string manifest) {
            std::istringstream iss(manifest);
            std::string line;
            while (std::getline(iss, line)) {
                ephemerides.push_back(line);
            }
        }

        // Obtain a Single Ephemeris
        void GetEphemeris() {}

        // List of Ephemeris
        std::vector<std::string> ephemerides;

};


int main(){
    StarlinkEphemeridesClient client;
}