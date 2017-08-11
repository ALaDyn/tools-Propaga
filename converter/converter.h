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


/*******************************
pID:
1 = proton
2 = antiproton
3 = electron
4 = positron
7 = photon


some special codes also exists, when energies in that contest are written in keV and not in MeV
101 = proton
102 = antiproton
103 = electron
104 = positron
********************************/


#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <streambuf>
#include <functional>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstring>
#include <string>
#include <limits> // for declaration of 'numeric_limits'
#include <ios>    // for declaration of 'streamsize'
#if defined(CINECA)
#include <inttypes.h>
#else
#include <cstdint>
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


// #define RESERVE_SIZE_VECTOR 10000
#ifdef RESERVE_SIZE_VECTOR
#define USE_RESERVE
#endif

#define ZERO    0.0   // used for the reference particle
#define TRASH_SIZE  2048
#define EPSILON   1.E-5

//#define C     299792458          // light speed [m / s]
//#define C     2.99792458e+10     // light speed [cm / s] (direttamente da Propaga)
#define Q       1.602176487E-19    // electric charge [C]
#define Q_nC    1.602176487e-10    // electric charge [nC]
//#define Q_sC  4.80320425e-10    // electric charge [statCoulomb]  commentata perche' Turchetti la usa un po' diversa
#define Q_sC    4.803262e-10       // electric charge [statCoulomb]  valore usato da Turchetti; nb: e' impreciso negli ultimi due decimali
#define MP_KG   1.6726E-27         // proton mass [kg]
#define MP_G    1.6726231E-24      // proton mass [g]
#define MP_EV   938272013.0        // proton mass [eV / c^2]
#define MP_MEV  938.272013         // proton mass [MeV / c^2]
#define MP_GEV  0.938272013        // proton mass [GeV / c^2]
#define MP_MEV_INV  0.0010657890101641558821599424601
#define ME_KG   9.10938291E-31     // electron mass [kg]
#define ME_G    9.10938291E-28     // electron mass [g]
#define ME_EV   510998.928         // electron mass [eV / c^2]
#define ME_MEV  0.510998928        // electron mass [MeV / c^2]
#define ME_GEV  5.10998928E-4      // electron mass [GeV / c^2]
#define ME_MEV_INV  1.9569512678116616322921131450984
#define TEMPO   0.0                // initial time [ns]
#define MQ      1.875E-7           // macro electric charge [nC]
#define type_ASTRA  3              // particle type: 1 (electrons), 2 (positrons), 3 (protons) //nb: non coerenti con Fluka ne' con la convenzione di Propaga
#define type_FLUKA  1              // particle type: 1 (protons), 3 (electrons)        //nb: non coerenti con Astra MA corretti con l'uso che ne si fa in Propaga
#define absorbed    0
#define ordinal_n   0
#define initial_status        5    // flag particelle: negative (not yet started or lost), 0-1 (passive), 3 (probe), 5 (standard)
#define DA_CM_A_MICRON        1.e+4
#define DA_MICRON_A_CM        1.e-4
#define FROM_TESLA_TO_GAUSS   1.e+4
#define DA_ERG_A_MEV          6.241509744512e+5 // conversione mia come sotto descritta
#define FROM_VOLT_TO_STATVOLT 3.335640951982e-3 // 1 statvolt = 299.792458 volts.
#define DIM_MAX_LINEA         1024
#define gamma_rel_inv(x)      (1.0 / sqrt(1.0 + x[3]*x[3]+x[4]*x[4]+x[5]*x[5]))
#define gamma_rel(x)          (sqrt(1.0 + x[3]*x[3]+x[4]*x[4]+x[5]*x[5]))     // gamma relativistico definito in funzione dei gamma*beta usati nei file


bool doubleEquality(double, double);

void preproc_from_micron_to_cm(char *, char *);
void preproc_from_NewP_to_OldP(char *, char *);
void convert_from_propaga_to_binary_dst(char *, char *, double, double, double);
void convert_from_binary_dst_to_ascii(char *, char *, double);
void convert_from_binary_plt_to_ascii(char *, char *, double);
void convert_from_propaga_to_astra(char *, char *, double);
void convert_from_astra_to_propaga(char *, char *, double, double);
void convert_from_propaga_to_astra_fixed(char *, char *, double, double);
void convert_from_fluka_to_propaga(char *, char *, double);
void convert_from_propaga_to_path(char *, char *, double, double, int, double, double, double);
void convert_from_path_to_propaga(char *, char *);
void convert_from_jasmine(char *, char *);
void preproc_remove_negatives(char *, char *, int);
void preproc_bunchfile_find_slowest(char *, char *);
void postproc_bunchfile_binning_2D(char *, char *, int, int, double, double, int, int, double, double, double);
void postproc_bunchfile_binning_1D(char *, char *, int, int, double, double, double);
void preproc_energyCut(char *, char *, double, double, double, int);
void preproc_energyCutAbove(char *, char *, double, double, int);
void preproc_angularCut(char *, char *, double, double);
void postproc_removeLostParticles(char *, char *, int);
void preproc_subsample(char *, char *, int);
void postproc_removeParticlesGoneToInfinity(char *, char *);
void from_ppg_to_vtk(char *, char *);   // INCOMPLETO
void postproc_split_p_and_e(char *, char *, char *);
void postproc_split_by_weight(char *, int);
void preproc_from_OldP_to_NewP(char *, char *, int, double, int);
void postproc_bunchfile_find_minmax(char *, char *, int);
void postproc_full_angle_and_energy(char *, char *, double, int, int, int, int, int);
void postproc_xyzE(char *, char *, double, int, int, int, int);

