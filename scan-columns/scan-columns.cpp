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
#include <vector>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <string>
#include <cstring>
#include <iomanip>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define SEPARATORS       "; \t"
#define COMMENTS         "#"

bool Belongs_to(char, std::string);
std::vector<std::vector<double>> Convert_to_double_vector(std::vector<std::vector<std::string>>, std::string = "");
std::vector<std::vector<std::string>> Parse_file(std::string, std::string, std::string = "");
bool checkEqual(double, double);
void seleziona(char*, char*, size_t, double);




int main(int argc, char *argv[])
{
  int file_da_leggere = -1, file_da_scrivere = -1;
  size_t colonna_da_selezionare = -1;
  double valore_riferimento;

  if (argc < 8)
  {
    std::cout << "Run as: " << argv[0] << " -in input_file -out output_file -select column_number reference_value" << std::endl;
    return -254;
  }

  for (int i = 1; i < argc; i++)  // * We will iterate over argv[] to get the parameters stored inside.
  {                               // * Note that we're starting on 1 because we don't need to know the path of the program, which is stored in argv[0]
    if (std::string(argv[i]) == "-in")
    {
      file_da_leggere = (i + 1);
      i++;                        // so that we skip in the for cycle the parsing of the <da_leggere> file.
    }
    else if (std::string(argv[i]) == "-out")
    {
      file_da_scrivere = i + 1;
      i++;                        // so that we skip in the for cycle the parsing of the <da_scrivere> file.
    }
    else if (std::string(argv[i]) == "-select")
    {
      colonna_da_selezionare = (size_t) atoi(argv[i + 1]);
      valore_riferimento = atof(argv[i + 2]);
      i = i + 2;                  // so that we skip in the for cycle the parsing of the selection properties.
    }
    else
    {
      std::cout << "Invalid argument: " << argv[i] << std::endl;
      return -243;
    }
  }

  if (file_da_leggere < 1 || file_da_scrivere < 1 || colonna_da_selezionare < 1)
  {
    std::cerr << "Something went wrong in the command line" << std::endl;
    return 200;
  }

  seleziona(argv[file_da_leggere], argv[file_da_scrivere], colonna_da_selezionare, valore_riferimento);

  std::cout << "Done!" << std::endl;

  return 0;
}


void seleziona(char* file_da_leggere, char* file_da_scrivere, size_t colonna_analizzata, double valore_riferimento)
{
  bool fallita_apertura_da_scrivere = true;

  std::vector< std::vector<std::string> > parsed_file = Parse_file(file_da_leggere, SEPARATORS);
  std::vector< std::vector<double> > doubled_file = Convert_to_double_vector(parsed_file, COMMENTS);

  std::cout << "Considering " << colonna_analizzata << "th column as the one used to select data" << std::endl;

  if (doubled_file.front().size() <= colonna_analizzata)
  {
    std::cerr << "Not enough columns in the input file" << std::endl;
    return;
  }

  std::vector< std::vector< double > > righe_salvate;
  for (auto i : doubled_file) if (checkEqual(i[colonna_analizzata - 1], valore_riferimento)) righe_salvate.push_back(i);
  std::cout << righe_salvate.size() << " useful lines found" << std::endl;

  std::ofstream da_scrivere;
  da_scrivere.open(file_da_scrivere);
  fallita_apertura_da_scrivere = da_scrivere.fail();
  std::cout << "File out: " << std::string(file_da_scrivere) << std::endl;

  if ((fallita_apertura_da_scrivere))
  {
    std::cout << "Unable to open output file" << std::endl;
    return;
  }

  for (auto i : righe_salvate)
  {
    for (auto j : i) da_scrivere << j << "\t";
    da_scrivere << std::endl;
  }


  da_scrivere.close();
}


bool checkEqual(double a, double b) {
  return (fabs(a - b) < std::numeric_limits<double>::epsilon());
}


bool Belongs_to(char c, std::string s) {
  for (size_t i = 0; i < s.size(); i++) { if (c == s.at(i)) return true; }
  return false;
}


std::vector< std::vector<double> > Convert_to_double_vector(std::vector< std::vector<std::string> > parsed_file, std::string comment) {
  std::vector<double> doubled_line;
  std::vector< std::vector<double> > doubled_file;

  for (auto &i : parsed_file) {
    doubled_line.clear();
    doubled_line.resize(i.size());
    if (Belongs_to(i[0][0], comment)) continue;
    for (size_t j = 0; j < i.size(); j++) doubled_line[j] = atof(i[j].c_str());
    doubled_file.push_back(doubled_line);
  }
  return doubled_file;
}


std::vector< std::vector<std::string> > Parse_file(std::string file_name, std::string separators, std::string comment) {
  // Safe file opening
  std::ifstream file_to_parse(file_name, std::ios::in);
  if (!file_to_parse) {
    std::cout << "Cannot open " << file_name << ". Quitting..." << std::endl;
    exit(12);
  }
  // Internal variables
  std::string line;
  std::vector<std::string> tokens;
  std::vector< std::vector<std::string> > parsed;
  while (getline(file_to_parse, line)) {
    boost::algorithm::trim(line);  // remove leading/trailing spaces
    if (Belongs_to(line[0], comment) || !line.size()) continue;
    boost::algorithm::split(tokens, line, boost::algorithm::is_any_of(separators), boost::token_compress_off);
    //std::transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
    for (size_t i = 0; i < tokens.size(); i++) {  // remove inline comments
      if (Belongs_to(tokens[i][0], comment)) { tokens.erase(tokens.begin() + i, tokens.end()); }
    }
    if (tokens.size()) {
      parsed.push_back(tokens);
    }
    line.clear(); tokens.clear();
  }
  file_to_parse.close();
  return parsed;
}



