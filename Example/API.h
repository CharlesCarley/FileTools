#ifndef __DATA_API__
#define __DATA_API__





typedef struct FileData
{
    char version[4];
}FileData;


typedef struct Record
{
    int id;
    float value;

}Record;



#endif//__DATA_API__
