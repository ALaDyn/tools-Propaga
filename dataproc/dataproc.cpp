/******************************************************************************
Copyright 2010, 2011, 2012, 2013, 2014, 2015 Stefano Sinigardi
The program is distributed under the terms of the GNU General Public License
******************************************************************************/

/**************************************************************************
This file is part of "tools-Propaga".

tools-Propaga are free software: you can redistribute them and/or modify
them under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tools-Propaga are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tools-Propaga. If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <string>
#include <cstdlib>  // for atoi
#include <vector>

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#pragma warning(disable : 869)
#pragma warning(disable : 981)
#endif


#define MP_EV       938272013.0   // proton mass [eV / c^2]
#define C         2.99792458e+10  // light speed [cm / s] (direttamente da Propaga)
//#define C         299792458.0   // light speed [m / s]
#define DIM_MAX_LINEA   1024
#define NUMERO_PARAMETRI  19



typedef struct
{
  double v[6];
  double gamma;
  double xprime, yprime, phase;
  double pz_diff;
  int id_type;
  int lost;
  int id_number;
  int charge_int;
  double charge_double;
  double mass;
  double time;
  double weight;
} particella;



int main(int argc, char *argv[])
{
  if (argc != 4 && argc != 5)
  {
    std::cout << "Usage:   dataproc.exe  file.input  selectedData.output (will be appended to the file)  opType  column(optional)" << std::endl;
    std::cout << "where opType can be the number of the particle to be extracted or" << std::endl;
    std::cout << "'-1' when looking for the maximum values (both positive and negative), '-2' for the minimum ones (both + and -)" << std::endl;
    std::cout << "and the column must be given in these two cases (useful for building bunch envelopes);" << std::endl;
    std::cout << "with -3 it will calculate emittances from dumps (propaga)" << std::endl;
    std::cout << "the time step used in the stdin of propaga;" << std::endl;
    std::cout << "with -4 it will calculate emittances from emitt.ppg files made by parallel_propaga" << std::endl;
    std::cout << "with -5 it will calculate emittances from emitt.ppg files made by parallel_propaga (OLD)" << std::endl;
    return 255;
  }

  int inputArgv3 = atoi(argv[3]);
  int Numerostep;
  char* numerostep;

  int lunghezza_nomefile = (int)strlen(argv[1]);
  int lunghezza_step = 0;
  int k = lunghezza_nomefile - 5;
  int primacifra;

  if (inputArgv3 != -4)
  {
    while (1)
    {
      if (argv[1][k] == '.')
      {
        primacifra = k + 1;
        break;
      }
      k--;
      lunghezza_step++;
    }

    numerostep = new char[lunghezza_step];

    for (int j = 0; j < lunghezza_step; j++)
    {
      numerostep[j] = argv[1][primacifra + j];
    }

    Numerostep = atoi(numerostep);
  }

  double timestep;

  std::ifstream inputFile(argv[1]);
  std::ofstream selectedData(argv[2], std::ios::app);


  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  inputFile.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  inputFile.clear();
  inputFile.seekg(0, std::ios::beg);
  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne <= 0)
  {
    printf("File is empty\n", contacolonne);
    inputFile.close();
    selectedData.close();
    return -1;
  }

  std::vector<double> emitt_param(NUMERO_PARAMETRI, 0);
  std::vector< std::vector<double> > emitt_timestep(2, emitt_param);
  std::vector<double> emitt_calc(32, 0);
  std::vector< std::vector<double> > emitt_calc_timestep;

  int maxlevel = 1;

  particella inLettura, choosenDataPlus, choosenDataMinus;

  // **** Initializing particles to remove warnings ****
  inLettura.id_number = inLettura.lost = inLettura.id_type = 0;
  choosenDataPlus.id_number = choosenDataPlus.lost = choosenDataPlus.id_type = 0;
  choosenDataMinus.id_number = choosenDataMinus.lost = choosenDataMinus.id_type = 0;
  // ***************************************************

  char skip[9];

  double bestFoundPlus = 0.0, bestFoundMinus = 0.0;
  double vx = 0., vy = 0., vz = 0.;
  double xvx_sum = 0., yvy_sum = 0., zvz_sum = 0.;
  double xvx_mean = 0., yvy_mean = 0., zvz_mean = 0.;
  double mixx = 0., mixy = 0., mixz = 0.;
  double x_sum = 0., x2_sum = 0., x_mean = 0., x2_mean = 0.,
    px_sum = 0., px2_sum = 0., px_mean = 0., px2_mean = 0., sigma_x = 0., sigma_px = 0., epsilon_x = 0.;
  double y_sum = 0., y2_sum = 0., y_mean = 0., y2_mean = 0.,
    py_sum = 0., py2_sum = 0., py_mean = 0., py2_mean = 0., sigma_y = 0., sigma_py = 0., epsilon_y = 0.;
  double z_sum = 0., z2_sum = 0., z_mean = 0., z2_mean = 0.,
    pz_sum = 0., pz2_sum = 0., pz_mean = 0., pz2_mean = 0., sigma_z = 0., sigma_pz = 0., epsilon_z = 0.;
  double gamma;

  int counter = 0, counterTotal = 0;
  int somethingDone = 0;
  int column;
  double temporanei[3];

  switch (inputArgv3)
  {
  case -1:
    if (argc != 5)
    {
      std::cout << "You asked to look for maximum without telling me on which column!" << std::endl;
      return 254;
    }
    column = atoi(argv[4]);
    if (column > 5 && column < 0)
    {
      std::cout << "You asked to look for maximum on a wrong column!" << std::endl;
      return 253;
    }
    std::cout << "Extracting maximum value from " << argv[1] << ", column #" << column << std::endl;

    while (1)
    {
      inputFile >> inLettura.v[0] >> inLettura.v[1] >> inLettura.v[2] >> inLettura.v[3] >> inLettura.v[4] >> inLettura.v[5]
        >> temporanei[0] >> inLettura.weight >> temporanei[1] >> temporanei[2];

      if (inputFile.eof()) break;

      inLettura.id_type = (int)temporanei[0];
      inLettura.lost = (int)temporanei[1];
      inLettura.id_number = (int)temporanei[2];
      if (inLettura.v[column] > bestFoundPlus && inLettura.lost == 0)
      {
        bestFoundPlus = inLettura.v[column];
        choosenDataPlus.v[0] = inLettura.v[0];
        choosenDataPlus.v[1] = inLettura.v[1];
        choosenDataPlus.v[2] = inLettura.v[2];
        choosenDataPlus.v[3] = inLettura.v[3];
        choosenDataPlus.v[4] = inLettura.v[4];
        choosenDataPlus.v[5] = inLettura.v[5];
        choosenDataPlus.id_type = inLettura.id_type;
        choosenDataPlus.weight = inLettura.weight;
        choosenDataPlus.lost = inLettura.lost;
        choosenDataPlus.id_number = inLettura.id_number;
      }
      else if (inLettura.v[column] < bestFoundMinus && inLettura.lost == 0)
      {
        bestFoundMinus = inLettura.v[column];
        choosenDataMinus.v[0] = inLettura.v[0];
        choosenDataMinus.v[1] = inLettura.v[1];
        choosenDataMinus.v[2] = inLettura.v[2];
        choosenDataMinus.v[3] = inLettura.v[3];
        choosenDataMinus.v[4] = inLettura.v[4];
        choosenDataMinus.v[5] = inLettura.v[5];
        choosenDataMinus.id_type = inLettura.id_type;
        choosenDataMinus.weight = inLettura.weight;
        choosenDataMinus.lost = inLettura.lost;
        choosenDataMinus.id_number = inLettura.id_number;
      }
    }
    somethingDone = 1;
    break;

  case -2:
    if (argc != 5)
    {
      std::cout << "You asked to look for minimum without telling me on which column!" << std::endl;
      return 254;
    }
    column = atoi(argv[4]);
    if (column > 5 && column < 0)
    {
      std::cout << "You asked to look for minimum on a wrong column!" << std::endl;
      return 253;
    }
    std::cout << "Extracting minimum value from " << argv[1] << ", column #" << column << std::endl;
    bestFoundPlus = 5.0;
    bestFoundMinus = -5.0;
    while (1)
    {
      inputFile >> inLettura.v[0] >> inLettura.v[1] >> inLettura.v[2] >> inLettura.v[3] >> inLettura.v[4] >> inLettura.v[5]
        >> temporanei[0] >> inLettura.weight >> temporanei[1] >> temporanei[2];

      if (inputFile.eof()) break;

      inLettura.id_type = (int)temporanei[0];
      inLettura.lost = (int)temporanei[1];
      inLettura.id_number = (int)temporanei[2];

      if (inLettura.v[column] < bestFoundPlus && inLettura.v[column] >= 0. && inLettura.lost == 0)
      {
        bestFoundPlus = inLettura.v[column];
        choosenDataPlus.v[0] = inLettura.v[0];
        choosenDataPlus.v[1] = inLettura.v[1];
        choosenDataPlus.v[2] = inLettura.v[2];
        choosenDataPlus.v[3] = inLettura.v[3];
        choosenDataPlus.v[4] = inLettura.v[4];
        choosenDataPlus.v[5] = inLettura.v[5];
        choosenDataPlus.id_type = inLettura.id_type;
        choosenDataPlus.weight = inLettura.weight;
        choosenDataPlus.lost = inLettura.lost;
        choosenDataPlus.id_number = inLettura.id_number;
      }
      else if (inLettura.v[column] > bestFoundMinus && inLettura.v[column] < 0. && inLettura.lost == 0)
      {
        bestFoundMinus = inLettura.v[column];
        choosenDataMinus.v[0] = inLettura.v[0];
        choosenDataMinus.v[1] = inLettura.v[1];
        choosenDataMinus.v[2] = inLettura.v[2];
        choosenDataMinus.v[3] = inLettura.v[3];
        choosenDataMinus.v[4] = inLettura.v[4];
        choosenDataMinus.v[5] = inLettura.v[5];
        choosenDataMinus.id_type = inLettura.id_type;
        choosenDataMinus.weight = inLettura.weight;
        choosenDataMinus.lost = inLettura.lost;
        choosenDataMinus.id_number = inLettura.id_number;
      }
    }
    somethingDone = 1;
    break;


  case -3:
    if (argc != 5)
    {
      std::cout << "You asked to calculate emittances without telling me the time step!" << std::endl;
      return 254;
    }
    timestep = atof(argv[4]);
    timestep *= Numerostep;

    std::cout << "I will calculate emittances for file " << argv[1] << std::endl;
    while (1)
    {
      inputFile >> inLettura.v[0] >> inLettura.v[1] >> inLettura.v[2] >> inLettura.v[3] >> inLettura.v[4] >> inLettura.v[5]
        >> temporanei[0] >> inLettura.weight >> temporanei[1] >> temporanei[2];

      if (inputFile.eof()) break;

      inLettura.id_type = (int)temporanei[0];
      inLettura.lost = (int)temporanei[1];
      inLettura.id_number = (int)temporanei[2];

      counterTotal++;

      if (!inLettura.lost)
      {
        counter++;

        x_sum += inLettura.v[0];
        x2_sum += inLettura.v[0] * inLettura.v[0];
        vx = inLettura.v[3];

        y_sum += inLettura.v[1];
        y2_sum += inLettura.v[1] * inLettura.v[1];
        vy = inLettura.v[4];

        z_sum += inLettura.v[2];
        z2_sum += inLettura.v[2] * inLettura.v[2];
        vz = inLettura.v[5];

        gamma = sqrt(1.0 + vx*vx + vy*vy + vz*vz);
        vx /= gamma;
        vy /= gamma;
        vz /= gamma;

        px_sum += vx;
        px2_sum += (vx*vx);
        py_sum += vy;
        py2_sum += (vy*vy);
        pz_sum += vz;
        pz2_sum += (vz*vz);
        xvx_sum += inLettura.v[0] * vx;
        yvy_sum += inLettura.v[1] * vy;
        zvz_sum += inLettura.v[2] * vz;
      }
    }
    x_mean = x_sum / ((double)counter);
    x2_mean = x2_sum / ((double)counter);
    px_mean = px_sum / ((double)counter);
    px2_mean = px2_sum / ((double)counter);
    xvx_mean = xvx_sum / ((double)counter);

    y_mean = y_sum / ((double)counter);
    y2_mean = y2_sum / ((double)counter);
    py_mean = py_sum / ((double)counter);
    py2_mean = py2_sum / ((double)counter);
    yvy_mean = yvy_sum / ((double)counter);

    z_mean = z_sum / ((double)counter);
    z2_mean = z2_sum / ((double)counter);
    pz_mean = pz_sum / ((double)counter);
    pz2_mean = pz2_sum / ((double)counter);
    zvz_mean = zvz_sum / ((double)counter);

    sigma_x = x2_mean - x_mean*x_mean;
    sigma_y = y2_mean - y_mean*y_mean;
    sigma_z = z2_mean - z_mean*z_mean;
    sigma_px = px2_mean - px_mean * px_mean;
    sigma_py = py2_mean - py_mean * py_mean;
    sigma_pz = pz2_mean - pz_mean * pz_mean;
    mixx = xvx_mean - x_mean * px_mean;
    mixy = yvy_mean - y_mean * py_mean;
    mixz = zvz_mean - z_mean * pz_mean;
    epsilon_x = sigma_x * sigma_px - mixx * mixx;
    epsilon_y = sigma_y * sigma_py - mixy * mixy;
    epsilon_z = sigma_z * sigma_pz - mixz * mixz;

    if (epsilon_x < 0) epsilon_x = -epsilon_x;
    if (epsilon_y < 0) epsilon_y = -epsilon_y;
    if (epsilon_z < 0) epsilon_z = -epsilon_z;
    epsilon_x = sqrt(epsilon_x);
    epsilon_y = sqrt(epsilon_y);
    epsilon_z = sqrt(epsilon_z);

    if (sigma_x < 0) sigma_x = -sigma_x;
    if (sigma_y < 0) sigma_y = -sigma_y;
    if (sigma_z < 0) sigma_z = -sigma_z;
    sigma_x = sqrt(sigma_x);
    sigma_y = sqrt(sigma_y);
    sigma_z = sqrt(sigma_z);

    if (contacolonne == 0) z_mean = epsilon_x = epsilon_y = epsilon_z = sigma_x = sigma_y = sigma_z = 0;

    somethingDone = 2;

    break;


  case -4:
    std::cout << "I will re-calculate emittances using output from file " << argv[1] << "\nwhich should be generated by parallel_propaga" << std::endl;
    inputFile >> Numerostep;
    inputFile.seekg(0, std::ios::beg);

    while (1)
    {
      //DEFINIZIONE FILE emitt.ppg direttamente da Propaga
      //  emitt_file << j << " : " << massime << "\t" << (lattice->dimmi_quante_sono_vive()) << "\t"
      //  << weightTot << "\t" << x_sum << "\t" << x2_sum << "\t" << y_sum << "\t" << y2_sum << "\t" << z_sum << "\t" << z2_sum << "\t"
      //  << px_sum << "\t" << px2_sum << "\t" << py_sum << "\t" << py2_sum << "\t " << pz_sum << "\t" << pz2_sum << "\t"
      //  << xvx_sum << "\t" << yvy_sum << "\t" << zvz_sum << std::endl;
      inputFile >> emitt_param[0] >> skip >> emitt_param[1] >> emitt_param[17] >> emitt_param[18] >> emitt_param[2] >> emitt_param[3] >> emitt_param[4] >> emitt_param[5] >> emitt_param[6] >> emitt_param[7]
        >> emitt_param[8] >> emitt_param[9] >> emitt_param[10] >> emitt_param[11] >> emitt_param[12] >> emitt_param[13] >> emitt_param[14] >> emitt_param[15] >> emitt_param[16];

      if (inputFile.eof()) break;

      column = (int)floor((emitt_param[0] / Numerostep) + 0.5);

      if (column <= maxlevel)
      {
        for (int jj = 1; jj < NUMERO_PARAMETRI; jj++) emitt_timestep[column][jj] += emitt_param[jj];
      }
      else
      {
        maxlevel++;
        emitt_timestep.push_back(emitt_param);
      }
    }
    for (int ii = 1; ii <= maxlevel; ii++)
    {
      if (emitt_timestep[ii][1] <= 0 && emitt_timestep[ii][1] >= 0)  // floating-point equality and inequality comparisons are unreliable, so this is the best way
      {
        std::cout << "step " << ii << ", counter = " << emitt_timestep[ii][1] << std::endl;
        continue;
      }

      emitt_calc[1] = emitt_timestep[ii][3] / emitt_timestep[ii][18];   // x2_mean  = x2_sum  / weightTot;
      emitt_calc[3] = emitt_timestep[ii][9] / emitt_timestep[ii][18];   // px2_mean = px2_sum / weightTot;
      emitt_calc[4] = emitt_timestep[ii][14] / emitt_timestep[ii][18];    // xvx_mean = xvx_sum / weightTot;

      emitt_calc[6] = emitt_timestep[ii][5] / emitt_timestep[ii][18];   // y2_mean  = y2_sum  / weightTot;
      emitt_calc[8] = emitt_timestep[ii][11] / emitt_timestep[ii][18];    // py2_mean = py2_sum / weightTot;
      emitt_calc[9] = emitt_timestep[ii][15] / emitt_timestep[ii][18];    // yvy_mean = yvy_sum / weightTot;

      emitt_calc[10] = emitt_timestep[ii][6] / emitt_timestep[ii][18];    // z_mean   = z_sum   / weightTot;
      emitt_calc[11] = emitt_timestep[ii][7] / emitt_timestep[ii][18];    // z2_mean  = z2_sum  / weightTot;
      emitt_calc[13] = emitt_timestep[ii][13] / emitt_timestep[ii][18];   // pz2_mean = pz2_sum / weightTot;
      emitt_calc[14] = emitt_timestep[ii][16] / emitt_timestep[ii][18];   // zvz_mean = zvz_sum / weightTot;

      emitt_calc[15] = sqrt(emitt_calc[1]);
      //      emitt_calc[15] = emitt_calc[1]  - emitt_calc[0] *emitt_calc[0];     // sigma_x   = x2_mean - x_mean*x_mean;
      emitt_calc[16] = sqrt(emitt_calc[6]);
      //      emitt_calc[16] = emitt_calc[6]  - emitt_calc[5] *emitt_calc[5];     // sigma_y   = y2_mean - y_mean*y_mean;
      emitt_calc[17] = sqrt(emitt_calc[11]);
      //      emitt_calc[17] = emitt_calc[11] - emitt_calc[10]*emitt_calc[10];    // sigma_z   = z2_mean - z_mean*z_mean;
      emitt_calc[18] = sqrt(emitt_calc[3]);
      //      emitt_calc[18] = emitt_calc[3]  - emitt_calc[2] *emitt_calc[2];     // sigma_px  = px2_mean - px_mean * px_mean;
      emitt_calc[19] = sqrt(emitt_calc[8]);
      //      emitt_calc[19] = emitt_calc[8]  - emitt_calc[7] *emitt_calc[7];     // sigma_py  = py2_mean - py_mean * py_mean;
      emitt_calc[20] = sqrt(emitt_calc[13]);
      //      emitt_calc[20] = emitt_calc[13] - emitt_calc[12]*emitt_calc[12];    // sigma_pz  = pz2_mean - pz_mean * pz_mean;

      emitt_calc[27] = emitt_timestep[ii][1];                 // double counter (# particelle totali)
      emitt_calc[28] = emitt_timestep[ii][17];                // double vive (# particelle vive)
      emitt_calc[29] = emitt_timestep[ii][18];                // double weightTot (peso totale particelle vive)

      // calcolo emittanze
      //      epsilon_x = x2_mean * px2_mean - xvx_mean*xvx_mean;
      //      epsilon_y = y2_mean * py2_mean - yvy_mean*yvy_mean;
      //      epsilon_z = z2_mean * pz2_mean - zvz_mean*zvz_mean;
      emitt_calc[24] = emitt_calc[1] * emitt_calc[3] - emitt_calc[4] * emitt_calc[4];
      emitt_calc[25] = emitt_calc[6] * emitt_calc[8] - emitt_calc[9] * emitt_calc[9];
      emitt_calc[26] = emitt_calc[11] * emitt_calc[13] - emitt_calc[14] * emitt_calc[14];
      if (emitt_calc[24] > 0.0) emitt_calc[24] = sqrt(emitt_calc[24]); else emitt_calc[24] = 0.0;
      if (emitt_calc[25] > 0.0) emitt_calc[25] = sqrt(emitt_calc[25]); else emitt_calc[25] = 0.0;
      if (emitt_calc[26] > 0.0) emitt_calc[26] = sqrt(emitt_calc[26]); else emitt_calc[26] = 0.0;

      emitt_calc_timestep.push_back(emitt_calc);
    }
    somethingDone = 4;
    break;


  default:
    std::cout << "I will output the #" << inputArgv3 << " particle from " << argv[1] << ", without considering its ordinal number!" << std::endl;
    while (1)
    {
      inputFile >> inLettura.v[0] >> inLettura.v[1] >> inLettura.v[2] >> inLettura.v[3] >> inLettura.v[4] >> inLettura.v[5]
        >> temporanei[0] >> inLettura.weight >> temporanei[1] >> temporanei[2];

      if (inputFile.eof() && somethingDone != 1)
      {
        std::cout << "Unable to find particle #" << inputArgv3 << std::endl;
        break;
      }
      inLettura.id_type = (int)temporanei[0];
      inLettura.lost = (int)temporanei[1];
      inLettura.id_number = (int)temporanei[2];
      counter++;
      if (counter == inputArgv3)
      {
        choosenDataPlus.v[0] = inLettura.v[0];
        choosenDataPlus.v[1] = inLettura.v[1];
        choosenDataPlus.v[2] = inLettura.v[2];
        choosenDataPlus.v[3] = inLettura.v[3];
        choosenDataPlus.v[4] = inLettura.v[4];
        choosenDataPlus.v[5] = inLettura.v[5];
        choosenDataPlus.id_type = inLettura.id_type;
        choosenDataPlus.weight = inLettura.weight;
        choosenDataPlus.lost = inLettura.lost;
        choosenDataPlus.id_number = inLettura.id_number;
        break;
      }
    }
    somethingDone = 3;
    break;
  }

  if (somethingDone == 1)
  {
    if (argc == 5)
    {
      selectedData << std::setprecision(7) << std::setiosflags(std::ios::scientific)
        << choosenDataPlus.v[0] << "\t" << choosenDataPlus.v[1] << "\t" << choosenDataPlus.v[2] << "\t"
        << choosenDataPlus.v[3] << "\t" << choosenDataPlus.v[4] << "\t" << choosenDataPlus.v[5]
        << std::endl;
      selectedData << std::setprecision(7) << std::setiosflags(std::ios::scientific)
        << choosenDataMinus.v[0] << "\t" << choosenDataMinus.v[1] << "\t" << choosenDataMinus.v[2] << "\t"
        << choosenDataMinus.v[3] << "\t" << choosenDataMinus.v[4] << "\t" << choosenDataMinus.v[5]
        << std::endl;
    }
    else std::cout << "Something went wrong." << std::endl;
  }

  else if (somethingDone == 2)
  {
    //selectedData << "Nome file\tNumero particelle\tNumero step\tct (cm)\tmean z (cm)\tsigma x\tepsilon x\tsigma y\tepsilon y\tsigma z\tepsilon z" << std::endl;
    selectedData << argv[1] << ":\t" << counter << "\t" << counterTotal << "\t" << Numerostep << "\t" << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << timestep << "\t" << z_mean << "\t" << sigma_x << "\t" << epsilon_x << "\t" << sigma_y << "\t" << epsilon_y << "\t" << sigma_z << "\t"
      << epsilon_z << std::endl;
  }

  else if (somethingDone == 4)
  {
    for (int kk = 0; kk < (int)emitt_calc_timestep.size(); kk++)
    {
      /*
      selectedData
      << Numero step                  //  1^ colonna
      << ":"                          //  2^ colonna
      << Numero particelle tot        //  3^ colonna
      << Numero particelle vive       //  4^ colonna
      << Peso totale particelle vive  //  5^ colonna
      << mean z (cm)                  //  6^ colonna
      << envelope x                   //  7^ colonna
      << epsilon x                    //  8^ colonna
      << envelope y                   //  9^ colonna
      << epsilon y                    // 10^ colonna
      << envelope z                   // 11^ colonna
      << epsilon z                    // 12^ colonna
      */
      selectedData << (kk + 1)*Numerostep << "\t:\t" << (int)emitt_calc_timestep[kk][27] << "\t" << (int)emitt_calc_timestep[kk][28] << "\t"
        << std::setprecision(7) << std::setiosflags(std::ios::scientific) << emitt_calc_timestep[kk][29] << "\t"
        << emitt_calc_timestep[kk][10] << "\t" << emitt_calc_timestep[kk][15] << "\t" << emitt_calc_timestep[kk][24] << "\t" << emitt_calc_timestep[kk][16]
        << "\t" << emitt_calc_timestep[kk][25] << "\t" << emitt_calc_timestep[kk][17] << "\t" << emitt_calc_timestep[kk][26] << std::endl;
    }
  }
  else if (somethingDone == 3)
  {
    selectedData << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << choosenDataPlus.v[0] << "\t" << choosenDataPlus.v[1] << "\t" << choosenDataPlus.v[2] << "\t"
      << choosenDataPlus.v[3] << "\t" << choosenDataPlus.v[4] << "\t" << choosenDataPlus.v[5] << "\t"
      << choosenDataPlus.id_type << "\t" << choosenDataPlus.weight << "\t" << choosenDataPlus.lost << "\t" << choosenDataPlus.id_number << std::endl;
  }

  else
  {
    std::cout << "Nothing could be done on file " << argv[1] << std::endl;
  }

  inputFile.close();
  selectedData.close();
  return 0;
}


