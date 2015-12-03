/*******************************************************************************
*                    Copyright 2010-2015 Stefano Sinigardi                     *
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


#define _USE_MATH_DEFINES


#include <cstdlib> 
#include <cmath> 
#include <ctime> 
#include <iostream> 
#include <iomanip>
#include <fstream> 
#include <limits>



#if (defined _MSC_VER) || (defined __INTEL_COMPILER)
#pragma warning(disable : 869)
#pragma warning(disable : 981)
#endif

#define NTAB          32 
#define IA            16807 
#define IM            2147483647 
#define AM           (1.0/(double)IM) 
#define IQ            127773 
#define IR            2836 
#define NDIV         (1+(IM-1)/(double)NTAB) 
#define EPS           1.2e-7 
#define RNMX          (1.0-EPS)
#define MP_KG         1.6726E-27                         // proton mass [kg]
#define MP_G          1.6726231E-24                      // proton mass [g]
#define MP_EV         938272013.0                        // proton mass [eV / c^2]
#define MP_MEV        938.272013                         // proton mass [MeV / c^2]
#define MP_MEV_INV    0.0010657890101641558821599424601
#define ME_KG         9.10938291E-31                     // electron mass [kg]
#define ME_G          9.10938291E-28                     // electron mass [g]
#define ME_EV         510998.928                         // electron mass [eV / c^2]
#define ME_MEV        0.510998928                        // electron mass [MeV / c^2]
#define ME_MEV_INV    1.9569512678116616322921131450984
#define ONE_eV_IN_MeV 0.000001
#define C             2.99792458e+10                     // cm / s
#define SQRT2         1.41421356


#ifndef M_PI
#define M_PI        3.141592653589793238462643383
#endif



class NumberGenerator
{
public:
  NumberGenerator(void);
  NumberGenerator(unsigned long inSeed);
  ~NumberGenerator(void);

  int operator()(unsigned long inValue) { return getUniformInteger(0, inValue - 1); }
  unsigned int getCurrentSeed(void) { return idum; }
  int getUniformInteger(int inFirst, int inLast);
  double getUniformFloat(double inFirst = 0., double inLast = 1.0);
  double getGaussianFloat(double inMean = 0, double inStdDev = 1);

private:
  long iy;
  long iv[NTAB];
  long idum;
  int iset;
  double gset;
};


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
    }
    while (rsq >= 1.0 || (rsq <= 0 && rsq >= 0.0)); // floating-point equality and inequality comparisons are unreliable so rsq == 0 is best done in this way

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

