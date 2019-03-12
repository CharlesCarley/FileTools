#ifndef __Example_h__
#define __Example_h__


#include "ftFile.h"


class Example : public ftFile
{
public:
    Example();
    virtual ~Example();

private:

    virtual void*       getFBT(void);
    virtual FBTsize     getFBTlength(void);
    virtual int         notifyData(void* p, const Chunk& id);
    virtual int         writeData(ftStream* stream);

    // Initializes the memory tables. 
    // This will be the compiled output from makefbt 
    virtual int initializeTables(ftBinTables* tables);
};


#endif//__Example_h__
