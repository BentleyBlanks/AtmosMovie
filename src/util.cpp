#include "util.h"
#include <ofMain.h>

std::string addKeyFrameInPath(int keyFrame, std::string path)
{
    string baseName = ofFilePath::getBaseName(path);
    string extension = ofFilePath::getFileExt(path);
    string pathWithoutName = ofFilePath::getEnclosingDirectory(path);

    int count = getNumOfDigits<float>(keyFrame);
    // 至多支持999999个关键帧
    int numOfZero = 6 - count;
    // XXXXXX / 0XXXXX / 00XXXX / 000XXX / 0000XX / 00000X
    for(int i = 0; i < numOfZero; i++)
    {
        baseName += i == 0 ? "_" : "";
        baseName += ofToString(0);
    }

    baseName += ofToString(keyFrame) + ".";

    return pathWithoutName + baseName + extension;
}

