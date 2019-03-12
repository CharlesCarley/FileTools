#include "Example.h"
#include "API.h"

#include "ftTables.h"
#include "ftStreams.h"









int main()
{
    {
        Example example;
        example.save("test.exff");
    }

    {


        Example example;
        
        if (example.parse("test.exff", ftFile::ParseMode::PM_READTOMEMORY) 
            != ftFile::FileStatus::FS_OK)
        {
            ftPrintf("File loading failed!");
            return -1;
        }


    }


    return 0;
}