#ifdef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <stdio.h>
#include <array.h>


typedef struct Vector{

    double X;
    double Y;
    double Z;

} Vector;


typedef struct Track{

    //int id;
    size_t cryo;

    Vector start;
    Vector end;
    Vector dir;
    Vector chargeCenter;

    double length;

} Track;


#endif // UTILS_H