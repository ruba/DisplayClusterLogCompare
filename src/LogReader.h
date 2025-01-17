#pragma once

#include <string>
#include <fstream>

#include <unordered_map>
#include <map>

#include <vector>

#include "EntryData.h"

struct FrameComparisonData
{
    size_t TotalEntriesCount = 0;
    size_t IdenticalEntriesCount = 0;
    size_t DifferentEntriesCount = 0;
    size_t AbsentEntriesCount = 0;

    void Print();
    void Accumulate(FrameComparisonData const& Data);
};

struct FrameData
{
    int FrameNumber = 0;

    // key - ActorHashName
    std::unordered_map<size_t, std::vector<EntryData>> Data;
};

class LogReader
{
protected:
    std::ifstream FileStream;
    std::string Line;

    EntryData PreviousEntryData;
    int LastFrameNumber = -1;
    
    int LineNumber = 1;

    bool bAddPreviousEntryData = false;

public:
    bool ReadNextFrame(FrameData& OutFrameData, int FrameCounter);

    bool Open(std::string const& FileName);
    void Close();  

    bool IsEndOfFile() const { return FileStream.eof(); }
    bool IsOpen() const { return FileStream.is_open(); }
    bool IsFinished() const { return !FileStream.is_open() || FileStream.eof(); }

protected:
    bool ReadNextLine(EntryData& Data, int& OutFrame);
};
