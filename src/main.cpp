#include "log.h"
#include "config.h"
#include "helper.h"

#include <httplib.h>
#include <filesystem>
#include <format>
#include <fstream>

namespace fs = std::filesystem;

int main() {
    Config::loadEnvironmentVariables();
    
    if (Config::STORAGE_DIRECTORY.empty() || !fs::exists(Config::STORAGE_DIRECTORY) || !fs::is_directory(Config::STORAGE_DIRECTORY)) {
        Log::error("Please set a STORAGE_DIRECTORY environment variable");
        return 0;
    }

    httplib::Server srv;

    /*
        Pings this server, returning a unix timestamp (in seconds) to the client
    */
    srv.Options("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");
        res.status = 204;
    });
    srv.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");

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
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");
        res.status = 204;
    });
    srv.Get("/files", [](const httplib::Request &, httplib::Response &res) {
        if (Config::DISABLE_INDEX) {
            res.status = 404;
            return;
        }

        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");

        std::stringstream files;

        try {
            for (const auto& entry : fs::directory_iterator(Config::STORAGE_DIRECTORY)) {
                auto filename = entry.path().filename().u8string();
                files << std::string(filename.begin(), filename.end()) << std::endl;
            }
        } catch (const fs::filesystem_error& error) {
            Log::error(error.what());
            res.status = 500;
            return;
        }

        res.set_content(files.str(), "text/plain; charset=utf-8");
    });

    /*
        Get a file from the storage directory
    */
    srv.Options(R"(/(.*))", [](const httplib::Request &, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");
        res.status = 204;
    });
    srv.Get(R"(/(.*))", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET");

        std::string requestedFile = req.matches[1];

        // If requestedFile is empty or if it contains characters that could cause a security risk, return 400
        if (requestedFile.empty() || Helper::containsUnallowedPathCharacters(requestedFile)) {
            res.status = 400;
            return;
        }

        try {
            // Assemble the full file path
            fs::path requestedFilePath = fs::path(Config::STORAGE_DIRECTORY) / requestedFile;

            // Check if the file exists and return it with some headers if it does
            if (fs::exists(requestedFilePath) && fs::is_regular_file(requestedFilePath)) {
                // Cache the file for a hour
                res.set_header("Cache-Control", "public, max-age=3600");

                res.set_header("Content-Disposition", std::format("inline; filename=\"{}\"", requestedFile));
                res.set_file_content(requestedFilePath.string());
            } else {
                res.status = 404;
            }

        // In case of an error while trying to check for/return the file
        } catch (const fs::filesystem_error &error) {
            Log::error(error.what());
            res.status = 500;
        }
    });

    Log::message(std::format("Server started on http://0.0.0.0:{}/!", Config::PORT));
    srv.listen("0.0.0.0", Config::PORT);
    
    return 0;
};