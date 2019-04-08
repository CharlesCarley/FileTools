#include "Example.h"
#include "ftStreams.h"
#include "ftTables.h"
#include "ftTypes.h"


#define TestFile "test.ex"

void writeFileVersion(int version)
{
    Example e;
    e.getInfo().major = version;
    e.save(TestFile);
}


void readFileVersion(void)
{
    Example e;
    e.load(TestFile);

    ftPrintf("Version = %i\n", e.getInfo().major);
}


int main()
{
    writeFileVersion(200);
    readFileVersion();
    return 0;
}