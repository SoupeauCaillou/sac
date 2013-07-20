#include "WWWAPIcURLImpl.h"

#include "base/Log.h"

#include <curl/curl.h>
#include <curl/easy.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

FILE* WWWAPIcURLImpl::downloadFile(const std::string &url, FILE *file) {
    //split url to keep filename (eg split at the last '/' char)
    unsigned found = url.find_last_of("/");
    //if url ends with a '/' or there is no '/' at all, it's a bad url
    if (found >= url.size() - 1) {
        LOGE("Invalid URL! " << url);
        return 0;
    }

    if (! file) {
        // std::string filename = _assetAPI->getWritableAppDatasPath() + url.substr(found+1);

        // //do not overwrite existing files
        // file = fopen(filename.c_str(), "r");
        // if (file) {
        //     LOGE("File already exists! Abort.");
        //     return 0;

        file = tmpfile();
        if (! file) {
            LOGE("Error creating temp file");
            return 0;
        }
    }

    CURL * curl = curl_easy_init();
    if (! curl) {
        LOGE("Could not init curl");
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    long HTTPCode;
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &HTTPCode);

    switch (res) {
        case 0:
            if (HTTPCode == 200) {
                LOGI("Successfully downloaded file " << url);
                return file;
            } else {
                LOGE("Error with file, http_code=" << HTTPCode);
            }
            break;
        case 6:
            LOGE("Could not access url " << url);
            break;
        default:
            LOGE("Unknown error: " << res << " and http_code=" << HTTPCode);
            break;
    }
    curl_easy_cleanup(curl);
    //there was an error, return 0
    fclose(file);
    return 0;
}
