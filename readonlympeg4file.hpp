#ifndef INCLUDE_READ_ONLY_MPEG4_FILE_HPP
#define INCLUDE_READ_ONLY_MPEG4_FILE_HPP

#include "psiarchivereader.hpp"
#include <map>
#include <memory>
#include <utility>
#include <vector>

#ifndef MASK_OFF_SPS_CS45_FLAGS
#define MASK_OFF_SPS_CS45_FLAGS 1
#endif

class CReadOnlyMpeg4File
{
    static const uint32_t READ_BOX_SIZE_MAX = 64 * 1024 * 1024;
    static const uint32_t BLOCK_SIZE_MAX = 65536;
    static const uint32_t BLOCK_LIST_SIZE_MAX = 1000000;
    static const uint32_t VIDEO_SAMPLE_MAX = 2 * 1024 * 1024;
    static const uint32_t AUDIO_SAMPLE_MAX = 8184;
    static const uint32_t CAPTION_FORWARD_MSEC = 500;
    static const uint16_t VIDEO_PID = 0x0100;
    static const uint16_t AUDIO1_PID = 0x0110;
    static const uint16_t CAPTION_PID = 0x0130;
    static const uint16_t PMT_PID = 0x01F0;
    static const uint16_t PCR_PID = 0x01FF;
    static const uint16_t DISPLACED_PID = 0x1E00;
    static const uint32_t PSI_MAX_STREAMS = 32;
public:
    enum MOVE_METHOD {
        MOVE_METHOD_BEGIN,
        MOVE_METHOD_CURRENT,
        MOVE_METHOD_END,
    };
    CReadOnlyMpeg4File()
        : m_fp(nullptr, fclose)
        , m_nid(1)
        , m_tsid(2)
        , m_sid(3)
        , m_totStart(946717200) // 2000-01-01T09:00:00
    {}
    ~CReadOnlyMpeg4File() { Close(); }
#ifdef _WIN32
    bool Open(const wchar_t *path, const wchar_t *captionPath, const wchar_t *dataPath, const char *&errorMessage);
#else
    bool Open(const char *path, const char *captionPath, const char *dataPath, const char *&errorMessage);
#endif
    void Close();
    int Read(uint8_t *pBuf, int numToRead);
    int64_t SetPointer(int64_t distanceToMove, MOVE_METHOD moveMethod);
    int64_t GetSize() const;
    int GetPositionMsecFromBytes(int64_t posBytes) const;
    int64_t GetPositionBytesFromMsec(int msec) const;
    void SetBroadcastID(uint16_t nid, uint16_t tsid, uint16_t sid);
    void SetTotStartTime(uint32_t t);
private:
    struct BLOCK_100MSEC {
        uint32_t pos;
        uint8_t counterV;
        uint8_t counterA[2];
        uint8_t counterC;
    };
    struct PSI_COUNTER_INFO {
        uint16_t mappedPid;
        uint8_t currentCounter;
        std::vector<uint8_t> counterList;
    };
    static inline uint32_t ArrayToDWORD(const uint8_t *data) {
        return data[3] | data[2] << 8 | data[1] << 16 | static_cast<uint32_t>(data[0]) << 24;
    }
    static inline int64_t ArrayToInt64(const uint8_t *data) {
        return ArrayToDWORD(&data[4]) | static_cast<int64_t>(ArrayToDWORD(&data[0])) << 32;
    }
    bool InitializeTable(const char *&errorMessage);
    bool ReadVideoSampleDesc(int64_t trakBoxPos, bool &fHevc, std::vector<uint8_t> &spsPps, std::vector<uint8_t> &buf) const;
    bool ReadAudioSampleDesc(int64_t trakBoxPos, uint8_t *adtsHeader, std::vector<uint8_t> &buf) const;
    bool ReadSampleTable(int64_t trakBoxPos, std::vector<int64_t> &stso, std::vector<uint32_t> &stsz,
                         std::vector<int64_t> &stts, std::vector<uint32_t> *ctts, int64_t &editTimeOffset, std::vector<uint8_t> &buf) const;
    bool InitializeBlockList(const char *&errorMessage);
    bool ReadCurrentBlock();
    bool InitializePsiCounterInfo(const char *&errorMessage);
    std::pair<int64_t, int64_t> FindBoxPosition(const char *path, int64_t currentBoxPos) const;
    int ReadBox(const char *path, std::vector<uint8_t> &data, int64_t currentBoxPos) const;
    int ReadSample(size_t index, const std::vector<int64_t> &stso, const std::vector<uint32_t> &stsz, std::vector<uint8_t> *data) const;
    static void AddTsPacketsFromPsi(std::vector<uint8_t> &buf, const uint8_t *psi, size_t psiSize, uint8_t &counter, uint16_t pid);
    static bool Add16TsPacketsFromPsi(std::vector<uint8_t> &buf, const uint8_t *psi, size_t psiSize, uint16_t pid);
    static size_t CreatePat(uint8_t *data, uint16_t tsid, uint16_t sid);
    static size_t CreatePatFromPat(uint8_t *data, const std::vector<uint8_t> &pat, uint16_t &firstPmtPid);
    static size_t CreateNit(uint8_t *data, uint16_t nid);
    static size_t CreateSdt(uint8_t *data, uint16_t nid, uint16_t tsid, uint16_t sid);
    static size_t CreateEmptyEitPf(uint8_t *data, uint16_t nid, uint16_t tsid, uint16_t sid);
    static size_t CreateTot(uint8_t *data, uint32_t t);
    static size_t CreatePmt(uint8_t *data, uint16_t sid, bool fHevc, bool fAudio2, bool fCaption);
    static bool AddPmtPacketsFromPmt(std::vector<uint8_t> &buf, const std::vector<uint8_t> &pmt, const std::map<uint16_t, PSI_COUNTER_INFO> &pidMap,
                                     bool fHevc, bool fAudio2, bool fCaption);
    static uint8_t CreatePmtProgramInfoFromProgramInfo(uint8_t *data, const uint8_t *programInfo, uint16_t programInfoLength);
    static size_t CreatePmt2ndLoop(uint8_t *data, bool fHevc, bool fAudio2, bool fCaption);
    static size_t CreateHeader(uint8_t *data, uint8_t unitStart, uint8_t adaptation, uint8_t counter, uint16_t pid);
    static size_t CreatePcrAdaptation(uint8_t *data, uint32_t pcr45khz);
    static size_t CreatePesHeader(uint8_t *data, uint8_t streamID, bool fDataAlignment, uint16_t packetLength, uint32_t pts45khz, uint8_t stuffingSize);
    static size_t CreateAdtsHeader(uint8_t *data, int profile, int freq, int ch, int bufferSize);
    static size_t NalFileToByte(std::vector<uint8_t> &data, bool &fIdr, bool fHevc);
    static uint32_t CalcCrc32(const uint8_t *data, size_t len, uint32_t crc = 0xFFFFFFFF);

    std::unique_ptr<FILE, decltype(&fclose)> m_fp;
    uint16_t m_nid, m_tsid, m_sid;
    uint32_t m_totStart;
    std::vector<std::pair<int64_t, std::vector<uint8_t>>> m_captionList;
    std::vector<int64_t> m_stsoV, m_stsoA[2];
    std::vector<uint32_t> m_stszV, m_stszA[2];
    std::vector<int64_t> m_sttsV, m_sttsA[2];
    std::vector<uint32_t> m_cttsV;
    uint32_t m_timeScaleV, m_timeScaleA[2];
    uint32_t m_offsetPtsV, m_offsetPtsA[2];
    bool m_fHevc;
    std::vector<uint8_t> m_spsPps;
    uint8_t m_adtsHeader[2][7];
    std::vector<BLOCK_100MSEC> m_blockList;
    std::vector<BLOCK_100MSEC>::const_iterator m_blockInfo;
    std::vector<uint8_t> m_blockCache;
    int64_t m_pointer;
    CPsiArchiveReader m_psiDataReader;
    std::map<uint16_t, PSI_COUNTER_INFO> m_psiCounterInfoMap;
};

#endif // INCLUDE_READ_ONLY_MPEG4_FILE_HPP
