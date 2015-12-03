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


#include "seleziona.h"

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#pragma warning(disable : 869)
#pragma warning(disable : 981)
#endif

struct particella
{
  double x, y, z, px, py, pz;
  double tipo_particella, weight, flag_assorbimento, ordinale;
};


bool doubleEquality(double a, double b)
{
  return fabs(a - b) < EPSILON;
}


void seleziona(char* file_da_leggere, char* file_da_selezionare, char* file_da_scrivere)
{
  std::ifstream da_leggere, da_selezionare;
  std::ofstream da_scrivere;
  bool fallita_lettura_da_leggere = true, fallita_lettura_da_selezionare = true, fallita_apertura_da_scrivere = true;

  da_leggere.open(file_da_leggere);
  da_selezionare.open(file_da_selezionare);
  fallita_lettura_da_leggere = da_leggere.fail();
  fallita_lettura_da_selezionare = da_selezionare.fail();
  std::cout << "File 1: " << std::string(file_da_leggere) << std::endl;
  std::cout << "File 2: " << std::string(file_da_selezionare) << std::endl;

  if ((fallita_lettura_da_leggere || fallita_lettura_da_selezionare))
  {
    std::cout << "Unable to work on your files" << std::endl;
    return;
  }

  int contacolonne_da_leggere = 0, contacolonne_da_selezionare = 0, colonna_analizzata = 8; //nb: la colonna analizzata e' scritta in modo valido per usarlo nell'array, quella effettiva e' quindi la colonna_analizzata+1
  char str[DIM_MAX_LINEA];
  char * pch;
  da_leggere.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  da_leggere.clear();
  da_leggere.seekg(0, std::ios::beg);

  pch = std::strtok(str, " \t");
  if (pch != NULL) contacolonne_da_leggere++;
  while (pch != NULL)
  {
    pch = std::strtok(NULL, " \t");
    if (pch != NULL) contacolonne_da_leggere++;
  }
  da_selezionare.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  da_selezionare.clear();
  da_selezionare.seekg(0, std::ios::beg);

  pch = std::strtok(str, " \t");
  if (pch != NULL) contacolonne_da_selezionare++;
  while (pch != NULL)
  {
    pch = std::strtok(NULL, " \t");
    if (pch != NULL) contacolonne_da_selezionare++;
  }
  printf("Trovate %d colonne nel file da leggere e %d colonne nel file da selezionare\n", contacolonne_da_leggere, contacolonne_da_selezionare);
  printf("Considero la %d colonna come quella contenente il flag vivo-morto\n", colonna_analizzata + 1);

  if (contacolonne_da_leggere != 10 || contacolonne_da_selezionare != 10)
  {
    printf("Attenzione, il programma funziona solo con file a 10 colonne\n");
    return;
  }

  da_scrivere.open(file_da_scrivere);
  fallita_apertura_da_scrivere = da_scrivere.fail();
  std::cout << "File out: " << std::string(file_da_scrivere) << std::endl;

  if ((fallita_apertura_da_scrivere))
  {
    std::cout << "Unable to work on your files" << std::endl;
    return;
  }



  particella in_lettura;
  std::vector< particella > protoni_salvati;
  std::vector< double > protoni_da_salvare;



  while (1) {
    da_leggere >> in_lettura.x >> in_lettura.y >> in_lettura.z >> in_lettura.px >> in_lettura.py >> in_lettura.pz;
    da_leggere >> in_lettura.tipo_particella >> in_lettura.weight >> in_lettura.flag_assorbimento >> in_lettura.ordinale;

    if (da_leggere.eof()) break;
    if (doubleEquality(in_lettura.flag_assorbimento, ZERODOUBLE)) {
      protoni_da_salvare.push_back(in_lettura.ordinale);
    }
  }

  std::cout << "Trovate " << protoni_da_salvare.size() << " particelle da selezionare" << std::endl;
  
  // assumo che i protoni siano in ordine di numero ordinale, per risparmiare il ciclo for sulla ricerca disordinata
  int contatore_protoni_salvati = 0;

  while (1) {
    da_selezionare >> in_lettura.x >> in_lettura.y >> in_lettura.z >> in_lettura.px >> in_lettura.py >> in_lettura.pz;
    da_selezionare >> in_lettura.tipo_particella >> in_lettura.weight >> in_lettura.flag_assorbimento >> in_lettura.ordinale;

    if (da_selezionare.eof()) break;

    // metodo valido per file scritti con numeri ordinali non crescenti   
    for (std::vector<double>::size_type kk = 0; kk < protoni_da_salvare.size(); kk++)
    {
      if (doubleEquality(in_lettura.ordinale, protoni_da_salvare.at(kk))) {
        protoni_salvati.push_back(in_lettura);
        break;
      }
    }

    /*    // metodo valido solo per file scritti con numeri ordinali crescenti
        if (doubleEquality(in_lettura.ordinale, protoni_da_salvare[contatore_protoni_salvati]))
        {
        protoni_salvati.push_back(in_lettura);
        contatore_protoni_salvati++;
        }
        */
  }


  // siccome tanto ad Astra non servono le colonne aggiuntive (7-10), non vengono nemmeno create
  for (std::vector<double>::size_type i = 0; i < protoni_salvati.size(); i++)
  {
    da_scrivere << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << protoni_salvati[i].x << "\t" << protoni_salvati[i].y << "\t" << protoni_salvati[i].z
      << "\t" << protoni_salvati[i].px << "\t" << protoni_salvati[i].py << "\t" << protoni_salvati[i].pz
      << "\t" << (int)protoni_salvati[i].tipo_particella << "\t" << protoni_salvati[i].weight << "\t"
      << (int)protoni_salvati[i].flag_assorbimento << "\t" << (int)protoni_salvati[i].ordinale << std::endl;
  }


  da_leggere.close();
  da_selezionare.close();
  da_scrivere.close();

  std::cout << "Selezionati " << protoni_salvati.size() << " nel file " << file_da_selezionare << " secondo quanto descritto in " << file_da_leggere << std::endl;

}




int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    std::cout << "Funzionamento: a.out -in file_da_cui_leggere file_da_selezionare -out file_da_produrre" << std::endl;
    return -254;
  }

  int file_da_leggere = -1, file_da_selezionare = -1, file_da_scrivere = -1;

  for (int i = 1; i < argc; i++)  // * We will iterate over argv[] to get the parameters stored inside.
  {               // * Note that we're starting on 1 because we don't need to know the path of the program, which is stored in argv[0]
    if (std::string(argv[i]) == "-in")
    {
      file_da_leggere = (i + 1);
      file_da_selezionare = (i + 2);
      i += 2;         // so that we skip in the for cycle the parsing of the <da_leggere> file and the <da_selezionare> file.
    }
    else if (std::string(argv[i]) == "-out")
    {
      file_da_scrivere = i + 1;
      i++;              // so that we skip in the for cycle the parsing of the <da_scrivere> file.
    }
    else
    {
      std::cout << "Invalid argument: " << argv[i] << std::endl;
      return -243;
    }
  }

  if (file_da_leggere < 0 || file_da_scrivere < 0 || file_da_selezionare < 0)
  {
    printf("Qualcosa e' andato storto nella riga comando\n");
    return 200;
  }

  seleziona(argv[file_da_leggere], argv[file_da_selezionare], argv[file_da_scrivere]);

  return 0;
}

