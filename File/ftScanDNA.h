/*
-------------------------------------------------------------------------------

    Copyright (c) Charles Carley.

    Contributor(s): none yet.

-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#ifndef _ftScanDNA_h_
#define _ftScanDNA_h_

#include "ftTypes.h"

class ftScanDNA
{
    // The idea of the DNA scan is to jump to the
    // DNA1 block first, extract the table data,
    // then seek back to the file header offset
    // and read the chunks up to the DNA1 block.
    // Then, create associations with the structure
    // and member declarations along with every chunk.

public:
    ftScanDNA();
    ~ftScanDNA();

    // Extracts the needed flags from the file header 
    // or an error status if the flags are not found.
    int findHeaderFlags(skStream *stream);
    
    // An option to set the required flags if they have 
    // already been extracted from the file 
    void setFlags(int headerFlags)
    {
        m_headerFlags = headerFlags;
    }

    int scan(skStream *stream);

    // Access to the found block
    // Note: that this class does not manage the 
    //       memory allocated for m_foundBlock
    //       It was allocated with malloc, so the 
    //       memory should be released with a call 
    //       to free
    void *getDNA()
    {
        return m_foundBlock;
    }

    FBTsize getLength()
    {
        return m_foundLen;
    }

private:
    void *  m_foundBlock;
    FBTsize m_foundLen;
    int     m_headerFlags;
};

#endif  //_ftScanDNA_h_
