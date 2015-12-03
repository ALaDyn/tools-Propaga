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


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <list>
#include <ctime>
#include <string>
#include <sstream>

// #define TRIGGER_COMPATTA 1000000
#define MINIMUM_TRIGGER 8192
// #define DEBUG


int main(int argc, char *argv[])
{
  std::ifstream infile;
  std::ofstream outfile;
  std::string nomefile_in, nomefile_out;
  std::ostringstream componi_nomefile;
  std::string line;

  std::srand((unsigned int)std::time(NULL));

#if defined (DEBUG)
  nomefile_in = "prova.txt";
  componi_nomefile << "scrambled_prova.txt";
#else
  if (argc < 2)
  {
    std::cout << "Si usa con scramble_lines nomefile.txt" << std::endl;
    return -1;
  }

  nomefile_in = argv[1];
  componi_nomefile << "scrambled_" << std::string(argv[1]);
#endif

  nomefile_out = componi_nomefile.str();

  std::cout << "Reading lines from " << nomefile_in.c_str() << " and rewriting scrambled in " << nomefile_out.c_str() << "," << std::endl;
  std::cout << "removing comment lines (those starting with a #) in the meanwhile" << std::endl;

  infile.open(nomefile_in.c_str(), std::ifstream::in);
  if (infile.fail())
  {
    std::cout << "Unable to open input file" << std::endl;
    return -2;
  }
  outfile.open(nomefile_out.c_str(), std::ifstream::out);

  std::vector <std::string> dati_originali;

  while (!infile.eof())
  {
    std::getline(infile, line);
    if (infile.eof()) break;
    if (line[0] != '#') dati_originali.push_back(line);
  }

  std::vector <std::string> dati_scrambled, dati_compattati;
  int contarighe = 0, contaestratti = 0, contatotale = 0;
  int numero_random;
  bool gia_estratto = true;
  contatotale = dati_originali.size();
  dati_scrambled.resize(contatotale);

  int TRIGGER_COMPATTA = dati_originali.size() / 2;

  std::vector <int> numero_random_archiviato(TRIGGER_COMPATTA, -1);
  do
  {
    do
    {
      numero_random = (int)(((float)std::rand() / (float)RAND_MAX) * (dati_originali.size()));
      for (int i = 0; i <= contaestratti; i++)
      {
        gia_estratto = (numero_random == numero_random_archiviato[i]);
        if (gia_estratto) break;
      }
    } while (gia_estratto);
#if defined (DEBUG)
    std::cout << numero_random << std::endl;
#endif
    numero_random_archiviato[contaestratti] = numero_random;
    dati_scrambled.at(contarighe) = dati_originali.at(numero_random);
    contaestratti++;
    contarighe++;

    if (!(contaestratti % TRIGGER_COMPATTA))
    {
      std::cout << "Scrambled " << contarighe << " lines!" << std::endl;
      int k = 0;
      dati_compattati.resize(dati_originali.size() - TRIGGER_COMPATTA);
      for (unsigned int j = 0; j < dati_originali.size(); j++)
      {
        for (int i = 0; i < contaestratti; i++)
        {
          gia_estratto = (j == numero_random_archiviato[i]);
          if (gia_estratto) break;
        }
        if (!gia_estratto)
        {
          dati_compattati.at(k) = dati_originali.at(j);
          k++;
        }
      }
      dati_originali = dati_compattati;
      //      dati_originali.resize(dati_compattati.size());
      //      for(std::vector<int>::size_type i = 0; i != dati_compattati.size(); i++) dati_originali.at(i) = dati_compattati.at(i);
      for (std::vector<int>::iterator it = numero_random_archiviato.begin(); it < numero_random_archiviato.end(); it++) *it = -1;
      dati_compattati.resize(0);
      contaestratti = 0;
      if (dati_originali.size() > MINIMUM_TRIGGER) TRIGGER_COMPATTA = dati_originali.size() / 2;
    }
  } while (contarighe < contatotale && dati_originali.size() > 0);

  for (unsigned int i = 0; i < dati_scrambled.size(); i++)
    outfile << dati_scrambled.at(i) << std::endl;

  infile.close();
  outfile.close();

  return 0;
}

