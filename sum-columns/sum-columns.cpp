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


#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <vector>


int main(int narg, char *args[])
{
  std::ifstream infile;
  std::ofstream outfile;
  std::string prefisso_nomefile;
  std::string line;
  std::ostringstream nomefile;
  int xmin = 0, xmax = 0, jump = 0, contarighe = 0;
  int contacolonne = 3;
  int riga = 0;
  double terza_colonna;

  if (narg != 9)
  {
    std::cerr << "si usa:  ./a.out -f <prefisso_file_da_leggere>  -min suffisso_primo_file -max suffisso_ultimo_file -jump salto_suffisso" << std::endl;
    std::cerr << "i files devono avere un nome strutturato del tipo nomerun_xx.ppg dove xx va dal min al max \ndefiniti precedentemente a step pari a jump" << std::endl;
    std::cerr << "nb: verra' creato un file di nome output.ppg" << std::endl;
    return -1;
  }

  for (int i = 1; i < narg; i++)
  {
    if (std::string(args[i]) == "-f")
    {
      prefisso_nomefile = args[i + 1];
      i++;
    }
    else if (std::string(args[i]) == "-min")
    {
      xmin = atoi(args[i + 1]);
      i++;
    }
    else if (std::string(args[i]) == "-max")
    {
      xmax = atoi(args[i + 1]);
      i++;
    }
    else if (std::string(args[i]) == "-jump")
    {
      jump = atoi(args[i + 1]);
      i++;
    }
    else
    {
      std::cerr << "Invalid argument: " << args[i] << std::endl;
      return -2;
    }
  }

  std::cout << "Reading files from " << prefisso_nomefile << "_" << xmin << ".ppg to " << prefisso_nomefile << "_" << xmax << ".ppg" << std::endl;
  // per prima cosa conto le righe contenute nel primo file. Il programma assume che tutti i file abbiano lo stesso numero di righe!
  nomefile.str("\0");
  nomefile.seekp(0, std::ios::beg);
  nomefile << prefisso_nomefile << "_" << xmin << ".ppg";
  std::cout << "Sto contando il numero di righe presenti in " << nomefile.str().c_str() << std::endl;
  infile.open(nomefile.str().c_str());
  while (infile.good())
  {
    std::getline(infile, line);
    if (!infile.good()) break;
    if (line[0] != '#') contarighe++;
  }
  infile.clear();
  infile.seekg(0, std::ios::beg);
  infile.close();

  std::cout << "Trovate " << contarighe << " righe non di commenti (#).\nVerra` creato un file che somma la terza colonna di tutti questi files." << std::endl;

  double ** dati;
  dati = new double*[contarighe];
  for (int i = 0; i < contarighe; i++) {
    dati[i] = new double[contacolonne];
    for (int j = 0; j < contacolonne; j++) dati[i][j] = 0.0;
  }

  for (int i = xmin; i <= xmax; i += jump)
  {
    nomefile.str("\0");
    nomefile.seekp(0, std::ios::beg);

    nomefile << prefisso_nomefile << "_" << i << ".ppg";
    infile.open(nomefile.str().c_str());
    riga = 0;

    if (infile.fail())
    {
      std::cerr << "Unable to open file " << nomefile.str().c_str() << std::endl;
      return -200;
    }
    std::cout << "Inizio lettura file " << nomefile.str().c_str() << std::endl;

    while (std::getline(infile, line))
    {
      // line.assign(line.substr(0,line.find('#')));
      if (line[0] != '#')
      {
        std::istringstream iss(line);

        iss >> dati[riga][0];
        iss >> dati[riga][1];
        iss >> terza_colonna;
        dati[riga][2] += terza_colonna;

        riga++;
      }
      if (riga > contarighe) {
        std::cerr << "Warning, too many lines in file " << nomefile.str().c_str() << std::endl;
        break;
      }
    }

    if (riga < contarighe) {
      std::cerr << "Warning, too few lines in file " << nomefile.str().c_str() << std::endl;
      break;
    }

    infile.close();
  }

  outfile.open("output.ppg");
  for (int j = 0; j < contarighe; j++)
  {
    outfile << dati[j][0] << "\t" << dati[j][1] << "\t" << dati[j][2] << std::endl;
  }
  outfile.close();

  return 0;
}

