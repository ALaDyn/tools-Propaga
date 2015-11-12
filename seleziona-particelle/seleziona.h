/******************************************************************************
Copyright 2010, 2011, 2012 Stefano Sinigardi
The program is distributed under the terms of the GNU General Public License
******************************************************************************/

/**************************************************************************
    This file is part of "seleziona_particelle".

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    **************************************************************************/


#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <iomanip>


#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#pragma warning(disable : 869)
#pragma warning(disable : 981)
#endif


#define RESERVE_SIZE_VECTOR 10000 


#define ZERO      0
#define ZERODOUBLE    0.0
#define USA_NUOVI

//#define C     299792458     // light speed [m / s]
//#define C     2.99792458e+10    // light speed [cm / s] (direttamente da Propaga)
#define Q     1.602176487E-19   // electric charge [C]
#define Q_nC    1.602176487e-10   // electric charge [nC]
#define MP_KG   1.6726E-27      // proton mass [kg]
#define MP_G    1.6726231E-24   // proton mass [g]
#define MP_EV   938272013.0     // proton mass [eV / c^2]
#define MP_MEV    938.272013      // proton mass [MeV / c^2]
#define MP_MEV_INV  0.0010657890101641558821599424601
#define ME_KG   9.10938291E-31    // electron mass [kg]
#define ME_G    9.10938291E-28    // electron mass [g]
#define ME_EV   510998.928      // electron mass [eV / c^2]
#define ME_MEV    0.510998928     // electron mass [MeV / c^2]
#define ME_MEV_INV  1.9569512678116616322921131450984
#define T     0.0         // initial time [ns]
#define MQ      1.875E-7      // macro electric charge [nC]
#define type    3         // particle type: 1 (electrons), 2 (positrons), 3 (protons)
#define element   -1          // element to which the particle belong (it's an ordinal number used in Propaga output; being not defined in ASTRA, we put it to -1)
#define absorbed  0
#define ordinal_n 0
#define initial_status      5     // flag particelle: negative (not yet started or lost), 0-1 (passive), 3 (probe), 5 (standard)
#define DA_CM_A_MICRON      1.e+4
#define FROM_TESLA_TO_GAUSS   1.0e+4
#define DA_ERG_A_MEV      6.241509744512e+5 // conversione mia come sotto descritta
#define FROM_VOLT_TO_STATVOLT 3.335640951982e-3 // 1 statvolt = 299.792458 volts.
#define DIM_MAX_LINEA     1024
#define EPSILON         1.0e-6

bool doubleEquality(double, double);
void estrai_da_sorgente(char *, char *, char *);
