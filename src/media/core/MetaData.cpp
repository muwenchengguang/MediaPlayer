//
// Created by richie on 7/6/17.
//

#include "MetaData.h"

namespace peng {

void MetaData::setCString(int key, std::string value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            mItems.erase(it);
            break;
        }
    }
    Item item;
    item.key = key;
    item.str_value = value;
    mItems.push_back(item);
}

void MetaData::setInt32(int key, int32_t value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            mItems.erase(it);
            break;
        }
    }
    Item item;
    item.key = key;
    item.int_value = value;
    mItems.push_back(item);
}

int MetaData::findCString(int key, std::string& value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            value = item.str_value;
            return 0;
        }
    }
    return -1;
}

int MetaData::findInt32(int key, int32_t& value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            value = item.int_value;
            return 0;
        }
    }
    return -1;
}

void MetaData::setInt64(int key, int64_t value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            mItems.erase(it);
            break;
        }
    }
    Item item;
    item.key = key;
    item.int64_value = value;
    mItems.push_back(item);
}

int MetaData::findInt64(int key, int64_t& value) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            value = item.int64_value;
            return 0;
        }
    }
    return -1;
}

void MetaData::setData (int key, const sp<MediaBuffer> &buffer) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            mItems.erase(it);
            break;
        }
    }
    Item item;
    item.key = key;
    item.buffer = buffer;
    mItems.push_back(item);
}

int MetaData::findData (int key, sp<MediaBuffer> &buffer) {
    for (std::list<Item>::iterator it = mItems.begin (); it != mItems.end (); it++) {
        Item item = *it;
        if (key == item.key) {
            buffer = item.buffer;
            return 0;
        }
    }
    return -1;
}

}
