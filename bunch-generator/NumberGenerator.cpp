/*******************************************************************************
*                    Copyright 2010-2017 Stefano Sinigardi                     *
* The program is distributed under the terms of the GNU General Public License *
*******************************************************************************/

/*******************************************************************************
* This file is part of tools-Propaga.                                          *
*                                                                              *
* tools-Propaga are free software: you can redistribute them and/or modify     *
* them under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* tools-Propaga are distributed in the hope that they will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
* GNU General Public License for more details.                                 *
*                                                                              *
* You should have received a copy of the GNU General Public License            *
* along with tools-Propaga. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************/



#include <cstdlib> 
#include <cmath> 
#include <ctime> 
#include <iostream> 
#include <iomanip>
#include <fstream> 
#include <limits>


#include "NumberGenerator.h"



int NumberGenerator::operator()(unsigned long inValue) {
  return getUniformInteger(0, inValue - 1); 
}


NumberGenerator::NumberGenerator()
{
  std::ifstream lStream("rand1.seed");
  if (lStream.good()) lStream >> idum;
  else idum = (long)time(0);
  for (int j = NTAB + 7; j >= 0; --j)
  {
    long k = idum / IQ;
    idum = IA*(idum - k*IQ) - IR*k;
    if (idum < 0) idum += IM;
    if (j < NTAB) iv[j] = idum;
  }
  iy = iv[0];
  iset = 0;
}



NumberGenerator::NumberGenerator(unsigned long inSeed)
{
  idum = inSeed;
  for (int j = NTAB + 7; j >= 0; --j)
  {
    long k = idum / IQ;
    idum = IA*(idum - k*IQ) - IR*k;
    if (idum < 0) idum += IM;
    if (j < NTAB) iv[j] = idum;
  }
  iy = iv[0];
  iset = 0;
}



NumberGenerator::~NumberGenerator()
{
  std::ofstream lStream("rand1.seed");
  if (!lStream.good()) std::cerr << "Unable to create file \"rand1.seed\"!" << std::endl;
  else lStream << idum;
}



int NumberGenerator::getUniformInteger(int inFirst, int inLast)
{
  int lNumber = (int)(inFirst + (inLast - inFirst + 1) * getUniformFloat());
  if (lNumber > inLast) lNumber = inLast;
  return lNumber;
}



double NumberGenerator::getUniformFloat(double inFirst, double inLast)
{
  double lTmp, lNumber;
  long k = idum / IQ;
  idum = IA*(idum - k*IQ) - IR*k;
  if (idum < 0) idum += IM;
  int j = (int)(iy / NDIV);
  iy = iv[j];
  iv[j] = idum;
  if ((lTmp = AM*iy) > RNMX) lNumber = RNMX;
  else lNumber = lTmp;
  return inFirst + (inLast - inFirst) * lNumber;
}



double NumberGenerator::getGaussianFloat(double inMean, double inStdDev)
{
  double fac, rsq, v1, v2;

  if (iset == 0)
  {
    do
    {
      v1 = 2.0*getUniformFloat() - 1.0;
      v2 = 2.0*getUniformFloat() - 1.0;
      rsq = v1*v1 + v2*v2;
    } while (rsq >= 1.0 || (rsq <= 0 && rsq >= 0.0)); // floating-point equality and inequality comparisons are unreliable so rsq == 0 is best done in this way

    fac = sqrt(-2.0*log(rsq) / rsq);
    gset = v1*fac;
    iset = 1;
    return v2*fac*inStdDev + inMean;
  }

  else
  {
    iset = 0;
    return gset*inStdDev + inMean;
  }
}

