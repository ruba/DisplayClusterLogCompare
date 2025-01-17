#include "LogReader.h"

#include <iostream>

bool LogReader::Open(std::string const& FileName)
{
    FileStream.open(FileName);

    if (!FileStream.is_open())
    {
        return false;
    }

    return true;
}

bool LogReader::ReadNextLine(EntryData& Data, int& OutFrame)
{
    getline(FileStream, Line);
    
    LineNumber++;

    static std::string LineMarker = "(DataIntegrityTest)";
    int LabelIndex = static_cast<int>(Line.find(LineMarker));

    if (LabelIndex < 0)
    {
        return false;
    }

    if (!Data.ParseLine(Line, OutFrame))
    {
        return false;
    }

    return true;
}

int GetFrameNumber(int FrameNumber, int FrameCounter)
{
    static const int FrameOverflow = 1000;

    // Account overflow
    int k = int(FrameCounter / FrameOverflow) * FrameOverflow;
    return k + FrameNumber;
}

bool LogReader::ReadNextFrame(FrameData& FrameData, int FrameCounter)
{
    if (IsFinished())
    {
        return false;
    }

    FrameData.FrameNumber = -1;
    FrameData.Data.clear();

    while (!FileStream.eof())
    {
        EntryData Data(LineNumber);

        int OutFrame = -1;

        if (ReadNextLine(Data, OutFrame))
        {
            // Add LineData from previous Reads
            if (bAddPreviousEntryData)
            {
                std::size_t ActorHash = std::hash<std::string>{}(PreviousEntryData.GetActorName());

                FrameData.Data[ActorHash].push_back(PreviousEntryData);
                FrameData.FrameNumber = GetFrameNumber(LastFrameNumber, FrameCounter);

                bAddPreviousEntryData = false;

                // Previous Read contained only one entry - save current and exit
                if (OutFrame != LastFrameNumber)
                {
                    PreviousEntryData = Data;
                    LastFrameNumber = OutFrame;

                    bAddPreviousEntryData = true;
                    break;
                }
            }

            // Init FrameNumber
            if (FrameData.FrameNumber == -1)
            {
                FrameData.FrameNumber = GetFrameNumber(OutFrame, FrameCounter);
            }

            // Add to Entry to the FrameData
            if (FrameData.FrameNumber == OutFrame)
            {
                std::size_t ActorHash = std::hash<std::string>{}(Data.GetActorName());
                FrameData.Data[ActorHash].push_back(Data);
            }

            // Reached next Frame - cache and add on next invocation
            if (FrameData.FrameNumber != OutFrame)
            {
                PreviousEntryData = Data;
                LastFrameNumber = OutFrame;

                bAddPreviousEntryData = true;
                break;
            }
        }
    }

    return (FrameData.FrameNumber != -1);
}

void LogReader::Close()
{
    if (FileStream.is_open())
    {
        FileStream.close();
    }
}

void FrameComparisonData::Print()
{
    std::cout << std::endl << "Final Results: ";

    if (TotalEntriesCount == IdenticalEntriesCount)
    {
        std::cout << GreenColor << "OK" << WhiteColor;
    }
    else
    {
        std::cout << RedColor << "FAILED" << WhiteColor << std::endl;
    }

    std::cout << "   Total Entries: " << TotalEntriesCount << std::endl;
    std::cout << "   Identical Entries: " << IdenticalEntriesCount << std::endl;
    std::cout << "   Different Entries: " << DifferentEntriesCount << std::endl;
    std::cout << "   Absent Entries: " << AbsentEntriesCount << std::endl;
}

void FrameComparisonData::Accumulate(FrameComparisonData const& Data)
{
    TotalEntriesCount += Data.TotalEntriesCount;
    IdenticalEntriesCount += Data.IdenticalEntriesCount;
    DifferentEntriesCount += Data.DifferentEntriesCount;
    AbsentEntriesCount += Data.AbsentEntriesCount;
}
