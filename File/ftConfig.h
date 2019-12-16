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
#ifndef _ftConfig_h_
#define _ftConfig_h_

#define FT_DEBUG 3              // Detail
#define FT_MAX_TABLE 5000       // Maximum number of elements in a table
#define FT_MAX_ID 64            // Maximum character array length
#define FT_MAX_MEMBERS 256      // Maximum number of members in a struct or class.
#define FT_DEF_ALLOC 2048       // Table default allocation size
#define FT_TYLE_LEN_VALIDATE 1  // Write a validation file (use MakeFBT.cmake->ADD_FT_VALIDATOR to add a self validating build)
#define FT_ARR_DIM_MAX 3        // Maximum dimensional array, EG; (int m_member[..][..] -> [FT_ARR_DIM_MAX])
#define FT_MAX_TOK 32            // Maximum allowed C/C++ name (keyword, identifier, ...)

#endif//_ftConfig_h_
