/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "WWWAPIcURLImpl.h"

#include "base/Log.h"

#include "api/AssetAPI.h"

#if ! SAC_EMSCRIPTEN
#include <curl/curl.h>
#include <curl/easy.h>
#endif

#include <cstring>

#include <cstdio>
size_t write_data(void *ptr, size_t size, size_t nmemb, FileBuffer *fb) {
    fb->data = (uint8_t*)realloc(fb->data, size*nmemb + fb->size);
    memcpy(&fb->data[fb->size], ptr, size * nmemb);
    fb->size += size * nmemb;
    return nmemb;
}

FileBuffer WWWAPIcURLImpl::downloadFile(const std::string &url) {
#if ! SAC_EMSCRIPTEN
    //check url
    unsigned found = url.find_last_of("/");
    //if url ends with a '/' or there is no '/' at all, it's a bad url
    if (found >= url.size() - 1) {
        LOGE("Invalid URL! " << url);
        return FileBuffer();
    }


    CURL * curl = curl_easy_init();
    if (! curl) {
        LOGE("Could not init curl");
        return FileBuffer();
    }

    FileBuffer fb;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fb);
    CURLcode res = curl_easy_perform(curl);
    long HTTPCode;
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &HTTPCode);

    switch (res) {
        case 0:
            if (HTTPCode == 200) {
                LOGI("Successfully downloaded FileBuffer " << url << ". Total size: " << fb.size << "o.");
                return fb;
            } else {
                LOGE("Error with FileBuffer, http_code=" << HTTPCode);
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
#endif
    //return empty file buffer, since we got an error
    return FileBuffer();
}
