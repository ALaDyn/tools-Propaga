#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <list>
#include <ctime>
#include <string>
#include <sstream>


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
  componi_nomefile << "inverted_prova.txt";
#else
  if (argc < 2)
  {
    std::cout << "Si usa con invert_lines nomefile.txt" << std::endl;
    return -1;
  }

  nomefile_in = argv[1];
  componi_nomefile << "inverted_" << std::string(argv[1]);
#endif

  nomefile_out = componi_nomefile.str();

  std::cout << "Reading lines from " << nomefile_in.c_str() << " and rewriting inverted in " << nomefile_out.c_str() << "," << std::endl;
  std::cout << "removing comment lines (those starting with a #) in the meanwhile" << std::endl;

  infile.open(nomefile_in.c_str(), std::ifstream::in);
  if (infile.fail())
  {
    std::cout << "Unable to open input file" << std::endl;
    return -2;
  }
  outfile.open(nomefile_out.c_str(), std::ifstream::out);

  std::list <std::string> dati;

  while (!infile.eof())
  {
    std::getline(infile, line);
    if (infile.eof()) break;
    if (line[0] != '#') dati.push_back(line);
  }

  for (std::list<std::string>::reverse_iterator rit = dati.rbegin(); rit != dati.rend(); ++rit) outfile << *rit << "\n";

  infile.close();
  outfile.close();

  return 0;
}

