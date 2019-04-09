#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Example.h"
#include "ftStreams.h"
#include "ftTables.h"
#include "ftTypes.h"



#define TestFile "test.ex"
#define PricePerSquareFoot 1.75




void writeFile(int version)
{
    Example e;
    e.getInfo().major = version;

    HomeType ht;
    strcpy(ht.type, "Condo");
    ht.sq_ft = 1456;
    ht.num_rooms = 3;
    ht.num_gurages = 1;
    ht.price = (float)ht.sq_ft / (float)PricePerSquareFoot;
    e.getData().push_back(ht);

    strcpy(ht.type, "Town home");
    ht.sq_ft = 1598;
    ht.num_rooms = 2;
    ht.num_gurages = 2;
    ht.price = (float)ht.sq_ft / (float)PricePerSquareFoot;
    e.getData().push_back(ht);

    strcpy(ht.type, "Two story");
    ht.sq_ft = 900;
    ht.num_rooms = 2;
    ht.num_gurages = 0;
    ht.price = (float)ht.sq_ft / (float)PricePerSquareFoot;
    e.getData().push_back(ht);

    e.save(TestFile);
}


void readFile(void)
{
    Example e;
    e.load(TestFile);
    e.generateTypeCastLog("log.html");

    Example::DataArray::Iterator it = e.getData().iterator();
    while (it.hasMoreElements())
    {
        HomeType &ht = it.getNext();

        printf("-----------------\n");
        printf("Type             :%s\n", ht.type);
        printf("Square feet      :%i\n", ht.sq_ft);
        printf("Number of Rooms  :%i\n", ht.num_rooms);
        printf("Price            :%f\n", ht.price);
    }

}


int main()
{
    writeFile(200);
    readFile();
    return 0;
}