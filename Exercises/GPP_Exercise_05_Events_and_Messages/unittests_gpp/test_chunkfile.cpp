#include "stdafx.h"

#include "gep/memory/leakDetection.h"
#include "gep/chunkfile.h"

using namespace gep;

GEP_UNITTEST_GROUP(ChunkFile)
GEP_UNITTEST_TEST(ChunkFile, Basic)
{
    //Writing
    {
        auto file = new Chunkfile("unittest.bin", Chunkfile::Operation::write);
        SCOPE_EXIT { delete file; });
        file->startWriting("unittest", 1);
        for(int i=0; i<10; i++)
        {
            uint32 vals[4];
            vals[0] = i;
            vals[1] = i+1;
            vals[2] = i+2;
            vals[3] = i+3;
            file->startWriteChunk("testcase");
            file->write(i);
            file->write(i * 0.25f);
            file->startWriteChunk("array");
            file->writeArrayWithLength<uint32, uint32>(vals);
            file->endWriteChunk();
            file->endWriteChunk();
        }
        file->endWriting();
    }

    //Reading
    {
        auto readfile = new Chunkfile("unittest.bin", Chunkfile::Operation::read);
        SCOPE_EXIT{ delete readfile; });
        auto result = readfile->startReading("unittest");
        GEP_ASSERT(result == SUCCESS);
        GEP_ASSERT(readfile->getFileVersion() == 1);
        GEP_ASSERT(readfile->getCurrentChunkName() == "unittest");
        for(int i=0; i<10; i++)
        {
            uint32 vals[4];
            vals[0] = i;
            vals[1] = i+1;
            vals[2] = i+2;
            vals[3] = i+3;

            result = readfile->startReadChunk();
            GEP_ASSERT(result == SUCCESS);
            GEP_ASSERT(readfile->getCurrentChunkName() == "testcase");
            if(i == 4)
            {
                readfile->skipCurrentChunk();
            }
            else
            {
                int iTemp;
                GEP_ASSERT(readfile->read(iTemp) == sizeof(iTemp));
                GEP_ASSERT(iTemp == i);
                float fTemp;
                GEP_ASSERT(readfile->read(fTemp) == sizeof(fTemp));
                GEP_ASSERT(fTemp == i * 0.25f);
                result = readfile->startReadChunk();
                GEP_ASSERT(result == SUCCESS);
                GEP_ASSERT(readfile->getCurrentChunkName() == "array");
                GEP_ASSERT(readfile->currentChunkHasMoreData() == true);
                if(i == 6)
                {
                    readfile->skipCurrentChunk();
                }
                else
                {
                    if(i == 7)
                    {
                        ArrayPtr<uint32> data = readfile->readAndAllocateArray<uint32, uint32>(&g_stdAllocator);
                        SCOPE_EXIT{ GEP_DELETE_ARRAY(g_stdAllocator, data); });
                        GEP_ASSERT(data.length() == 4);
                        for(int j=0; j<4; j++)
                            GEP_ASSERT(data[j] == i+j);
                    }
                    else
                    {
                        uint32 data[4];
                        uint32 len;
                        GEP_ASSERT( readfile->read(len) == sizeof(len) );
                        GEP_ASSERT(len == 4);
                        GEP_ASSERT( readfile->readArray<uint32>(data) == sizeof(uint32) * GEP_ARRAY_SIZE(data) );
                        for(int j=0; j<4; j++)
                            GEP_ASSERT(data[j] == i+j);
                    }
                    readfile->endReadChunk();
                }
                readfile->endReadChunk();
            }
        }
        readfile->endReading();
    }
}
