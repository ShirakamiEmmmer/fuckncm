//
// Created by Shirakami Emmmer on 2020/3/22.
//
#include <map>
#include <string>
#include <iostream>
#include <json-c/json.h>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>

void fixStr(std::string &str, size_t size) {
    for (int i = 0; i < size; ++i) { str[i] = str[i + 1]; }
    for (int j = size;; --j) {
        if (str[j] == 0x22) {
            str[j] = '\0';
            break;
        }
    }
}

std::string addArtist(struct json_object *jsonFile) {
    struct json_object *iter = nullptr;
    struct json_object *result = json_object_object_get(jsonFile, "artist");

    std::string allArtist;
    std::string eachArtist;

    for (int i = 0; i < json_object_array_length(result); ++i) {
        iter = json_object_array_get_idx(result, i);
        iter = json_object_array_get_idx(iter, 0);
        eachArtist = json_object_to_json_string(iter);
        fixStr(eachArtist, eachArtist.size());

        if (i == json_object_array_length(result) - 1) {
            allArtist += eachArtist;
            break;
        } else {
            eachArtist[eachArtist.size() - 1] = 0x20;
            eachArtist[eachArtist.size() - 2] = 0x2C;
            allArtist += eachArtist;
        }
    }
    return allArtist;
}

int main() {
    TagLib::FileRef f("tmp/out.mp3");

    struct json_object *jsonP = nullptr;
    struct json_object *result = nullptr;
    std::string buffer;
    jsonP = json_object_from_file("tmp/tmp.json");

    result = json_object_object_get(jsonP, "musicName");
    buffer = json_object_to_json_string(result);
    fixStr(buffer, buffer.size());
    TagLib::String musicName(buffer, TagLib::String::UTF8);
    f.tag()->setTitle(musicName);

    TagLib::String artist(addArtist(jsonP), TagLib::String::UTF8);
    f.tag()->setArtist(artist);

    result = json_object_object_get(jsonP, "album");
    buffer = json_object_to_json_string(result);
    fixStr(buffer, buffer.size());
    TagLib::String album(buffer, TagLib::String::UTF8);
    f.tag()->setAlbum(album);


    f.save();
    return 0;
}