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

#pragma once 

#define _USE_MATH_DEFINES


#include <cstdlib> 
#include <cmath> 
#include <ctime> 
#include <iostream> 
#include <iomanip>
#include <fstream> 
#include <limits>



//#define USE_STRINGENT_CHECK


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
  NumberGenerator(unsigned long );
  ~NumberGenerator(void);

  int operator()(unsigned long);
  int getUniformInteger(int , int );
  double getUniformFloat(double inFirst = 0., double inLast = 1.0);
  double getGaussianFloat(double inMean = 0, double inStdDev = 1);

private:
  long iy;
  long iv[NTAB];
  long idum;
  int iset;
  double gset;
};

