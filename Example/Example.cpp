#include "Example.h"
#include "ftTables.h"


Example::Example() :
    // File identifier, it’s the first 12 bytes in the file 
    // [0-7]    identifier
    // [7-12]   file version
    ftFile("EXAMPLE_v100")
{
    m_version = 100;
}


Example::~Example()
{
}




int Example::notifyData(void* p, const Chunk& id)
{
    return FS_OK;
}



int Example::writeData(ftStream* stream)
{

    return FS_OK;
}






int Example::initializeTables(ftBinTables* tables)
{
    return tables->read(getFBT(), getFBTlength(), false) ? FS_OK : FS_FAILED;
}



extern unsigned char TablesFBT[];
extern int TablesLen;

void* Example::getFBT(void)
{
    return (void*)TablesFBT;
}

FBTsize Example::getFBTlength(void)
{
    return TablesLen;
}

