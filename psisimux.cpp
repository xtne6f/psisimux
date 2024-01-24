#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include "readonlympeg4file.hpp"

namespace
{
#ifdef _WIN32
std::string NativeToString(const wchar_t *s)
{
    std::string ret;
    for (; *s; ++s) {
        ret += 0 < *s && *s <= 127 ? static_cast<char>(*s) : '?';
    }
    return ret;
}
#else
std::string NativeToString(const char *s)
{
    return s;
}
#endif
}

#ifdef _WIN32
int wmain(int argc, wchar_t **argv)
#else
int main(int argc, char **argv)
#endif
{
    CReadOnlyMpeg4File file;
    int64_t seekBytes = 0;
    int seekMsec = 0;
    int64_t rangeBytes = -1;
    int rangeMsec = -1;
    bool printHeader = false;
#ifdef _WIN32
    std::wstring captionExtension;
    std::wstring dataExtension;
    const wchar_t *captionName = L"";
    const wchar_t *dataName = L"";
    const wchar_t *mediaName = L"";
    const wchar_t *destName = L"";
#else
    std::string captionExtension;
    std::string dataExtension;
    const char *captionName = "";
    const char *dataName = "";
    const char *mediaName = "";
    const char *destName = "";
#endif

    for (int i = 1; i < argc; ++i) {
        char c = '\0';
        std::string s = NativeToString(argv[i]);
        if (s[0] == '-' && s[1] && !s[2]) {
            c = s[1];
        }
        if (c == 'h') {
            fprintf(stderr, "Usage: psisimux [-s seek][-m msec_seek][-r range][-u duration][-b broadcast_id][-t time][-p][-x caption_ext][-y data_ext][-c caption_src][-d data_src] media_src dest\n");
            return 2;
        }
        bool invalid = false;
        if (i < argc - 2) {
            if (c == 's') {
                seekBytes = strtoll(NativeToString(argv[++i]).c_str(), nullptr, 10);
                invalid = seekBytes && seekMsec;
            }
            else if (c == 'm') {
                seekMsec = static_cast<int>(strtol(NativeToString(argv[++i]).c_str(), nullptr, 10));
                invalid = seekBytes && seekMsec;
            }
            else if (c == 'r') {
                rangeBytes = strtoll(NativeToString(argv[++i]).c_str(), nullptr, 10);
            }
            else if (c == 'u') {
                rangeMsec = static_cast<int>(strtol(NativeToString(argv[++i]).c_str(), nullptr, 10));
            }
            else if (c == 'b') {
                s = NativeToString(argv[++i]);
                uint16_t ids[3] = {};
                int n = 0;
                for (size_t j = 0; j < s.size() && n < 3; ) {
                    char *endp;
                    long id = strtol(s.c_str() + j, &endp, 10);
                    ids[n++] = static_cast<uint16_t>(id);
                    invalid = !(0 <= id && id <= 65535 && s.c_str() + j != endp && (!*endp || *endp == '/'));
                    if (invalid || !*endp) {
                        break;
                    }
                    j = endp - s.c_str() + 1;
                }
                invalid = invalid || n < 3;
                file.SetBroadcastID(ids[0], ids[1], ids[2]);
            }
            else if (c == 't') {
                file.SetTotStartTime(static_cast<uint32_t>(strtoul(NativeToString(argv[++i]).c_str(), nullptr, 10)));
            }
            else if (c == 'p') {
                printHeader = true;
            }
            else if (c == 'x') {
                captionExtension = argv[++i];
            }
            else if (c == 'y') {
                dataExtension = argv[++i];
            }
            else if (c == 'c') {
                captionName = argv[++i];
            }
            else if (c == 'd') {
                dataName = argv[++i];
            }
        }
        else if (i < argc - 1) {
            mediaName = argv[i];
            invalid = !mediaName[0];
        }
        else {
            destName = argv[i];
            invalid = !destName[0];
        }
        if (invalid) {
            fprintf(stderr, "Error: argument %d is invalid.\n", i);
            return 1;
        }
    }
    if (!mediaName[0] || !destName[0]) {
        fprintf(stderr, "Error: not enough arguments.\n");
        return 1;
    }

    if (!captionExtension.empty() || !dataExtension.empty()) {
#ifdef _WIN32
        size_t len = wcslen(mediaName);
#else
        size_t len = strlen(mediaName);
#endif
        size_t i = len;
        for (; i > 0; --i) {
            if (mediaName[i - 1] == '.') {
                break;
            }
            if (mediaName[i - 1] == '/' || mediaName[i - 1] == '\\') {
                i = 0;
                break;
            }
        }
        if (!captionExtension.empty()) {
            captionExtension.insert(0, mediaName, i > 0 ? i - 1 : len);
        }
        if (!dataExtension.empty()) {
            dataExtension.insert(0, mediaName, i > 0 ? i - 1 : len);
        }
    }

    const char *errorMessage = nullptr;
    if (!file.Open(mediaName, captionName[0] ? captionName : !captionExtension.empty() ? captionExtension.c_str() : nullptr,
                              dataName[0] ? dataName : !dataExtension.empty() ? dataExtension.c_str() : nullptr, errorMessage)) {
        fprintf(stderr, "Error: %s.\n", errorMessage ? errorMessage : "cannot open file");
        return 1;
    }

    int64_t fileSize = file.GetSize();
    int durationMsec = file.GetPositionMsecFromBytes(fileSize);
    if (seekMsec) {
        seekBytes = file.GetPositionBytesFromMsec(seekMsec < 0 ? durationMsec + seekMsec + 1 : seekMsec);
        if (seekBytes < 0 || seekBytes >= file.GetSize()) {
            fprintf(stderr, "Error: seek failed.\n");
            return 1;
        }
    }
    if (seekBytes) {
        seekBytes = file.SetPointer(seekBytes < 0 ? seekBytes + 1 : seekBytes,
                                    seekBytes < 0 ? CReadOnlyMpeg4File::MOVE_METHOD_END : CReadOnlyMpeg4File::MOVE_METHOD_BEGIN);
        if (seekBytes < 0) {
            fprintf(stderr, "Error: seek failed.\n");
            return 1;
        }
    }
    seekMsec = file.GetPositionMsecFromBytes(seekBytes);

    std::unique_ptr<FILE, decltype(&fclose)> destFile(nullptr, fclose);

#ifdef _WIN32
    if (destName[0] != L'-' || destName[1]) {
        destFile.reset(_wfopen(destName, L"wb"));
        if (!destFile) {
            fprintf(stderr, "Error: cannot create file.\n");
            return 1;
        }
    }
    else if (_setmode(_fileno(stdout), _O_BINARY) < 0) {
        fprintf(stderr, "Error: _setmode.\n");
        return 1;
    }
#else
    if (destName[0] != '-' || destName[1]) {
        destFile.reset(fopen(destName, "w"));
        if (!destFile) {
            fprintf(stderr, "Error: cannot create file.\n");
            return 1;
        }
    }
#endif

    if (printHeader) {
        uint8_t packet[188] = {};
        // NULL TS packet
        packet[0] = 0x47;
        packet[1] = 0x1f;
        packet[2] = 0xff;
        packet[3] = 0x10;
        for (int i = 0; i < 8; ++i) {
            packet[4 + i] = fileSize >> (8 * i) & 0xff;
            packet[12 + i] = seekBytes >> (8 * i) & 0xff;
        }
        for (int i = 0; i < 4; ++i) {
            packet[20 + i] = durationMsec >> (8 * i) & 0xff;
            packet[24 + i] = seekMsec >> (8 * i) & 0xff;
        }
        if (fwrite(packet, 1, sizeof(packet), destFile ? destFile.get() : stdout) != sizeof(packet)) {
            return 1;
        }
    }

    static uint8_t buf[48128];
    int64_t wroteSize = 0;
    for (;;) {
        int numRead = sizeof(buf);
        if (rangeBytes >= 0 && wroteSize + numRead >= rangeBytes) {
            numRead = static_cast<int>(rangeBytes - wroteSize);
        }
        numRead = file.Read(buf, numRead);
        if (numRead <= 0) {
            break;
        }
        if (fwrite(buf, 1, numRead, destFile ? destFile.get() : stdout) != static_cast<size_t>(numRead)) {
            return 1;
        }
        wroteSize += numRead;
        if ((rangeBytes >= 0 && wroteSize >= rangeBytes) ||
            (rangeMsec >= 0 && file.GetPositionMsecFromBytes(seekBytes + wroteSize) >= seekMsec + rangeMsec)) {
            break;
        }
    }
    if (fflush(destFile ? destFile.get() : stdout) != 0) {
        return 1;
    }
    return 0;
}
