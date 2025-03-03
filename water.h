//-----------------------------------------------------------------------------------------
/**
 * \file       water.h
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      File for generating water texture
 *
*/
//-----------------------------------------------------------------------------------------
#ifndef __WATER_H
#define __WATER_H
#include "pgr.h"
#include <time.h>
#include "data.h"

// size of one square
const int SQUARE_SIZE = 48; 

void generateWater(GLfloat waterVertices[]);
void createSquare(GLfloat waterVertices[], float x1, float y1, float x2, float y2, int index);

#endif 
