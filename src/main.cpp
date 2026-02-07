#include "log.h"
#include "config.h"

#include <httplib.h>
#include <filesystem>
#include <format>
#include <fstream>

int main() {
    Config::loadEnvironmentVariables();
    
    if (Config::STORAGE_DIRECTORY.empty()) {
        Log::error("Please set a STORAGE_DIRECTORY environment variable");
        return 0;
    }

    httplib::Server srv;

    /*
        Pings this server, returning a unix timestamp (in seconds) to the client
    */
    srv.Options("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");
        res.status = 204;
    });
    srv.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");

        res.set_header("Cache-Control", "no-store, no-cache, must-revalidate, proxy-revalidate");
        res.set_header("Pragma", "no-cache");
        res.set_header("Expires", "0");
        res.set_header("Surrogate-Control", "no-store");

        const time_t unixTimestamp = std::time(nullptr);
        res.set_content(std::to_string(unixTimestamp), "text/plain");
    });

    /*
        Get a list of all the files available for download in the storage directory
    */
    srv.Options("/files", [](const httplib::Request &, httplib::Response &res) {
        if (Config::DISABLE_INDEX) {
            res.status = 404;
            return;
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");
        res.status = 204;
    });
    srv.Get("/files", [](const httplib::Request &, httplib::Response &res) {
        if (Config::DISABLE_INDEX) {
            res.status = 404;
            return;
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");

        std::stringstream files;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(Config::STORAGE_DIRECTORY))
                files << entry.path().filename().string() << std::endl;
        } catch (const std::filesystem::filesystem_error& error) {
            Log::error(error.what());
            res.status = 500;
            return;
        }

        res.set_content(files.str(), "text/plain");
    });

    /*
        Get a file from the storage directory
    */
    srv.Options(R"(/(.*))", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");
        res.status = 204;
    });
    srv.Get(R"(/(.*))", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, HEAD, GET");

        std::string requestedFile = req.matches[1];

        // If requestedFile is empty, return 400
        if (requestedFile.empty()) {
            res.status = 400;
            return;
        }

        try {
            // Iterate over every file in the storage directory
            for (const auto& entry : std::filesystem::directory_iterator(Config::STORAGE_DIRECTORY)) {
                const auto filePath = entry.path();
                const auto fileName = filePath.filename().string();
                // If a match is found, return the file
                if (fileName == requestedFile) {
                    // Cache the file for a hour
                    res.set_header("Cache-Control", "public, max-age=3600");

                    res.set_header("Content-Disposition", std::format("inline; filename=\"{}\"", fileName));
                    res.set_file_content(filePath.string());
                    return;
                }
            }

            // Otherwise if no match was found, return 404
            res.status = 404;

        // In the case of an error while trying to iterate over every file
        } catch (const std::filesystem::filesystem_error &error) {
            Log::error(error.what());
            res.status = 500;
        }
    });

    Log::message(std::format("Server started on http://0.0.0.0:{}/!", Config::PORT));
    srv.listen("0.0.0.0", Config::PORT);
    
    return 0;
};