/******************************************************************************
Copyright 2010, 2011, 2012, 2013, 2014, 2015 Stefano Sinigardi
The program is distributed under the terms of the GNU General Public License
******************************************************************************/

/**************************************************************************
This file is part of tools-Propaga.

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



#include "converter.h"

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#pragma warning(disable : 869)
#pragma warning(disable : 981)
#endif



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
} data;


bool doubleEquality(double a, double b)
{
  return fabs(a - b) < EPSILON;
}


void preproc_from_micron_to_cm(char * fileIN, char* fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);


  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    // positions in input are expressed in micrometers and we convert it in centimeters
    dati_particella.at(0) *= 1.0e-4;
    dati_particella.at(1) *= 1.0e-4;
    dati_particella.at(2) *= 1.0e-4;

    particelle_file.push_back(dati_particella);
    contaparticelle++;
  }

  for (int i = 0; i < contaparticelle; i++)
  {
    for (int j = 0; j < 6; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }


  in.close();
  out.close();

  std::cout << "\nConvertite le colonne x, y, z da micrometri a centimetri" << std::endl;
}


void convert_from_jasmine(char * fileIN, char* fileOUT)
{
  // assumo, tanto comunque non penso mi sara' mai utile in dimensionalita' piu' basse, che il file di output di jasmine sia a 9 colonne
  std::ifstream in(fileIN, std::ios::binary);
  std::ofstream out(fileOUT);

  int ncolIN = 9;
  int64_t length;
  in.seekg(0, std::ios::end);
  length = in.tellg();
  in.seekg(0, std::ios::beg);

  int numero_particelle = (int)(length / (ncolIN*sizeof(double)));

  if (numero_particelle == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector <std::vector <double> > particelle_file(numero_particelle, std::vector<double>(ncolIN + 5, 0));
  double temporary_buffer = 0.0;

  //  std::cout << "Inizializzato un vettore da " << numero_particelle << " righe e  " << ncolIN << " colonne." << std::endl;

  for (int i = 0; i < ncolIN; i++)
  {
    for (int j = 0; j < numero_particelle; j++)
    {
      in.read(reinterpret_cast<char*>(&temporary_buffer), sizeof temporary_buffer);
      particelle_file.at(j).at(i) = temporary_buffer;
    }
    if (in.eof()) break;
  }

  // colonne standard di jasmine: 
  //    1-2-3: posizioni in cm
  //    4-5-6: momenti normalizzati (gamma*beta)
  //    7: massa in g
  //    8: carica in statCoulomb
  //    9: 1/gamma
  for (int j = 0; j < numero_particelle; j++)
  {
    if (particelle_file.at(j).at(7) > 0)
    {
      particelle_file.at(j).at(ncolIN) = particelle_file.at(j).at(7) / Q_sC;
      particelle_file.at(j).at(ncolIN + 1) = particelle_file.at(j).at(6) / MP_G;
      particelle_file.at(j).at(ncolIN + 2) = 1;
      particelle_file.at(j).at(ncolIN + 3) = 0;
      particelle_file.at(j).at(ncolIN + 4) = j;
    }
    if (particelle_file.at(j).at(7) < 0)
    {
      particelle_file.at(j).at(ncolIN) = -particelle_file.at(j).at(7) / Q_sC;
      particelle_file.at(j).at(ncolIN + 1) = particelle_file.at(j).at(6) / ME_G;
      particelle_file.at(j).at(ncolIN + 2) = 3;
      particelle_file.at(j).at(ncolIN + 3) = 0;
      particelle_file.at(j).at(ncolIN + 4) = j;
    }
  }


  for (int j = 0; j < numero_particelle; j++)
  {
    for (int i = 0; i < 6; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(j).at(i) << "\t";
    for (int i = ncolIN + 2; i < ncolIN + 5; i++) out << (int)particelle_file.at(j).at(i) << "\t";
    for (int i = 6; i < ncolIN + 2; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(j).at(i) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nConvertito il file da jasmine (binario) a propaga (ascii)" << std::endl;
}


void preproc_from_OldP_to_NewP(char *fileIN, char *fileOUT, int particleId, double weight, int colbin)
{
  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(6, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  char str[DIM_MAX_LINEA];
  in.getline(str, DIM_MAX_LINEA);

  if (in.fail())
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere se c'è qualcosa nel file
  in.clear();
  in.seekg(0, std::ios::beg);

  while (1)
  {
    if (colbin == 0)  // non bisogna invertire x e z
    {
      in >> dati_particella.at(0) >> dati_particella.at(1) >> dati_particella.at(2)
        >> dati_particella.at(3) >> dati_particella.at(4) >> dati_particella.at(5);
    }
    else
    {
      in >> dati_particella.at(2) >> dati_particella.at(0) >> dati_particella.at(1)
        >> dati_particella.at(5) >> dati_particella.at(3) >> dati_particella.at(4);
    }
    if (in.eof()) break;

    dati_particella[0] *= 1.e-4;    // l'input avviene in cm, non piu' in micrometri
    dati_particella[1] *= 1.e-4;
    dati_particella[2] *= 1.e-4;
    particelle_file.push_back(dati_particella);
    dimensione_file++;  //utile per dei check se sta crescendo troppo

#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }


  for (int i = 0; i < dimensione_file; i++)
  {
    out << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << particelle_file[i][0] << "\t" << particelle_file[i][1] << "\t" << particelle_file[i][2] << "\t"
      << particelle_file[i][3] << "\t" << particelle_file[i][4] << "\t" << particelle_file[i][5] << "\t"
      << particleId << "\t" << weight << "\t" << "0" << "\t"
      << i + 1 << std::endl;
  }



  in.close();
  out.close();

  std::cout << "\nConvertite " << dimensione_file << " particelle dal vecchio al nuovo formato di Propaga" << std::endl;
}


void preproc_from_NewP_to_OldP(char * fileIN, char* fileOUT)
{
  // in realta' non e' solo da un file a 9 colonne, ma da un file con qualunque numero > 6 di colonne ad un file che ne tiene le prime 6
  // ed inoltre moltiplica le prime 3 colonne per 10^4 (da cm a micrometri)
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);


  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    dati_particella.at(0) *= DA_CM_A_MICRON;      // files with 6 columns historically have positions in micrometers, not centimeters
    dati_particella.at(1) *= DA_CM_A_MICRON;
    dati_particella.at(2) *= DA_CM_A_MICRON;

    if (in.eof()) break;

    particelle_file.push_back(dati_particella);
    contaparticelle++;
  }

  for (int i = 0; i < contaparticelle; i++)
  {
    for (int j = 0; j < 6; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }
  in.close();
  out.close();

  std::cout << "\nConvertito il file e preservate solo le prime 6 colonne" << std::endl;
}


void convert_from_propaga_to_binary_dst(char *fileIN, char *fileOUT, double C, double freq, double curr)
{
  std::ifstream in(fileIN, std::ios::in);
  std::ofstream out(fileOUT, std::ofstream::binary | std::ios::out);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (contacolonne < 6)
  {
    printf("Unable to proceed, missing required columns\n");
    in.close();
    out.close();
    return;
  }

  printf("Assuming x, y and z in cm  on columns 1-2-3\n");
  printf("Assuming normalized px, py and pz on columns 4-5-6\n");

  int ncol_dst = 6;
  std::vector<double> dati_particella(ncol_dst, 0);
  std::vector<double> dati_letti(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;
  std::vector<char> support_char(2, 0);
  double wavelength = C / freq;

  while (true)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_letti.at(i);

    if (in.eof()) break;

    // dst particle structure: x(cm),x'(rad),y(cm),y'(rad),phi(rad),Energie(MeV)

    // positions along x and y must be in cm, so they are already ok!
    // px and py must be x' and y' (rad), so we should be ready!
    dati_particella[0] = dati_letti[0];
    dati_particella[1] = dati_letti[3];
    dati_particella[2] = dati_letti[1];
    dati_particella[3] = dati_letti[4];

    // position along z is converted to a phase in rad
    dati_particella[4] = dati_letti[2] * 2.0 * M_PI / (wavelength * dati_letti[5]);

    // pz is converted to particle energy in MeV
    double relativistic_gamma = (sqrt(1.0 + dati_letti[3] * dati_letti[3] + dati_letti[4] * dati_letti[4] + dati_letti[5] * dati_letti[5]));
    dati_particella[5] = (DA_ERG_A_MEV)* (relativistic_gamma - 1.0) * MP_G * C * C;
    particelle_file.push_back(dati_particella);
  }

  int number_of_particles = particelle_file.size();
  double mc2 = MP_MEV; // *C*C ??

  
  //A.dst file use a binary format.It contains information of a beam at a given longitudinal position : number of particles, beam current, repetition frequency and rest mass as well as the 6D particles coordinates.The format is the following :
  //2xCHAR + INT(Np) + DOUBLE(Ib(mA)) + DOUBLE(freq(MHz)) + CHAR +
  //Np×[6×DOUBLE(x(cm), x'(rad),y(cm),y'(rad), phi(rad), Energie(MeV))] +
  //DOUBLE(mc2(MeV))
  //Comments :
  //- CHAR is 1 byte long,
  //- INT is 4 bytes long,
  //- DOUBLE is 8 bytes long.
  //- Np is the number of particles,
  //- Ib is the beam current,
  //- freq is the bunch frequency,
  //- mc2 is the particle rest mass.

  out.write(reinterpret_cast<const char*>(&(support_char[0])), sizeof(support_char) * sizeof(char));
  out.write(reinterpret_cast<const char*>(&number_of_particles), sizeof(number_of_particles) * sizeof(int));
  out.write(reinterpret_cast<const char*>(&freq), sizeof(double));
  out.write(reinterpret_cast<const char*>(&curr), sizeof(double));
  out.write(reinterpret_cast<const char*>(&(support_char[0])), 1 * sizeof(char));
  //std::copy(particelle_file.begin(), particelle_file.end(), std::ostreambuf_iterator<char>(out));
  out.write(reinterpret_cast<const char *>(&(particelle_file[0])), sizeof(particelle_file) * ncol_dst * sizeof(double));
  out.write(reinterpret_cast<const char*>(&mc2), sizeof(double));

  in.close();
  out.close();

  std::cout << "\nConvertite " << particelle_file.size() << " particelle in binario .dst" << std::endl;
}


void convert_from_binary_dst_to_ascii(char *fileIN, char *fileOUT, double C)
{
  std::ifstream in(fileIN, std::ifstream::binary | std::ios::in);
  std::ofstream out(fileOUT, std::ios::out);

  //2xCHAR + INT(Np) + DOUBLE(Ib(mA)) + DOUBLE(freq(MHz)) + CHAR + Np×[6×DOUBLE(x(cm), x'(rad),y(cm),y'(rad), phi(rad), Energie(MeV))] + DOUBLE(mc2(MeV))
  int ncol_dst = 6;
  std::vector<char> support_char(2, 0);
  int np;
  double curr, freq, mc2;

  in.read(&(support_char[0]), support_char.size()*sizeof(char));
  in.read((char*)&np, sizeof(int));
  in.read((char*)&curr, sizeof(double));
  in.read((char*)&freq, sizeof(double));
  in.read(&(support_char[0]), 1 * sizeof(char));
  std::vector<std::vector<double>> particelle_file(np, std::vector<double>(ncol_dst, 0));
  in.read((char*)&(particelle_file[0]), np * ncol_dst * sizeof(double));
  in.read((char*)&mc2, sizeof(double));

  double wavelength = C / freq;
  std::vector< std::vector<double> > particelle(np, std::vector<double>(ncol_dst, 0));

  for (int i = 0; i < np; i++)
  {
    // positions along x and y must be in cm, so they are already ok!
    // px and py must be x' and y' (rad), so we should be ready!
    particelle.at(i).at(0) = particelle_file.at(i).at(0);
    particelle.at(i).at(1) = particelle_file.at(i).at(2);
    particelle.at(i).at(3) = particelle_file.at(i).at(1);
    particelle.at(i).at(4) = particelle_file.at(i).at(3);

    // pz must be converted from a particle energy in MeV to a z' (beta_z)
    double beta_transverse2 = particelle_file.at(i).at(1) * particelle_file.at(i).at(1) + particelle_file.at(i).at(3) * particelle_file.at(i).at(3);
    double relativistic_gamma = (particelle_file.at(i).at(5) / (DA_ERG_A_MEV*MP_G*C*C)) + 1.0;
    double relativistic_gamma2 = relativistic_gamma*relativistic_gamma;
    particelle.at(i).at(5) = sqrt(relativistic_gamma2 - beta_transverse2 - 1.0);

    // position along z must be converted from a phase in rad to a position in cm
    particelle.at(i).at(2) = particelle_file.at(i).at(4) * wavelength * particelle.at(i).at(5) / (2.0 * M_PI);
  }

  out << "#" << np << ' ' << freq << ' ' << curr << ' ' << mc2 << std::endl;
  for (int i = 0; i < np; i++)
  {
    for (int j = 0; j < ncol_dst; j++) out << particelle.at(i).at(j) << '\t';
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nConvertite " << particelle_file.size() << " particelle da binario .dst ad ascii" << std::endl;
}


void convert_from_binary_plt_to_ascii(char *fileIN, char *fileOUT, double C)
{
  std::ifstream in(fileIN, std::ifstream::binary | std::ios::in);
  std::ofstream out(fileOUT, std::ios::out);

  //2xCHAR+INT(Ne)+INT(Np)+DOUBLE(Ib(A))+DOUBLE(freq(MHz))+DOUBLE(mc2(MeV)) +
  // + Ne×[CHAR + INT(Nelp) + DOUBLE(Zgen) + DOUBLE(phase0(deg)) + DOUBLE(wgen(MeV)) + Np×[7×FLOAT(x(cm), x'(rad),y(cm),y'(rad), phi(rad), Energie(MeV), Loss)]]

  
  //Comments:
  //- CHAR is 1 byte long,
  //- INT is 4 bytes long,
  //- FLOAT is a Real 4 bytes long.
  //- DOUBLE is a Real 8 bytes long.
  //- Ne is the number of different positions,
  //- Np is the number of particles,
  //- Ib is the beam current,
  //- freq is the bunch frequency,
  //- mc2 is the particle rest mass,
  //- Nelp is the longitudinal element position,
  //- Zgen is the longitudinal position in cm,
  //- Phase0 & wgen are the phase and energy references of the beam
  //- Loss (UNDEFINED in TraceWin Manual - maybe related to a particle lost flag?)

  int npos, ncol_plt = 7;
  std::vector<char> support_char(2, 0);
  int np;
  double curr, freq, mc2;

  in.read(&(support_char[0]), support_char.size()*sizeof(char));
  in.read((char*)&npos, sizeof(int));
  in.read((char*)&np, sizeof(int));
  in.read((char*)&curr, sizeof(double));
  in.read((char*)&freq, sizeof(double));
  in.read((char*)&mc2, sizeof(double));

  double wavelength = C / freq;
  std::vector<int> nelp(npos, 0);
  std::vector<double> zgen(npos, 0);
  std::vector<double> phase0(npos, 0);
  std::vector<double> wgen(npos, 0);
  std::vector< std::vector< std::vector<double> > > particelle_file(npos, std::vector<std::vector<double> >(np, std::vector<double>(ncol_plt, 0)));

  for (int i = 0; i < npos; i++)
  {
    in.read(&(support_char[0]), 1 * sizeof(char));
    in.read((char*)&(nelp[i]), sizeof(int));
    in.read((char*)&(zgen[i]), sizeof(double));
    in.read((char*)&(phase0[i]), sizeof(double));
    in.read((char*)&(wgen[i]), sizeof(double));
    in.read((char*)&(particelle_file[i][0]), np * ncol_plt * sizeof(double));
  }

  out << "#" << np << ' ' << freq << ' ' << curr << ' ' << mc2 << std::endl;

  for (int i = 0; i < npos; i++)
  {
    out << "#" << npos << ' ' << nelp[i] << ' ' << zgen[i] << ' ' << phase0[i] << ' ' << wgen[i] << std::endl;
    for (int j = 0; j < np; j++)
    {
      for (int k = 0; k < ncol_plt; k++) out << particelle_file.at(i).at(j).at(k) << '\t';

      double beta_transverse2 = particelle_file.at(i).at(j).at(1) * particelle_file.at(i).at(j).at(1) + particelle_file.at(i).at(j).at(3) * particelle_file.at(i).at(j).at(3);
      double relativistic_gamma = (particelle_file.at(i).at(j).at(5) / (DA_ERG_A_MEV*MP_G*C*C)) + 1.0;
      double relativistic_gamma2 = relativistic_gamma*relativistic_gamma;
      // z' will be appended at the end of the line
      out << sqrt(relativistic_gamma2 - beta_transverse2 - 1.0) << '\t';
      // z in cm will be appended at the end of the line
      out << particelle_file.at(i).at(j).at(4) * wavelength * particelle_file.at(i).at(j).at(5) / (2.0 * M_PI);
      out << std::endl;
    }
  }

  in.close();
  out.close();

  std::cout << "\nConvertite " << particelle_file.size() << " particelle da binario .plt ad ascii" << std::endl;
}


void convert_from_propaga_to_astra(char *fileIN, char *fileOUT, double C)
{
  int N = 0;

  double z = 0.;
  double pz = 0.;

  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (contacolonne < 6)
  {
    printf("Unable to proceed, missing required columns\n");
    in.close();
    out.close();
    return;
  }

  printf("Assuming px, py and pz on columns 4-5-6\n");

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    /******************************
    // NON PIU' VALIDO, ORA LE POSIZIONI SONO ESPRESSE IN CENTIMETRI!
    // positions in input are expressed in micrometers, so we convert it in meters
    dati_particella[0] *= 1.0e-6;
    //    x += dati_particella[0];
    dati_particella[1] *= 1.0e-6;
    //    y += dati_particella[1];
    dati_particella[2] *= 1.0e-6;
    z += dati_particella[2];
    ********************************/


    // positions in input are expressed in centimeters, so we convert it in meters
    dati_particella[0] *= 1.0e-2;
    dati_particella[1] *= 1.0e-2;
    dati_particella[2] *= 1.0e-2;
    z += dati_particella[2];

    // px, py and pz's are pure numbers (p is divided by mc) but we need them in eV/c
    dati_particella[3] *= MP_EV * C;
    dati_particella[4] *= MP_EV * C;
    dati_particella[5] *= MP_EV * C;
    pz += dati_particella[5];

    N++;
    particelle_file.push_back(dati_particella);
  }


  if (N > 0)
  {
    z = z / (double)N;
    pz = pz / (double)N;
  }

  std::cout << std::setprecision(7) << "Valori medi:\nz: " << z << "\npz: " << pz;

  /**********************************************************************************************
  * The first line defines the coordinates of the reference particle in absolute coordinates.   *
  * It is recommended to refer it to the bunch center                       *
  **********************************************************************************************/

  // nb: mettiamo la particella di riferimento in z=0 e non nello z medio appena calcolato, sembra funzionare meglio ma forse e' sbagliato
  out << std::setprecision(6) << std::setiosflags(std::ios::scientific) << ZERO << "\t" << ZERO << "\t" << ZERO
    << "\t" << ZERO << "\t" << ZERO << "\t" << pz << "\t" << TEMPO << "\t" << Q_nC << "\t" << type_ASTRA << "\t" << initial_status << std::endl;

  for (int i = 0; i < N; i++)
  {
    particelle_file.at(i).at(2) -= z;
    particelle_file.at(i).at(5) -= pz;

    out << std::setprecision(6) << std::setiosflags(std::ios::scientific)
      << particelle_file.at(i).at(0) << "\t" << particelle_file.at(i).at(1) << "\t" << particelle_file.at(i).at(2) << "\t"
      << particelle_file.at(i).at(3) << "\t" << particelle_file.at(i).at(4) << "\t" << particelle_file.at(i).at(5) << "\t"
      << TEMPO << "\t" << Q_nC << "\t" << type_ASTRA << "\t" << initial_status << std::endl;
  }




  in.close();
  out.close();

  std::cout << "\nConvertite " << N << " particelle nella forma usabile da ASTRA" << std::endl;
}


void convert_from_propaga_to_astra_fixed(char *fileIN, char *fileOUT, double C, double pzref)
{
  int N = 0;

  double z = 0.;
  pzref *= (MP_EV * C);

  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (contacolonne < 6)
  {
    printf("Unable to proceed, missing required columns\n");
    in.close();
    out.close();
    return;
  }

  printf("Assuming px, py and pz on columns 4-5-6\n");

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    /******************************
    // NON PIU' VALIDO, ORA LE POSIZIONI SONO ESPRESSE IN CENTIMETRI!
    // positions in input are expressed in micrometers, so we convert it in meters
    dati_particella[0] *= 1.0e-6;
    x += dati_particella[0];
    dati_particella[1] *= 1.0e-6;
    y += dati_particella[1];
    dati_particella[2] *= 1.0e-6;
    z += dati_particella[2];
    ********************************/


    // positions in input are expressed in centimeters, so we convert it in meters
    dati_particella[0] *= 1.0e-2;
    dati_particella[1] *= 1.0e-2;
    dati_particella[2] *= 1.0e-2;
    z += dati_particella[2];

    // px, py and pz's are pure numbers (p is divided by mc) but we need them in eV/c
    dati_particella[3] *= MP_EV * C;
    dati_particella[4] *= MP_EV * C;
    dati_particella[5] *= MP_EV * C;

    N++;
    particelle_file.push_back(dati_particella);
  }

  z = z / (double)N;

  std::cout << std::setprecision(7) << "Valori medi:\nz: " << z;

  /**********************************************************************************************
  * The first line defines the coordinates of the reference particle in absolute coordinates.   *
  * It is recommended to refer it to the bunch center                       *
  **********************************************************************************************/

  // nb: mettiamo la particella di riferimento in z=0 e non nello z medio appena calcolato, sembra funzionare meglio ma forse e' sbagliato
  out << std::setprecision(6) << std::setiosflags(std::ios::scientific) << ZERO << "\t" << ZERO << "\t" << ZERO
    << "\t" << ZERO << "\t" << ZERO << "\t" << pzref << "\t" << TEMPO << "\t" << Q_nC << "\t" << type_ASTRA << "\t" << initial_status << std::endl;


  for (int i = 0; i < N; i++)
  {
    particelle_file.at(i).at(2) -= z;
    particelle_file.at(i).at(5) -= pzref;

    out << std::setprecision(6) << std::setiosflags(std::ios::scientific)
      << particelle_file.at(i).at(0) << "\t" << particelle_file.at(i).at(1) << "\t" << particelle_file.at(i).at(2) << "\t"
      << particelle_file.at(i).at(3) << "\t" << particelle_file.at(i).at(4) << "\t" << particelle_file.at(i).at(5) << "\t"
      << TEMPO << "\t" << Q_nC << "\t" << type_ASTRA << "\t" << initial_status << std::endl;
  }


  in.close();
  out.close();

  std::cout << "\nConvertite " << N << " particelle nella forma usabile da ASTRA (fixed reference particle pz)" << std::endl;
}


void convert_from_astra_to_propaga(char *fileIN, char *fileOUT, double C, double weight)
{
  int N = 0;

  double mpevc_inv = 1. / (MP_EV * C);

  data particella_base;

  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  std::vector<data> particelle_file;
  data temporanea;

  in >> particella_base.v[0] >> particella_base.v[1] >> particella_base.v[2] >> particella_base.v[3] >> particella_base.v[4] >> particella_base.v[5]
    >> particella_base.time >> particella_base.charge_double >> particella_base.id_type >> particella_base.lost;

  if (in.fail())
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }


  while (1)
  {
    in >> temporanea.v[0] >> temporanea.v[1] >> temporanea.v[2] >> temporanea.v[3] >> temporanea.v[4] >> temporanea.v[5]
      >> temporanea.time >> temporanea.charge_double >> temporanea.id_type >> temporanea.lost;

    //dati_particella[0] >> dati_particella[1] >> dati_particella[2] >> dati_particella[3] >> dati_particella[4] >> dati_particella[5]
    //>> inutile1 >> inutile2 >> tipo_particella >> status;

    if (in.eof()) break;

    temporanea.v[0] += particella_base.v[0];
    temporanea.v[1] += particella_base.v[1];
    temporanea.v[2] += particella_base.v[2];
    temporanea.v[3] += particella_base.v[3];
    temporanea.v[4] += particella_base.v[4];
    temporanea.v[5] += particella_base.v[5];

    // positions in input are expressed in meters, but we are used to have cm
    temporanea.v[0] *= 1.0e2;
    temporanea.v[1] *= 1.0e2;
    temporanea.v[2] *= 1.0e2;
    // px, py and pz's are in eV/c, but we now need them as pure numbers (p divided by mc)
    temporanea.v[3] *= mpevc_inv;
    temporanea.v[4] *= mpevc_inv;
    temporanea.v[5] *= mpevc_inv;

    if (temporanea.lost == 5)
    {
      temporanea.lost = 0;
      // conversione tipo da Astra a Fluka/Propaga
      if (temporanea.id_type == 3) temporanea.id_type = 1;
      else if (temporanea.id_type == 1) temporanea.id_type = 3;
      else temporanea.id_type = 0;
      particelle_file.push_back(temporanea);
      N++;
    }
  }

  for (int i = 0; i < N; i++)
  {
    out << std::setprecision(6) << std::setiosflags(std::ios::scientific)
      << particelle_file.at(i).v[0] << "\t" << particelle_file.at(i).v[1] << "\t" << particelle_file.at(i).v[2] << "\t"
      << particelle_file.at(i).v[3] << "\t" << particelle_file.at(i).v[4] << "\t" << particelle_file.at(i).v[5] << "\t"
      << particelle_file.at(i).id_type << "\t" << weight << "\t" << particelle_file.at(i).lost << "\t" << i << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nConvertite " << N << " particelle dalla forma usata da ASTRA alla convenzionale" << std::endl;
}


void preproc_remove_negatives(char *fileIN, char *fileOUT, int whichcol)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  whichcol--;   //cosi' puo' essere usata direttamente come indirizzo array
  if (whichcol < 0 || whichcol >= contacolonne)
  {
    printf("You asked to analyze a column which does not exist!");
    return;
  }

  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;

    if (dati_particella.at(whichcol) >= 0.0)
    {
      particelle_file.push_back(dati_particella);
      dimensione_file++;  //utile per dei check se sta crescendo troppo
    }

#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }


  for (int i = 0; i < dimensione_file; i++)
  {
    for (int j = 0; j < contacolonne; j++)
    {
      out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
    }
    out << std::endl;
  }



  in.close();
  out.close();

  std::cout << "\nConservate solo le particelle con coordinata positiva nella " << whichcol << " colonna" << std::endl;
  particelle_file.clear();

    }


void preproc_nullify_transverse_momentum(char *fileIN, char *fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;

    dati_particella.at(3) = dati_particella.at(4) = 0.;
    //    dati_particella.at(0) = dati_particella.at(1) = 0.;

    particelle_file.push_back(dati_particella);
    dimensione_file++;


#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }


  for (int i = 0; i < dimensione_file; i++)
  {
    for (int j = 0; j < contacolonne; j++)
    {
      out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
    }
    out << std::endl;
  }



  in.close();
  out.close();

  std::cout << "\nAzzerati gli impulsi trasversi delle particelle!" << std::endl;
  particelle_file.clear();

    }


void convert_from_fluka_to_propaga(char *fileIN, char *fileOUT, double weight)
{
  double C = 29979245800.0;

  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(11, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  char str[DIM_MAX_LINEA];
  in.getline(str, DIM_MAX_LINEA);

  if (in.fail())
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere se c'è qualcosa nel file
  in.clear();
  in.seekg(0, std::ios::beg);

  while (1)
  {
    in >> dati_particella.at(0) >> dati_particella.at(1) >> dati_particella.at(2)     // JTRACK, ETRACK-AM(JTRACK), ATRACK
      >> dati_particella.at(3) >> dati_particella.at(4) >> dati_particella.at(5)      // XSCO, YSCO, ZSCO,
      >> dati_particella.at(6) >> dati_particella.at(7) >> dati_particella.at(8);     // CXTRCK, CYTRCK, CZTRCK
    if (in.eof()) break;

    // non dobbiamo piu' convertire le posizioni in micrometri, siccome anche in propaga l'input e l'output ora sono in centimetri
    //    dati_particella[3] *= DA_CM_A_MICRON;
    //    dati_particella[4] *= DA_CM_A_MICRON;
    //    dati_particella[5] *= DA_CM_A_MICRON;

    if (dati_particella[0] > 0.9 && dati_particella[0] < 1.1) // e' un protone
    {
      dati_particella[9] = MP_G;
    }
    else if (dati_particella[0] > 2.9 && dati_particella[0] < 3.1)  // e' un elettrone: cfr. Particles and material codes http://www.fluka.org/fluka.php?id=man_onl&sub=7
    {
      dati_particella[9] = ME_G;
    }
    else if (dati_particella[0] > 6.9 && dati_particella[0] < 7.1) // e' un fotone, per ora non lo vogliamo nel ppg!
    {
      continue;
    }
    else // non riconosciuta, sicuramente non la vogliamo!
    {
      continue;
    }

    if (dati_particella[1] > 0.0) dati_particella[10] = 0.0;        // le particelle sono vive solo se hanno energia cinetica positiva
    else if (dati_particella[1] <= 0.0) dati_particella[10] = 1.0;

    if (dati_particella[9] >= 0.) dati_particella[1] = sqrt(pow(((dati_particella[1] * 1000.0) / (DA_ERG_A_MEV * dati_particella[9] * C*C)) + 1.0, 2.0) - 1.0);
    dati_particella[6] *= dati_particella[1];
    dati_particella[7] *= dati_particella[1];
    dati_particella[8] *= dati_particella[1];

    particelle_file.push_back(dati_particella);
    dimensione_file++;  //utile per dei check se sta crescendo troppo

#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }


  for (int i = 0; i < dimensione_file; i++)
  {
    out << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << particelle_file[i][3] << "\t" << particelle_file[i][4] << "\t" << particelle_file[i][5] << "\t"
      << particelle_file[i][6] << "\t" << particelle_file[i][7] << "\t" << particelle_file[i][8] << "\t"
      << (int)(particelle_file[i][0]) << "\t" << weight << "\t" << (int)(particelle_file[i][10]) << "\t" << i + 1 << std::endl;
  }



  in.close();
  out.close();

  std::cout << "\nConvertite " << dimensione_file << " particelle da Fluka a Propaga" << std::endl;
    }


void convert_from_propaga_to_path(char *fileIN, char *fileOUT, double C, double pz_ref, int particleId, double conv_factor, double frequency, double ref_phase)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);
  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(contacolonne, 0);
  std::vector <data> particelle_file;
  data temp;
  double average_z = 0.;
  int which_phase = 0;

  if (contacolonne < 6)
  {
    printf("Not enough data\n");
    return;
  }

  if (contacolonne == 6 && particleId < 0)
  {
    printf("What kind of particles are those in this file? (1) protons, (2) antiprotons, (3) electrons, (4) positrons: ");
    std::cin >> particleId;
  }

  if (contacolonne > 6 && particleId < 0)
  {
    printf("I'm assuming that the particleId is contained in the 7th column but I still need a reference Id.\n");
    printf("Insert (1) for a reference proton, (2) for an antiproton, (3) for an electron and (4) for a positron: ");
    std::cin >> particleId;
  }

  if (conv_factor < 0.)
  {
    printf("The positions are expressed in micrometers (1) or centimeters (2)? ");
    std::cin >> conv_factor;
  }

  if (conv_factor > 0.9 && conv_factor < 1.1) conv_factor = 1.0e-6;
  else if (conv_factor > 1.9 && conv_factor < 2.1) conv_factor = 1.0e-2;
  else
  {
    printf("Conversion factor not valid\n");
    return;
  }

  if (ref_phase < 0.)
  {
    printf("Do you want to define a reference phase (1) or to just find the average one (2)? ");
    std::cin >> which_phase;
  }

  if (which_phase == 1)
  {
    printf("Plase tell me so the reference phase (in rad): ");
    std::cin >> ref_phase;
  }

  if (frequency <= 0.)
  {
    printf("z positions must be expressed as a phase, so a frequency in MHz is required\nTell me the frequency: ");
    std::cin >> frequency;
  }

  if (pz_ref <= 0.)
  {
    printf("Tell me the reference momentum in GeV/c for the structure (p/c=γmβ, mind the m!): ");
    std::cin >> pz_ref;
  }

  std::cout << "Using these parameters:\nReference_Particle_Id: " << particleId << "\nConversion_factor: " << conv_factor
    << "\nFrequency: " << frequency << "\nReference_pz: " << pz_ref << "\nReference_phase: " << ref_phase << std::endl;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;
    if (contacolonne == 6) temp.id_type = particleId;
    else if (contacolonne > 6)
    {
      temp.id_type = (int)dati_particella[6];
      if (temp.id_type != particleId)
      {
        std::cout << "Particella scartata (contatore valide = " << dimensione_file << ") perche' non consistente con quella di riferimento" << std::endl;
        continue;
      }
    }

    temp.v[0] = dati_particella[0] * conv_factor;
    temp.v[1] = dati_particella[1] * conv_factor;
    temp.v[2] = dati_particella[2] * conv_factor;
    temp.v[3] = dati_particella[3];
    temp.v[4] = dati_particella[4];
    temp.v[5] = dati_particella[5];
    temp.xprime = atan(dati_particella[3] / dati_particella[5]);
    temp.yprime = atan(dati_particella[4] / dati_particella[5]);
    temp.phase = temp.v[2] * 2. * M_PI;
    average_z += temp.v[2];

    if (temp.id_type == 1)
    {
      temp.mass = MP_GEV;
      temp.charge_int = 1;
    }
    else if (temp.id_type == 2)
    {
      temp.mass = MP_GEV;
      temp.charge_int = -1;
    }
    else if (temp.id_type == 3)
    {
      temp.mass = ME_GEV;
      temp.charge_int = -1;
    }
    else if (temp.id_type == 4)
    {
      temp.mass = ME_GEV;
      temp.charge_int = 1;
    }
    temp.pz_diff = (dati_particella[5] * temp.mass - pz_ref) / pz_ref;
    if (contacolonne > 7) temp.lost = (int)dati_particella[7];
    else temp.lost = 0;
    if (contacolonne > 8) temp.id_number = (int)dati_particella[8];
    else temp.id_number = (int)dimensione_file + 1;

    particelle_file.push_back(temp);
    dimensione_file++;  //utile per dei check se sta crescendo troppo

#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }

  if (which_phase == 2)
  {
    average_z /= dimensione_file;
    ref_phase = average_z * 2. * M_PI;
  }
  for (int i = 0; i < dimensione_file; i++)
  {
    particelle_file.at(i).phase -= ref_phase;
  }

  /*
  time_t now;
  char* now_string;

  now = time(0);
  now_string = ctime(&now);
  */

  out << "TRAVEL BEAM DATA" << std::endl;
  //  out << now << std::endl;
  out << "01/10/2012              01:00" << std::endl;
  out << pz_ref << "\t!reference momentum (GeV/c)" << std::endl;
  out << ref_phase << "\t!reference phase (rad)" << std::endl;
  out << frequency << "\t!Frequency (Hz)" << std::endl;
  if (particleId == 1)
  {
    out << MP_GEV << "\t!reference mass (GeV/c2)" << std::endl;
    out << 1 << "\t!Reference charge" << std::endl;
  }
  else if (particleId == 2)
  {
    out << MP_GEV << "\t!reference mass (GeV/c2)" << std::endl;
    out << -1 << "\t!Reference charge" << std::endl;
  }
  else if (particleId == 3)
  {
    out << ME_GEV << "\t!reference mass (GeV/c2)" << std::endl;
    out << -1 << "\t!Reference charge" << std::endl;
  }
  else if (particleId == 4)
  {
    out << ME_GEV << "\t!reference mass (GeV/c2)" << std::endl;
    out << 1 << "\t!Reference charge" << std::endl;
  }
  out << dimensione_file << "\t!Number of particles" << std::endl;

  for (int i = 0; i < dimensione_file; i++)
  {
    out << particelle_file[i].id_number << "\t" << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << particelle_file[i].v[0] << "\t" << particelle_file[i].xprime << "\t" << particelle_file[i].v[1] << "\t"
      << particelle_file[i].yprime << "\t" << particelle_file[i].phase << "\t" << particelle_file[i].pz_diff << "\t"
      << ZERO << "\t" << particelle_file[i].charge_int << "\t" << particelle_file[i].mass << std::endl;
  }



  in.close();
  out.close();

  std::cout << "\nConvertite " << dimensione_file << " particelle dal formato ppg al txt di Travel" << std::endl;
    }


void convert_from_path_to_propaga(char *fileIN, char *fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(10, 0);
  std::vector <data> particelle_file;
  data temp;
  int colonne_distribuzione_path = 10;
  //  double average_z = 0.;
  double conv_factor = 1.0E2;
  //  int which_phase = 0;
  int number_of_particles;
  double pz_ref, ref_phase, frequency, reference_mass, reference_charge;
  char * trash;
  trash = new char[TRASH_SIZE];

  in.getline(trash, TRASH_SIZE);
  in.getline(trash, TRASH_SIZE);

  if (in.fail())
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  in >> pz_ref;
  in.getline(trash, TRASH_SIZE);

  in >> ref_phase;
  in.getline(trash, TRASH_SIZE);

  in >> frequency;
  in.getline(trash, TRASH_SIZE);

  in >> reference_mass;
  in.getline(trash, TRASH_SIZE);

  in >> reference_charge;
  in.getline(trash, TRASH_SIZE);

  in >> number_of_particles;
  in.getline(trash, TRASH_SIZE);

  size_t size_vector = number_of_particles;
  particelle_file.reserve(size_vector);

  while (1)
  {
    for (int i = 0; i < colonne_distribuzione_path; i++) in >> dati_particella.at(i);
    if (in.eof()) break;
    temp.id_number = (int)dati_particella[0];
    temp.lost = (int)dati_particella[7];
    temp.charge_int = (int)dati_particella[8];
    temp.mass = dati_particella[9];
    temp.v[0] = dati_particella[1] * conv_factor;
    temp.v[1] = dati_particella[3] * conv_factor;
    // la seguente coordinata z forse non e' corretta
    temp.v[2] = -(dati_particella[5] + ref_phase) / (2. * M_PI);
    temp.v[5] = ((dati_particella[6] * pz_ref) + pz_ref) / temp.mass;
    temp.v[3] = tan(dati_particella[2]) * temp.v[5];
    temp.v[4] = tan(dati_particella[4]) * temp.v[5];

    if (temp.charge_int == 1)
    {
      if ((temp.mass > 0.93 && temp.mass < 0.94) || (temp.mass > 930 && temp.mass < 940))
        temp.id_type = 1;
      else if ((temp.mass > 5.0E-4 && temp.mass < 5.2E-4) || (temp.mass > 5.0E-1 && temp.mass < 5.2E-1))
        temp.id_type = 4;
      else std::cout << "Particella " << temp.id_number << " non riconosciuta" << std::endl;
    }
    else if (temp.charge_int == -1)
    {
      if ((temp.mass > 0.93 && temp.mass < 0.94) || (temp.mass > 930 && temp.mass < 940))
        temp.id_type = 2;
      else if ((temp.mass > 5.0E-4 && temp.mass < 5.2E-4) || (temp.mass > 5.0E-1 && temp.mass < 5.2E-1))
        temp.id_type = 3;
      else std::cout << "Particella " << temp.id_number << " non riconosciuta" << std::endl;
    }
    else std::cout << "Particella " << temp.id_number << " non riconosciuta" << std::endl;

    particelle_file.push_back(temp);
    dimensione_file++;  //utile per controllare che alla fine dimensione_file e number_of_particles coincidano
  }

  std::cout << "Promised: " << number_of_particles << ", read: " << dimensione_file << " particles" << std::endl;

  for (int i = 0; i < dimensione_file; i++)
  {
    out << std::setprecision(7) << std::setiosflags(std::ios::scientific)
      << particelle_file[i].v[0] << "\t" << particelle_file[i].v[1] << "\t" << particelle_file[i].v[2] << "\t"
      << particelle_file[i].v[3] << "\t" << particelle_file[i].v[4] << "\t" << particelle_file[i].v[5] << "\t"
      << (int)particelle_file[i].id_type << "\t" << (int)particelle_file[i].lost << "\t" << (int)particelle_file[i].id_number << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nConvertite " << dimensione_file << " particelle dal formato txt di Travel a ppg" << std::endl;
}


void preproc_bunchfile_find_slowest(char *fileIN, char *fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  double v_inf = 1.E+100;


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    if (dati_particella.at(5) < v_inf && dati_particella.at(5) > 0.) v_inf = dati_particella.at(5);

    particelle_file.push_back(dati_particella);
    contaparticelle++;
  }

  for (int j = 0; j < contaparticelle; j++)
  {
    for (int i = 0; i < contacolonne; i++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(j).at(i) << "\t";
    out << std::endl;
  }

  for (int i = 0; i < 5; i++)       out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << ZERO << "\t";
  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << v_inf << "\t";
  for (int i = 6; i < contacolonne; i++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << ZERO << "\t";
  out << std::endl;

  in.close();
  out.close();

  if (v_inf < 1.E+100)
    std::cout << "\nPreprocessato il file convenzionale di bunch e fatto l'append ad esso di una particella \n(in asse) con pz pari al piu' piccolo pz trovato" << std::endl;

  else
    std::cout << "\nNon e' stato trovata alcuna particella \"lenta\" in questo file..." << std::endl;
}


void postproc_bunchfile_find_minmax(char *fileIN, char *fileOUT, int whichcol)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);
  int npart = 0;
  double inverse_npart = 0;
  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (whichcol > contacolonne)
  {
    printf("You asked to analyze a column which does not exist!");
    return;
  }


  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<double> particella_iniziale(contacolonne, 0);
  std::vector<double> particella_finale(contacolonne, 0);
  std::vector<double> somma_colonne(contacolonne, 0);
  std::vector<double> media_colonne(contacolonne, 0);

  double minimo = 2.0e10;
  double massimo = -2.0e10;
  double delta = 0;

  whichcol--; // cosi' da poterla utilizzare direttamente per indirizzare l'array

  while (1)
  {
    for (int i = 0; i < contacolonne; i++)
    {
      in >> dati_particella.at(i);
      somma_colonne.at(i) += dati_particella.at(i);
      media_colonne.at(i) += dati_particella.at(i);
    }
    if (in.eof()) break;

    if (dati_particella.at(whichcol) < minimo)
    {
      minimo = dati_particella.at(whichcol);
      particella_iniziale = dati_particella;
    }
    if (dati_particella.at(whichcol) > massimo)
    {
      massimo = dati_particella.at(whichcol);
      particella_finale = dati_particella;
    }
    npart++;
  }

  inverse_npart = 1.0 / npart;
  std::transform(media_colonne.begin(), media_colonne.end(), media_colonne.begin(), std::bind1st(std::multiplies<double>(), inverse_npart));

  for (int i = 0; i < contacolonne; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particella_iniziale.at(i) << "\t";
  out << std::endl;
  for (int i = 0; i < contacolonne; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particella_finale.at(i) << "\t";
  out << std::endl;
  for (int i = 0; i < contacolonne; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << somma_colonne.at(i) << "\t";
  out << std::endl;
  for (int i = 0; i < contacolonne; i++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << media_colonne.at(i) << "\t";
  out << std::endl;

  delta = fabs(massimo - minimo);
  in.close();
  out.close();

  std::cout << "\nAnalizzate " << npart << " particelle e trovati gli estremi sulla " << whichcol << " colonna; delta = " << delta << std::endl;
}


void postproc_bunchfile_binning_1D(char *fileIN, char *fileOUT, int colbin, int nbins, double low, double high, double weight)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);
  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  colbin--; // cosi' che sia valida per usarla direttamente nell'array

  if (colbin < 0 || colbin >= contacolonne)
  {
    std::cout << "Colonna non esistente per il binning" << std::endl;
    return;
  }
  if (low > high)
  {
    std::cout << "Estremi di binnaggio invertiti" << std::endl;
    return;
  }

  bool usa_colonna_pesi = false;
  int colonna_peso = 0;
  if (weight < 0)
  {
    usa_colonna_pesi = true;
    colonna_peso = (int)(-weight);    // il -1 serve per avere gia' una variabile valida per l'array c++
    std::cout << "Uso la colonna #" << colonna_peso << " per i pesi delle particelle" << std::endl;
  }
  colonna_peso--;
  if (usa_colonna_pesi && (colonna_peso < 0 || colonna_peso >= contacolonne))
  {
    std::cout << "Colonna non esistente per il weighting" << std::endl;
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  double dimbin, whichbin;
  int whichbin_int;
  double *binfilling = new double[nbins + 3];
  for (int i = 0; i < nbins + 3; i++) binfilling[i] = 0.0;

  dimbin = (high - low) / (double)nbins;

  std::cout << "Minimo: " << low << "\nMassimo: " << high << "\nNumero bin: " << nbins << "\nDimensione bin: " << dimbin << std::endl;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    if (dati_particella.at(colbin) < low)
    {
      whichbin = 0.0;
      whichbin_int = 0;
    }
    else if (dati_particella.at(colbin) > high)
    {
      whichbin = (double)(nbins + 2);
      whichbin_int = nbins + 2;
    }
    else
    {
      whichbin = (dati_particella.at(colbin) - low) / dimbin;
      whichbin_int = (int)(whichbin + 1.0);
    }

    if (usa_colonna_pesi) binfilling[whichbin_int] += dati_particella.at(colonna_peso);
    else binfilling[whichbin_int] += weight;
  }

  high = low;
  low -= dimbin;

  for (int i = 0; i < nbins + 3; i++)
  {
    out << std::setprecision(7) << low << "\t" << high << "\t" << binfilling[i] << std::endl;

    low += dimbin;
    high += dimbin;
  }

  in.close();
  out.close();

  std::cout << "\nBinnato (bin fissati) i dati sulla colonna " << colbin + 1 << std::endl;
}


void postproc_bunchfile_binning_2D(char *fileIN, char *fileOUT, int colbin1, int nbins1, double lowbin1, double highbin1, int colbin2, int nbins2, double lowbin2, double highbin2, double weight)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);
  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  colbin1--;  //per indirizzo array
  colbin2--;  //per indirizzo array

  if (colbin1 < 0 || colbin2 < 0 || colbin1 >= contacolonne || colbin2 >= contacolonne)
  {
    std::cout << "Colonna non esistente per il binning" << std::endl;
    return;
  }

  if (lowbin1 > highbin1 || lowbin2 > highbin2)
  {
    std::cout << "Estremi di binnaggio invertiti" << std::endl;
    return;
  }

  bool usa_colonna_pesi = false;
  int colonna_peso = 0;
  if (weight < 0)
  {
    usa_colonna_pesi = true;
    colonna_peso = (int)((-weight) - 1);  //-1 per indirizzo array
  }
  if (usa_colonna_pesi && (colonna_peso < 0 || colonna_peso >= contacolonne))
  {
    std::cout << "Colonna non esistente per il weighting" << std::endl;
    return;
  }


  std::vector<double> dati_particella(contacolonne, 0);

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  double minimum1, maximum1, minimum2, maximum2;
  double dimbin1, dimbin2, whichbin1, whichbin2;
  int whichbin1_int, whichbin2_int;
  double **binfilling = new double*[nbins1 + 3];
  for (int i = 0; i < nbins1 + 3; i++)
  {
    binfilling[i] = new double[nbins2 + 3];
    for (int j = 0; j < nbins2 + 3; j++) binfilling[i][j] = 0.0;
  }

  dimbin1 = (highbin1 - lowbin1) / (double)nbins1;
  std::cout << "----------\nPrimo binning\nMinimo: " << lowbin1 << "\nMassimo: " << highbin1 << "\nNumero bin: " << nbins1 << "\nDimensione bin: " << dimbin1 << std::endl;
  dimbin2 = (highbin2 - lowbin2) / (double)nbins2;
  std::cout << "----------\nSecondo binning\nMinimo: " << lowbin2 << "\nMassimo: " << highbin2 << "\nNumero bin: " << nbins2 << "\nDimensione bin: " << dimbin2 << std::endl;

  in.clear();
  in.seekg(0, std::ios::beg);

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;

    if (dati_particella.at(colbin1) < lowbin1)
    {
      whichbin1 = 0.0;
      whichbin1_int = 0;
    }
    else if (dati_particella.at(colbin1) > highbin1)
    {
      whichbin1 = (double)(nbins1 + 2);
      whichbin1_int = nbins1 + 2;
    }
    else
    {
      whichbin1 = (dati_particella.at(colbin1) - lowbin1) / dimbin1;
      whichbin1_int = (int)(whichbin1 + 1.0);
    }

    if (dati_particella.at(colbin2) < lowbin2)
    {
      whichbin2 = 0.0;
      whichbin2_int = 0;
    }
    else if (dati_particella.at(colbin2) > highbin2)
    {
      whichbin2 = (double)(nbins2 + 2);
      whichbin2_int = nbins2 + 2;
    }
    else
    {
      whichbin2 = (dati_particella.at(colbin2) - lowbin2) / dimbin2;
      whichbin2_int = (int)(whichbin2 + 1.0);
    }

    //    std::cout << "whichbin1: " << whichbin1_int << ", whichbin2: " << whichbin2_int << std::endl;
    if (usa_colonna_pesi) binfilling[whichbin1_int][whichbin2_int] += dati_particella.at(colonna_peso);
    else binfilling[whichbin1_int][whichbin2_int] += weight;

  }

  minimum1 = lowbin1 - dimbin1;
  maximum1 = lowbin1;
  minimum2 = lowbin2 - dimbin2;
  maximum2 = lowbin2;

  //  out << std::setprecision(7) << minimum1-dimbin1 << "\t" << minimum1 << "\t" << minimum2-dimbin2 << "\t" << minimum2 << "\t 0" << std::endl;

  for (int i = 0; i < nbins1 + 3; i++)
  {
    for (int j = 0; j < nbins2 + 3; j++)
    {
      out << std::setprecision(7) << minimum1 << "\t" << maximum1 << "\t" << minimum2 << "\t" << maximum2 << "\t" << binfilling[i][j] << std::endl;
      minimum2 += dimbin2;
      maximum2 += dimbin2;
    }
    //    out << std::setprecision(7) << minimum1 << "\t" << maximum1 << "\t" << minimum2 << "\t" << maximum2 << "\t 0" << std::endl;
    minimum1 += dimbin1;
    maximum1 += dimbin1;
    minimum2 = lowbin2 - dimbin2;
    maximum2 = lowbin2;
  }


  in.close();
  out.close();

  std::cout << "\nBinning 2D concluso sulle colonne x:" << colbin1 + 1 << " ed y:" << colbin2 + 1 << std::endl;
}


void preproc_energyCut(char* fileIN, char* fileOUT, double C, double meanE, double deltaE, int descrittore)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  bool usa_colonna_descrittore = false;
  int descrittore_colonna = 0;
  int64_t dimensione_file = 0;
  double mass = 0.;
  double moltiplicatore_unita_misura = 1.0;
  if (descrittore == 1 || descrittore == 2) mass = MP_G;
  else if (descrittore == 3 || descrittore == 4) mass = ME_G;
  else if (descrittore == 101 || descrittore == 102) mass = MP_G, moltiplicatore_unita_misura = 1.0E-3;   // da usare se meanE e deltaE in ingresso sono in keV
  else if (descrittore == 103 || descrittore == 104) mass = ME_G, moltiplicatore_unita_misura = 1.0E-3;   // da usare se meanE e deltaE in ingresso sono in keV
  else if (descrittore < 0)
  {
    descrittore = -descrittore;
    usa_colonna_descrittore = true;
    if (descrittore < 1 || descrittore > contacolonne)
    {
      std::cout << "Colonna non valida" << std::endl;
      return;
    }
    descrittore--;  //si puo' usare questa variabile nell'indice descrittore
  }
  else
  {
    std::cout << "Descrittore non valido" << std::endl;
    return;
  }

  meanE *= moltiplicatore_unita_misura;
  deltaE *= moltiplicatore_unita_misura;

  std::vector<double> dati_particella(contacolonne + 1, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;
    //E = (DA_ERG_A_MEV) * (gamma-1) * massaP * c^2
    if (usa_colonna_descrittore)
    {
      descrittore_colonna = (int)dati_particella.at(descrittore);
      if (descrittore_colonna == 1 || descrittore_colonna == 2) mass = MP_G;
      else if (descrittore_colonna == 3 || descrittore_colonna == 4) mass = ME_G;

      dati_particella.at(contacolonne) = DA_ERG_A_MEV * (sqrt(1.0 + dati_particella[3] * dati_particella[3] + dati_particella[4] * dati_particella[4]
        + dati_particella[5] * dati_particella[5]) - 1.0) * mass * C * C;
    }
    else
    {
      dati_particella.at(contacolonne) = DA_ERG_A_MEV * (sqrt(1.0 + dati_particella[3] * dati_particella[3] + dati_particella[4] * dati_particella[4]
        + dati_particella[5] * dati_particella[5]) - 1.0) * mass * C * C;
    }
    if (dati_particella.at(contacolonne) >= (meanE - deltaE) && dati_particella.at(contacolonne) <= (meanE + deltaE))
    {
      particelle_file.push_back(dati_particella);
      dimensione_file++;  //utile per dei check se sta crescendo troppo
    }
#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }

  std::cout << "Elaborate " << dimensione_file << " particelle" << std::endl;

  for (int i = 0; i < dimensione_file; i++)
  {
    for (int j = 0; j < contacolonne; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(j) << "\t";
    out << std::endl;
  }


  in.close();
  out.close();

  std::cout << "\nPreprocessato il file e rimosse le particelle con energia diversa da " << meanE << "MeV ± " << deltaE << "MeV" << std::endl;

    }


void preproc_energyCutAbove(char* fileIN, char* fileOUT, double C, double minE, int descrittore)
// nb: se minE e' negativa, vengono selezionate le particelle aventi energia inferiore al valore assoluto dell'energia minima comunicata
// in questa routine si assume che nelle colonne 4, 5, 6 ci siano i momenti normalizzati, indipendentemente da quante altre colonne esistano nel file
// se il descrittore e' negativo, si assume che esso equivalga, in valore assoluto, alla colonna nella quale leggere il descrittore per ciascuna particella
{
  bool leggi_descrittore = false;
  bool below = false;
  if (minE < 0.)
  {
    below = true;
    minE = -minE;
  }
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  in.clear();
  in.seekg(0, std::ios::beg);
  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  int64_t dimensione_file = 0;
  double mass = 0.0;
  double moltiplicatore_unita_misura = 1.0;
  int colonna_descrittore = 0;
  if (descrittore == 1 || descrittore == 2) mass = MP_G;
  else if (descrittore == 3 || descrittore == 4) mass = ME_G;
  else if (descrittore == 101 || descrittore == 102) mass = MP_G, moltiplicatore_unita_misura = 1.0E-3;   // da usare se meanE e deltaE in ingresso sono in keV
  else if (descrittore == 103 || descrittore == 104) mass = ME_G, moltiplicatore_unita_misura = 1.0E-3;   // da usare se meanE e deltaE in ingresso sono in keV
  else if (descrittore < 0) { leggi_descrittore = true; colonna_descrittore = -descrittore; colonna_descrittore--; }        // inverto il segno e calo di uno per poterlo usare nell'indicizzazione colonna
  else
  {
    std::cout << "Descrittore non valido" << std::endl;
    return;
  }

  if (below && !leggi_descrittore)
  {
    printf("Cutting distribution file below %f ", minE);
    if (descrittore == 1 || descrittore == 2) printf("MeV\n");
    else printf("keV\n");
  }
  else if (!below && !leggi_descrittore)
  {
    printf("Cutting distribution file above %f ", minE);
    if (descrittore == 1 || descrittore == 2) printf("MeV\n");
    else printf("keV\n");
  }
  else printf("Cutting distribution file using your requests: input value: %f\n", minE);

  minE *= moltiplicatore_unita_misura;

  std::vector<double> dati_particella(contacolonne + 1, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;
    //E = (DA_ERG_A_MEV) * (gamma-1) * massaP * c^2
    if (leggi_descrittore)
    {
      descrittore = (int)dati_particella.at(colonna_descrittore);
      if (descrittore == 1 || descrittore == 2) mass = MP_G;
      else if (descrittore == 3 || descrittore == 4) mass = ME_G;
      else
      {
        printf("Descrittore non riconosciuto\n");
        return;
      }
    }
    dati_particella.at(contacolonne) = DA_ERG_A_MEV * (sqrt(1.0 + dati_particella[3] * dati_particella[3] + dati_particella[4] * dati_particella[4]
      + dati_particella[5] * dati_particella[5]) - 1.0) * mass * C * C;
    if (below)
    {
      if (dati_particella.at(contacolonne) <= (minE))
      {
        particelle_file.push_back(dati_particella);
        dimensione_file++;  //utile per dei check se sta crescendo troppo
      }
    }
    else
    {
      if (dati_particella.at(contacolonne) >= (minE))
      {
        particelle_file.push_back(dati_particella);
        dimensione_file++;  //utile per dei check se sta crescendo troppo
      }
    }
#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }

  std::cout << "Selezionate " << dimensione_file << " particelle. Inizio la scrittura..." << std::endl;

  for (int i = 0; i < dimensione_file; i++)
  {
    for (int j = 0; j < contacolonne; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(j) << "\t";
    out << std::endl;
  }


  in.close();
  out.close();

  if (below) std::cout << "\nPreprocessato il file e selezionate le particelle con energia inferiore a  " << minE << "MeV" << std::endl;
  else std::cout << "\nPreprocessato il file e selezionate le particelle con energia superiore a  " << minE << "MeV" << std::endl;

    }


void postproc_split_by_weight(char* fileIN, int colweight)
{
  std::ifstream in(fileIN);

  //  int64_t dimensione_file = 0;
  //  int numero_pesi = 0;
  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed! No files have been created, even empty ones.\nPay attention to subsequent scripted actions\n");
    in.close();
    return;
  }

  if (colweight < 1 || colweight > contacolonne)
  {
    std::cout << "Colonna peso non valida" << std::endl;
    return;
  }
  else
  {
    std::cout << "Lavoro sulla " << colweight << "a colonna" << std::endl;
    colweight--;
  }

  std::vector<double> pesi(1, 0);
  std::vector<int> conta_particelle_per_peso(1, 0);
  std::vector<double> dati_particella(contacolonne, 0);


  std::vector<std::vector <double> > file_completo;

  std::vector<std::vector<std::vector <double> > > files_splittati;

  for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
  pesi[0] = dati_particella.at(colweight);
  conta_particelle_per_peso[0]++;
  file_completo.push_back(dati_particella);
  std::cout << "Trovato nuovo peso: " << dati_particella.at(colweight) << std::endl;

  int trovato_i = 0;
  bool trovato = false;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;

    trovato = false;
    for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
    {
      if (doubleEquality(dati_particella.at(colweight), pesi.at(i)))
      {
        trovato_i = i;
        trovato = true;
        break;
      }
    }

    if (trovato) conta_particelle_per_peso[trovato_i]++;
    else
    {
      pesi.push_back(dati_particella.at(colweight));
      conta_particelle_per_peso.push_back(1);
      std::cout << "Trovato nuovo peso: " << dati_particella.at(colweight) << "; estesi gli array pesi a dim: " << pesi.size() << std::endl;
      std::cout << " e conta_particelle_per_peso a dim: " << conta_particelle_per_peso.size() << std::endl;
    }

    file_completo.push_back(dati_particella);
  }

  files_splittati.resize(pesi.size());
  for (std::vector<double>::size_type kk = 0; kk < file_completo.size(); kk++)
  {
    trovato = false;
    for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
    {
      if (doubleEquality(file_completo.at(kk).at(colweight), pesi.at(i)))
      {
        trovato_i = i;
        trovato = true;
        break;
      }
    }

    files_splittati.at(trovato_i).push_back(file_completo.at(kk));
  }

  std::cout << "peso\t#particelle\tsizeArray" << std::endl;
  for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
  {
    std::cout << pesi.at(i) << "\t" << conta_particelle_per_peso.at(i) << "\t" << files_splittati.at(i).size() << std::endl;
  }

  char file_output_name[100];
  std::ofstream *output_files = new std::ofstream[pesi.size()];
  for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
  {
    sprintf(file_output_name, "peso%lu.%s", i, fileIN);
    output_files[i].open(file_output_name);
  }

  for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
  {
    for (std::vector<double>::size_type k = 0; k < files_splittati.at(i).size(); k++)
    {
      for (int j = 0; j < contacolonne; j++) output_files[i] << std::setprecision(7) << std::setiosflags(std::ios::scientific) << files_splittati.at(i)[k][j] << "\t";
      output_files[i] << std::endl;
    }
  }


  in.close();
  for (std::vector<double>::size_type i = 0; i < pesi.size(); i++)
  {
    output_files[i].close();
  }
  std::cout << "\nSeparati le particelle per peso in vari files" << std::endl;
}


void postproc_split_p_and_e(char* fileIN, char* fileOUT1, char* fileOUT2)
{
  std::ifstream in(fileIN);
  std::ofstream out_p(fileOUT1);
  std::ofstream out_e(fileOUT2);

  int64_t dimensione_file = 0;
  int64_t numero_protoni = 0;
  int64_t numero_elettroni = 0;

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out_p.close();
    out_e.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);
    if (in.eof()) break;

    dimensione_file++;  //utile per dei check se sta crescendo troppo
    particelle_file.push_back(dati_particella);

#ifdef USE_RESERVE
    if (!(dimensione_file % size_vector))
    {
      multiplo++;
      particelle_file.reserve(size_vector*multiplo);
    }
#endif
  }

  std::cout << "Lette " << dimensione_file << " particelle" << std::endl;

  for (int i = 0; i < dimensione_file; i++)
  {
    if ((particelle_file[i][6] > 0.9 && particelle_file[i][6] < 1.1) || (particelle_file[i][6] > -0.1 && particelle_file[i][6] < 0.1))
    {
      for (int j = 0; j < 3; j++) out_p << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
      for (int j = 3; j < 6; j++) out_p << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
      out_p << (int)(particelle_file[i][6]) << "\t";
      out_p << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][7] << "\t";
      out_p << (int)(particelle_file[i][8]) << "\t";
      out_p << (int)(particelle_file[i][9]) << std::endl;
      numero_protoni++;
    }
    else if (particelle_file[i][6] > 2.9 && particelle_file[i][6] < 3.1)
    {
      for (int j = 0; j < 3; j++) out_e << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
      for (int j = 3; j < 6; j++) out_e << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][j] << "\t";
      out_e << (int)(particelle_file[i][6]) << "\t";
      out_e << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i][7] << "\t";
      out_e << (int)(particelle_file[i][8]) << "\t";
      out_e << (int)(particelle_file[i][9]) << std::endl;
      numero_elettroni++;
    }
  }


  in.close();
  out_p.close();
  out_e.close();

  std::cout << "\nSeparati i " << numero_protoni << " protoni dai " << numero_elettroni << " elettroni nei due files prodotti" << std::endl;
    }


void preproc_angularCut(char* fileIN, char* fileOUT, double angleMin, double angleMax)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  double angle, pz2;

  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;
    pz2 = dati_particella.at(5)*dati_particella.at(5);
    angle = atan(sqrt((dati_particella.at(3)*dati_particella.at(3) / pz2) + (dati_particella.at(4)*dati_particella.at(4) / pz2)));
    angle *= 1000.;

    if (angle >= angleMin && angle <= angleMax && dati_particella.at(5) > 0.0)
    {
      particelle_file.push_back(dati_particella);
      contaparticelle++;
    }
  }

  for (int i = 0; i < contaparticelle; i++)
  {
    for (int j = 0; j < contacolonne; j++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nPreprocessato il file e rimosse le particelle con angolo di apertura superiore a " << angleMax << " mrad" << std::endl;

}


void postproc_full_angle_and_energy(char* fileIN, char* fileOUT, double C, int colpx, int colpy, int colpz, int coltype, int colweight)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (colpx < 1 || colpx > contacolonne) std::cout << "Colonna px non trovata" << std::endl;
  if (colpy < 1 || colpy > contacolonne) std::cout << "Colonna py non trovata" << std::endl;
  if (colpz < 1 || colpz > contacolonne) std::cout << "Colonna pz non trovata" << std::endl;
  if ((coltype < 1 && coltype != -1 && coltype != -2 && coltype != -3 && coltype != -4) || coltype > contacolonne) std::cout << "Colonna tipo particella non trovata o tipo particella non valido" << std::endl;
  if ((colweight < 1 && colweight != -1) || colweight > contacolonne) std::cout << "Colonna peso particella non trovata oppure peso unitario non definito" << std::endl;

  colpx--; colpy--; colpz--; // per poterli usare direttamente negli array, che iniziano da 0
  if (coltype > 0) coltype--;
  if (colweight > 0) colweight--;
  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(contacolonne + 5, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  double pz2, mass;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;
    dimensione_file++;
    pz2 = dati_particella.at(colpz)*dati_particella.at(colpz);
    if (coltype >= 0)
    {
      if (dati_particella.at(coltype) > 0.9 && dati_particella.at(coltype) < 1.1) mass = MP_G;
      else if (dati_particella.at(coltype) > 1.9 && dati_particella.at(coltype) < 2.1) mass = MP_G;
      else if (dati_particella.at(coltype) > 2.9 && dati_particella.at(coltype) < 3.1) mass = ME_G;
      else if (dati_particella.at(coltype) > 3.9 && dati_particella.at(coltype) < 4.1) mass = ME_G;
      else
      {
        std::cout << "Ho incontrato una particella non riconoscibile, la #" << dimensione_file << std::endl;
        return;
      }
    }
    else if (coltype == -4 || coltype == -3) mass = ME_G;
    else if (coltype == -2 || coltype == -1) mass = MP_G;
    else mass = MP_G;   // storicamente di default andiamo sui protoni

    dati_particella.at(contacolonne) = atan(sqrt((dati_particella.at(colpx)*dati_particella.at(colpx) / pz2) + (dati_particella.at(colpy)*dati_particella.at(colpy) / pz2)));
    dati_particella.at(contacolonne + 1) = atan(dati_particella.at(colpx) / dati_particella.at(colpz));
    dati_particella.at(contacolonne + 2) = atan(dati_particella.at(colpy) / dati_particella.at(colpz));
    dati_particella.at(contacolonne + 3) = 2 * M_PI*(1.0 - cos(dati_particella.at(contacolonne)));
    dati_particella.at(contacolonne) *= 1000.;  // from rad to mrad
    dati_particella.at(contacolonne + 1) *= 1000.;  // from rad to mrad
    dati_particella.at(contacolonne + 2) *= 1000.;  // from rad to mrad
    dati_particella.at(contacolonne + 3) *= 1000.;  // from srad to msr
    //E = (DA_ERG_A_MEV) * (gamma-1) * massaP * c^2
    dati_particella.at(contacolonne + 4) = DA_ERG_A_MEV * (sqrt(1.0 + dati_particella.at(colpx)*dati_particella.at(colpx) + dati_particella.at(colpy)*dati_particella.at(colpy) + dati_particella.at(colpz)*dati_particella.at(colpz)) - 1.0) * mass * C * C;
    particelle_file.push_back(dati_particella);
  }

  for (int i = 0; i < dimensione_file; i++)
  {
    if (coltype >= 0 && colweight != -1)        out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(coltype) << "\t" << particelle_file[i].at(colweight) << "\t";
    else                        out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << -coltype << "\t" << -colweight << "\t";
    for (int j = contacolonne; j < contacolonne + 5; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(j) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nProcessato il file e prodotto il file contenente l'angolo piano, l'angolo solido e l'energia" << std::endl;
}


void postproc_xyzE(char* fileIN, char* fileOUT, double C, int colpx, int colpy, int colpz, int coltype)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  if (colpx < 1 || colpx > contacolonne) std::cout << "Colonna px non trovata" << std::endl;
  if (colpy < 1 || colpy > contacolonne) std::cout << "Colonna py non trovata" << std::endl;
  if (colpz < 1 || colpz > contacolonne) std::cout << "Colonna pz non trovata" << std::endl;
  if ((coltype < 1 && coltype != -1 && coltype != -2 && coltype != -3 && coltype != -4) || coltype > contacolonne) std::cout << "Colonna tipo particella non trovata o tipo particella non valido" << std::endl;

  colpx--; colpy--; colpz--; // per poterli usare direttamente negli array, che iniziano da 0
  if (coltype > 0) coltype--;
  int64_t dimensione_file = 0;
  std::vector<double> dati_particella(contacolonne + 1, 0);
  std::vector<std::vector <double> > particelle_file;

#ifdef USE_RESERVE
  size_t size_vector = RESERVE_SIZE_VECTOR;
  int multiplo = 1;
  particelle_file.reserve(size_vector*multiplo);
#endif

  double mass;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;
    dimensione_file++;
    if (coltype >= 0)
    {
      if (dati_particella.at(coltype) > 0.9 && dati_particella.at(coltype) < 1.1) mass = MP_G;
      else if (dati_particella.at(coltype) > 1.9 && dati_particella.at(coltype) < 2.1) mass = MP_G;
      else if (dati_particella.at(coltype) > 2.9 && dati_particella.at(coltype) < 3.1) mass = ME_G;
      else if (dati_particella.at(coltype) > 3.9 && dati_particella.at(coltype) < 4.1) mass = ME_G;
      else
      {
        std::cout << "Ho incontrato una particella non riconoscibile, la #" << dimensione_file << std::endl;
        return;
      }
    }
    else if (coltype == -4 || coltype == -3) mass = ME_G;
    else if (coltype == -2 || coltype == -1) mass = MP_G;
    else mass = MP_G;   // storicamente di default andiamo sui protoni

    dati_particella.at(contacolonne) = DA_ERG_A_MEV * (sqrt(1.0 + dati_particella.at(colpx)*dati_particella.at(colpx) + dati_particella.at(colpy)*dati_particella.at(colpy) + dati_particella.at(colpz)*dati_particella.at(colpz)) - 1.0) * mass * C * C;
    particelle_file.push_back(dati_particella);
  }

  for (int i = 0; i < dimensione_file; i++)
  {
    for (int j = 0; j < 3; j++) out << std::setprecision(5) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(j) << "\t";
    out << std::setprecision(5) << std::setiosflags(std::ios::scientific) << particelle_file[i].at(contacolonne) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nProcessato il file: ne e' stato prodotto uno nuovo contenente solo x, y, z ed E" << std::endl;
}


void postproc_removeLostParticles(char *fileIN, char* fileOUT, int colstatus)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

  colstatus--;  // so that it can be used with the array index
  if (colstatus < 0 || colstatus >= contacolonne)
  {
    std::cout << "Colonna stato particella non trovata" << std::endl;
    return;
  }


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    if (dati_particella.at(colstatus) > -0.1 && dati_particella.at(colstatus) < 0.1)
    {
      particelle_file.push_back(dati_particella);
      contaparticelle++;
    }
  }

  for (int i = 0; i < contaparticelle; i++)
  {
    for (int j = 0; j < contacolonne; j++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nPreprocessato il file e rimosse le particelle perse (assorbite)." << std::endl;
}


void preproc_subsample(char *fileIN, char* fileOUT, int jump)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;

    if (contaparticelle % jump == 0) particelle_file.push_back(dati_particella);
    contaparticelle++;
  }

  printf("Saved %lu particles\n", particelle_file.size());

  for (std::vector<double>::size_type i = 0; i < particelle_file.size(); i++)
  {
    for (int j = 0; j < contacolonne; j++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nPreprocessato il file e costruito un subsample ad 1:" << jump << std::endl;
}


void postproc_removeParticlesGoneToInfinity(char *fileIN, char* fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;

  double infinity = 1.E+100;

  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;


    if (dati_particella.at(0) < infinity) particelle_file.push_back(dati_particella);
  }


  for (std::vector<double>::size_type i = 0; i < particelle_file.size(); i++)
  {
    for (int j = 0; j < contacolonne; j++)  out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }

  in.close();
  out.close();

  std::cout << "\nPostprocessato il file e rimosse le particelle andate all'infinito." << std::endl;
}


//INCOMPLETO, fa l'output solo di x,y,z: non so come impostare un file contenente piu' colonne in vtk, non viene accettato se ne ha >3
void from_ppg_to_vtk(char* fileIN, char* fileOUT)
{
  std::ifstream in(fileIN);
  std::ofstream out(fileOUT);

  int contacolonne = 0, contaparticelle = 0;
  char str[DIM_MAX_LINEA];
  char * pch;
  in.getline(str, DIM_MAX_LINEA);
  // riportiamo lo stream di lettura all'inizio, la prima riga la leggiamo solo per sapere la struttura in colonne del file
  in.clear();
  in.seekg(0, std::ios::beg);

  //  printf ("Splitting string \"%s\" into tokens:\n",str);

  pch = strtok(str, " \t");
  if (pch != NULL) contacolonne++;
  while (pch != NULL)
  {
    pch = strtok(NULL, " \t");
    if (pch != NULL) contacolonne++;
  }
  printf("Found %d columns in your file\n", contacolonne);

  if (contacolonne == 0)
  {
    printf("Unable to proceed!\n");
    in.close();
    out.close();
    return;
  }

  std::vector<double> dati_particella(contacolonne, 0);
  std::vector<std::vector <double> > particelle_file;


  while (1)
  {
    for (int i = 0; i < contacolonne; i++) in >> dati_particella.at(i);

    if (in.eof()) break;
    particelle_file.push_back(dati_particella);
    contaparticelle++;
  }

  out << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET UNSTRUCTURED_GRID\nPOINTS " << contaparticelle << " float" << std::endl;

  for (int i = 0; i < contaparticelle; i++)
  {
    for (int j = 0; j < 3; j++) out << std::setprecision(7) << std::setiosflags(std::ios::scientific) << particelle_file.at(i).at(j) << "\t";
    out << std::endl;
  }


  in.close();
  out.close();

  std::cout << "\nTrasformazione file in vtk completata" << std::endl;


}




int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cout << "Funzionamento: a.out mode[1=auto,2=manual] C[onlyif mode==1] how[onlyif mode==1]:"
      << "\n1=toASTRA"
      << "\n2=fromASTRA"
      << "\n3=preproc_findslowest"
      << "\n4=remove_particlesGoneToInfinity"
      << "\n8=from_ppg_to_vtk"
      << "\n9=remove_lostParticles"
      << "\n15=make_a_subset [jump]"
      << "\n16=xyz_from_micron_to_cm"
      << "\n20=angular_cut(mrad) [anglemin anglemax]"
      << "\n25=fromNewPropaga_to_OldPropaga"
      << "\n26=toASTRA_fixedRef [pz_reference]"
      << "\n28=energy_cut [idParticle]"
      << "\n29=from_fluka_to_propaga"
      << "\n30=split_p_and_e"
      << "\n31=fromOldPropaga_to_NewPropaga [idParticle] [weight] [switchXZ]"
      << "\n33=find_min_max [whichcol]"
      << "\n35=remove_negatives_from_a_column [whichcol]"
      << "\n36=to_PATH [pz_reference idParticle conv_factor frequency phase_reference]"
      << "\n37=from_PATH"
      << "\n38=cutE_above_below [energy: if >0 cut above, if <0 cut below]"
      << "\n39=bin2D [colbinX nbinX minX maxX colbinY nbinY minY maxY weight]"
      << "\n40=bin1D [colbinX nbinX minX maxX weight]"
      << "\n41=from_beta_to_angle_and_energy [colpx colpy colpz coltype colweight]"
      << "\n42=from_jasmine"
      << "\n43=split_by_weight [colweight]"
      << "\n44=remove_transverse_momentum"
      << "\n45=write_x_y_z_E"
      << "\n46=from_propaga_to_dst"
      << "\n47=from_dst_to_ascii"
      << "\n48=from_plt_to_ascii"
      << "\ninput_file"
      << std::endl;
    return -254;
  }

  else
  {
    int *argV;
    double *argVdouble;
    argV = new int[argc];
    argVdouble = new double[argc];

    argV[1] = atoi(argv[1]);

    char output_file[100];
    char output_file2[100];
    double C = 0.0;
    int changeC = 0;
    int flow = 0;
    double meanE = 0.0, deltaE = 0.0, angleMin = -1.0, angleMax = -1.0, highbin = 0.0, lowbin = 0.0, pzref, conv_factor, frequency, ref_phase;
    int particleId = 1, colbin = -1, colbin2 = -1, nbins = -1, nbins2 = -1, jump = -1;
    int descriptor = 0;
    double weight = 0.;
    int colweight = 0, colstatus = 0;
    switch (argV[1])
    {
    case 2:
      if (argc != 3) { std::cout << "Non hai specificato correttamente i parametri di avvio" << std::endl; return -252; }
      sprintf(output_file, "conv.%s", argv[2]);
      sprintf(output_file2, "conv2.%s", argv[2]);
      std::cout << "Lettura " << argv[2] << " e scrittura su " << output_file << std::endl;

      std::cout << "Cosa vuoi fare?" << std::endl;
      std::cout << " 1- Trasformare file convenzionali in file di astra" << std::endl;
      std::cout << " 2- Viceversa" << std::endl;
      std::cout << " 3- Preparare un file convenzionale per Propaga (MPI)" << std::endl;
      std::cout << " 4- Rimuovere le particelle andate all'infinito" << std::endl;
      std::cout << " 8- From PPG to VTK" << std::endl;
      std::cout << " 9- Rimuovere le particelle perse" << std::endl;
      std::cout << "15- Costruire un subset del file iniziale" << std::endl;
      std::cout << "16- Modificare le colonne x, y, z da micrometri a centimetri" << std::endl;
      std::cout << "20- Filtrare le particelle in un certo cono angolare" << std::endl;
      std::cout << "25- Trasformare un file a 9 colonne in uno a 6 colonne" << std::endl;
      std::cout << "26- Trasformare file convenzionali in file di astra fissando il pz della particella di riferimento" << std::endl;
      std::cout << "28- Filtrare le particelle in energia" << std::endl;
      std::cout << "29- Convertire il file da FLUKA mgdraw.f a Propaga" << std::endl;
      std::cout << "30- Separare in un file i protoni dagli elettroni" << std::endl;
      std::cout << "31- Aggiornare un file di input di propaga da 6 a 9 colonne" << std::endl;
      std::cout << "33- Cercare minimi e massimi di una colonna in un file di n colonne (nuovo)" << std::endl;
      std::cout << "35- Rimuovere le particelle che hanno la coordinata negativa ad una colonna prefissata" << std::endl;
      std::cout << "36- Convertire il file da Propaga a Path/Travel" << std::endl;
      std::cout << "37- Convertire il file da Path/Travel a Propaga" << std::endl;
      std::cout << "38- Tagliare in energia sopra o sotto ad un certo valore" << std::endl;
      std::cout << "39- Binnare pesato in 2D, con tutti i valori min e max prefissati" << std::endl;
      std::cout << "40- Binnare pesato in 1D, con i valori min e max prefissati" << std::endl;
      std::cout << "41- Creare un file con tipo particella, peso, angolo piano, angoli xz/yz, angolo solido, energia" << std::endl;
      std::cout << "42- Convertire un file binario di Jasmine in un file ascii per Propaga" << std::endl;
      std::cout << "43- Separare le particelle in files distinti in base al loro peso" << std::endl;
      std::cout << "44- Rimuovere gli impulsi trasversi dalla distribuzione" << std::endl;
      std::cout << "45- Creare un file con x, y, z, E" << std::endl;
      std::cout << "46- Convertire un file Propaga in un file binario .dst" << std::endl;
      std::cout << "47- Convertire un file binario .dst in ascii" << std::endl;
      std::cout << "48- Convertire un file binario .plt in ascii" << std::endl;

      std::cout << ": ";
      std::cin >> flow;

      if (flow == 1)
      {
        std::cout << "Vuoi usare per c il valore\n1: 1 (unita' naturali) oppure\n2: 299 792 458 m/s oppure\n3: 2.99792458e+10 cm/s?\n: ";
        while (changeC != 1 && changeC != 2 && changeC != 3)
        {
          std::cin >> changeC;
          if (changeC != 1 && changeC != 2 && changeC != 3)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }

        if (changeC == 1) C = 1.0;
        else if (changeC == 2) C = 299792458.0;
        else if (changeC == 3) C = 29979245800.0;
        else
        {
          std::cerr << "E' successo qualcosa di veramente grave con il valore di c" << std::endl;
          return -255;
        }
        std::cout << "Il valore usato di c e' " << C << std::endl;
        std::cout << "Non usare file contenenti la particella lenta di riferimento! Nel caso, eliminarla prima di procedere!" << std::endl;
        convert_from_propaga_to_astra(argv[2], output_file, C);
      }
      else if (flow == 2)
      {
        std::cout << "Vuoi usare per c il valore\n1: 1 (unita' naturali) oppure\n2: 299 792 458 m/s oppure\n3: 2.99792458e+10 cm/s?\n: ";
        while (changeC != 1 && changeC != 2 && changeC != 3)
        {
          std::cin >> changeC;
          if (changeC != 1 && changeC != 2 && changeC != 3)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }

        if (changeC == 1) C = 1.0;
        else if (changeC == 2) C = 299792458.0;
        else if (changeC == 3) C = 29979245800.0;
        else
        {
          std::cerr << "E' successo qualcosa di veramente grave con il valore di c" << std::endl;
          return -255;
        }
        std::cout << "Il valore usato di c e' " << C << std::endl;
        std::cout << "dimmi il peso delle particelle: " << std::endl;
        std::cin >> weight;
        convert_from_astra_to_propaga(argv[2], output_file, C, weight);
      }
      else if (flow == 3) preproc_bunchfile_find_slowest(argv[2], output_file);
      else if (flow == 4) postproc_removeParticlesGoneToInfinity(argv[2], output_file);
      else if (flow == 8) from_ppg_to_vtk(argv[2], output_file);
      else if (flow == 9)
      {
        std::cout << "dimmi la colonna nella quale cercare lo stato delle particelle: " << std::endl;
        std::cin >> colstatus;
        postproc_removeLostParticles(argv[2], output_file, colstatus);
      }
      else if (flow == 15)
      {
        C = 1.0;
        std::cout << "Per fare un sottoinsieme del file di input (a 6 colonne) devi darmi un parametro di jump.\nJump: ";
        while (jump <= 0)
        {
          std::cin >> jump;
          if (jump <= 0)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }
        preproc_subsample(argv[2], output_file, jump);
      }
      else if (flow == 16) preproc_from_micron_to_cm(argv[2], output_file);
      else if (flow == 20)
      {
        C = 29979245800.0;
        std::cout << "Per tagliare in angolo voglio l'angolo minimo [mrad] (può anche essere zero se si vuole il cono intero): ";
        while (angleMin < 0.0)
        {
          std::cin >> angleMin;
          if (angleMin < 0.0)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }
        std::cout << "e l'angolo massimo [mrad], che deve essere maggiore del minimo: ";
        while (angleMax < angleMin)
        {
          std::cin >> angleMax;
          if (angleMax < angleMin)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }
        preproc_angularCut(argv[2], output_file, angleMin, angleMax);
      }
      else if (flow == 25) preproc_from_NewP_to_OldP(argv[2], output_file);
      else if (flow == 26)
      {
        std::cout << "Vuoi usare per c il valore\n1: 1 (unita' naturali) oppure\n2: 299 792 458 m/s oppure\n3: 2.99792458e+10 cm/s?\n: ";
        while (changeC != 1 && changeC != 2 && changeC != 3)
        {
          std::cin >> changeC;
          if (changeC != 1 && changeC != 2 && changeC != 3)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }

        if (changeC == 1) C = 1.0;
        else if (changeC == 2) C = 299792458.0;
        else if (changeC == 3) C = 29979245800.0;
        else
        {
          std::cerr << "E' successo qualcosa di veramente grave con il valore di c" << std::endl;
          return -255;
        }
        std::cout << "Il valore usato di c e' " << C << std::endl;
        std::cout << "dimmi il pz della particella di riferimento: " << std::endl;
        std::cin >> pzref;
        convert_from_propaga_to_astra_fixed(argv[2], output_file, C, pzref);
      }
      else if (flow == 28)
      {
        C = 29979245800.0;
        std::cout << "Per tagliare in energia voglio il valore medio e il delta dentro il quale accettarle.\nE media (MeV): ";
        while (meanE <= 0.0)
        {
          std::cin >> meanE;
          if (meanE <= 0.0)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }
        std::cout << "delta E (MeV): ";
        while (deltaE <= 0.0)
        {
          std::cin >> deltaE;
          if (deltaE <= 0.0)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }
        std::cout << "dimmi se sono protoni (1) o elettroni (3) o la colonna dove leggerlo (con un - davanti): " << std::endl;
        std::cin >> descriptor;
        preproc_energyCut(argv[2], output_file, C, meanE, deltaE, descriptor);
      }
      else if (flow == 29)
      {
        std::cout << "dimmi il peso delle particelle: " << std::endl;
        std::cin >> weight;
        convert_from_fluka_to_propaga(argv[2], output_file, weight);
      }
      else if (flow == 30) postproc_split_p_and_e(argv[2], output_file, output_file2);
      else if (flow == 31)
      {
        std::cout << "Dimmi l'id delle particelle in input: 1 per protoni, 3 per elettroni: ";
        do
        {
          std::cin >> particleId;
          if (particleId != 1 && particleId != 3)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        } while (particleId != 1 && particleId != 3);
        std::cout << "Dimmi peso delle particelle (con un - davanti la colonna dove trovarlo): ";
        std::cin >> weight;
        std::cout << "E' necessario ribaltare la colonna x con quella z? 1 si, 0 no: ";
        do
        {
          std::cin >> colbin;
          if (colbin != 0 && colbin != 1)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        } while (colbin != 1 && colbin != 0);
        preproc_from_OldP_to_NewP(argv[2], output_file, particleId, weight, colbin);
      }
      else if (flow == 33)
      {
        std::cout << "Dimmi quale colonna analizzare: ";
        std::cin >> colbin;
        postproc_bunchfile_find_minmax(argv[2], output_file, colbin);
      }
      else if (flow == 35)
      {
        C = 1.0;
        std::cout << "Dimmi quale colonna analizzare alla ricerca dei valori negativi da rimuovere.\nNum. colonna: ";
        std::cin >> colbin;
        preproc_remove_negatives(argv[2], output_file, colbin);
      }
      else if (flow == 36)
      {
        std::cout << "Vuoi usare per c il valore\n1: 1 (unita' naturali) oppure\n2: 299 792 458 m/s oppure\n3: 2.99792458e+10 cm/s?\n: ";
        while (changeC != 1 && changeC != 2 && changeC != 3)
        {
          std::cin >> changeC;
          if (changeC != 1 && changeC != 2 && changeC != 3)
          {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Non valido!\n: ";
          }
        }

        if (changeC == 1) C = 1.0;
        else if (changeC == 2) C = 299792458.0;
        else if (changeC == 3) C = 29979245800.0;
        else
        {
          std::cerr << "E' successo qualcosa di veramente grave con il valore di c" << std::endl;
          return -255;
        }
        std::cout << "Il valore usato di c e' " << C << std::endl;
        pzref = -1.;
        particleId = -1;
        conv_factor = -1.;
        frequency = -1.;
        ref_phase = -1.;
        convert_from_propaga_to_path(argv[2], output_file, C, pzref, particleId, conv_factor, frequency, ref_phase);
      }
      else if (flow == 37)
      {
        convert_from_path_to_propaga(argv[2], output_file);
      }
      else if (flow == 38)
      {
        C = 29979245800.0;
        std::cout << "Inserisci il valore minimo dell'energia in MeV. Se vuoi inserire il valore massimo, scrivilo come numero negativo.\n: ";
        std::cin >> meanE;
        std::cout << "dimmi se sono protoni (1) o elettroni (3) o se devo leggerlo da una colonna;" << std::endl;
        std::cout << "in tal caso, scrivere il numero della colonna preceduto dal segno -\n:";
        std::cin >> descriptor;
        preproc_energyCutAbove(argv[2], output_file, C, meanE, descriptor);
      }
      else if (flow == 39)
      {
        C = 1.0;
        std::cout << "Primo binnaggio: dimmi su quale colonna: ";
        std::cin >> colbin;
        std::cout << "Dimmi quanti bin fare: ";
        std::cin >> nbins;
        std::cout << "Dimmi il valore minimo: ";
        std::cin >> lowbin;
        std::cout << "Dimmi il valore massimo: ";
        std::cin >> highbin;
        std::cout << "Secondo binnaggio: dimmi su quale colonna: ";
        std::cin >> colbin2;
        std::cout << "Dimmi quanti bin fare: ";
        std::cin >> nbins2;
        std::cout << "Dimmi il valore minimo: ";
        std::cin >> angleMin; //riciclo la variabile angleMin/Max
        std::cout << "Dimmi il valore massimo: ";
        std::cin >> angleMax;
        std::cout << "Dimmi peso delle particelle (con un - davanti la colonna dove trovarlo): ";
        std::cin >> weight;
        postproc_bunchfile_binning_2D(argv[2], output_file, colbin, nbins, lowbin, highbin, colbin2, nbins2, angleMin, angleMax, weight);
      }
      else if (flow == 40)
      {
        C = 1.0;
        std::cout << "Binnaggio: dimmi su quale colonna: ";
        std::cin >> colbin;
        std::cout << "Dimmi quanti bin fare: ";
        std::cin >> nbins;
        std::cout << "Dimmi il valore minimo: ";
        std::cin >> lowbin;
        std::cout << "Dimmi il valore massimo: ";
        std::cin >> highbin;
        std::cout << "Dimmi peso delle particelle (con un - davanti la colonna dove trovarlo): ";
        std::cin >> weight;
        postproc_bunchfile_binning_1D(argv[2], output_file, colbin, nbins, lowbin, highbin, weight);
      }
      else if (flow == 41)
      {
        C = 29979245800.0;
        std::cout << "Dimmi la colonna dei px normalizzati: ";
        std::cin >> colbin; // riciclo variabile, nome variabile non affidabile per contenuto!
        std::cout << "Dimmi la colonna dei py normalizzati: ";
        std::cin >> nbins;  // riciclo variabile
        std::cout << "Dimmi la colonna dei pz normalizzati: ";
        std::cin >> colbin2;
        std::cout << "Dimmi la colonna nella quale si specifica il tipo di particella" << std::endl;
        std::cout << "Se nel file non e' presente questa colonna, puoi indicare il tipo da usare" << std::endl;
        std::cout << "per tutte le particelle mettendo -1 se sono protoni, -2 per antiprotoni," << std::endl;
        std::cout << "-3 per elettroni e -4 per antielettroni: ";
        std::cin >> nbins2;
        std::cout << "Dimmi la colonna del peso delle particelle: ";
        std::cout << "Se nel file non e' presente questa colonna, puoi indicare con -1 " << std::endl;
        std::cout << "di mettere un peso unitario per tutte le particelle: ";
        std::cin >> colweight;
        postproc_full_angle_and_energy(argv[2], output_file, C, colbin, nbins, colbin2, nbins2, colweight);
      }
      else if (flow == 42)
      {
        convert_from_jasmine(argv[2], output_file);
      }
      else if (flow == 43)
      {
        std::cout << "Dimmi la colonna del peso delle particelle: ";
        std::cin >> colweight;
        postproc_split_by_weight(argv[2], colweight);
      }
      else if (flow == 44)
      {
        preproc_nullify_transverse_momentum(argv[2], output_file);
      }
      else if (flow == 45)
      {
        C = 29979245800.0;
        std::cout << "Dimmi la colonna dei px normalizzati: ";
        std::cin >> colbin; // riciclo variabile, nome variabile non affidabile per contenuto!
        std::cout << "Dimmi la colonna dei py normalizzati: ";
        std::cin >> nbins;  // riciclo variabile
        std::cout << "Dimmi la colonna dei pz normalizzati: ";
        std::cin >> colbin2;
        std::cout << "Dimmi la colonna nella quale si specifica il tipo di particella" << std::endl;
        std::cout << "Se nel file non e' presente questa colonna, puoi indicare il tipo da usare" << std::endl;
        std::cout << "per tutte le particelle mettendo -1 se sono protoni, -2 per antiprotoni," << std::endl;
        std::cout << "-3 per elettroni e -4 per antielettroni: ";
        std::cin >> nbins2;
        postproc_xyzE(argv[2], output_file, C, colbin, nbins, colbin2, nbins2);
      }
      else if (flow == 46)
      {
        C = 29979245800.0;
        std::cout << "Dimmi la frequenza di riferimento: ";
        std::cin >> frequency;
        std::cout << "Dimmi la corrente di riferimento: ";
        std::cin >> ref_phase;  // riciclo variabile
        convert_from_propaga_to_binary_dst(argv[2], output_file, C, frequency, ref_phase);
      }
      else if (flow == 47)
      {
        C = 29979245800.0;
        convert_from_binary_dst_to_ascii(argv[2], output_file, C);
      }
      else if (flow == 48)
      {
        C = 29979245800.0;
        convert_from_binary_plt_to_ascii(argv[2], output_file, C);
      }
      else std::cout << "Scelta non valida!" << std::endl;
      break;

    case 1:
      if (argc < 5) { std::cout << "Non hai specificato correttamente i parametri di avvio" << std::endl; return -253; }
      sprintf(output_file, "conv.%s", argv[4]);
      sprintf(output_file2, "conv2.%s", argv[4]);

      C = argVdouble[2] = atof(argv[2]);
      argV[3] = atoi(argv[3]);
      std::cout << "Il valore usato di c e' " << C << std::endl;
      switch (argV[3])
      {
      case 1:
        std::cout << "Non usare file contenenti la particella lenta di riferimento! Nel caso, eliminarla prima di procedere!" << std::endl;
        convert_from_propaga_to_astra(argv[4], output_file, C);
        break;
      case 2:
        weight = atof(argv[5]);
        convert_from_astra_to_propaga(argv[4], output_file, C, weight);
        break;
      case 3:
        preproc_bunchfile_find_slowest(argv[4], output_file);
        break;
      case 4:
        postproc_removeParticlesGoneToInfinity(argv[4], output_file);
        break;
      case 8:
        from_ppg_to_vtk(argv[4], output_file);
        break;
      case 9:
        colstatus = atoi(argv[5]);
        postproc_removeLostParticles(argv[4], output_file, colstatus);
        break;
      case 15:
        jump = atoi(argv[5]);
        preproc_subsample(argv[4], output_file, jump);
        break;
      case 16:
        preproc_from_micron_to_cm(argv[4], output_file);
        break;
      case 20:
        angleMin = atof(argv[5]);
        angleMax = atof(argv[6]);
        if (angleMin < 0.0 || angleMax < angleMin) { std::cout << "Non hai specificato correttamente angleMin e angleMax" << std::endl; return -211; }
        preproc_angularCut(argv[4], output_file, angleMin, angleMax);
        break;
      case 25:
        preproc_from_NewP_to_OldP(argv[4], output_file);
        break;
      case 26:
        pzref = atof(argv[5]);
        convert_from_propaga_to_astra_fixed(argv[4], output_file, C, pzref);
      case 28:
        std::cout << "Se non hai messo C = 29979245800 (cm/s) occhio che forse non funziona!" << std::endl;
        meanE = atof(argv[5]);
        deltaE = atof(argv[6]);
        descriptor = atoi(argv[7]);
        if (meanE <= 0.0 || deltaE <= 0.0) { std::cout << "Non hai specificato correttamente meanE e deltaE" << std::endl; return -211; }
        preproc_energyCut(argv[4], output_file, C, meanE, deltaE, descriptor);
        break;
      case 29:
        weight = atof(argv[5]);
        convert_from_fluka_to_propaga(argv[4], output_file, weight);
        break;
      case 30:
        postproc_split_p_and_e(argv[4], output_file, output_file2);
        break;
      case 31:
        particleId = atoi(argv[5]);
        weight = atof(argv[6]);
        colbin = atoi(argv[7]);
        if (particleId != 1 && particleId != 3) { std::cout << "Occhio che per ora sono noti solo protoni (1) ed elettroni (3)" << std::endl; }
        if (colbin != 1 && colbin != 0) { std::cout << "Non hai specificato se sia necessario invertire la colonna x con z" << std::endl; }
        preproc_from_OldP_to_NewP(argv[4], output_file, particleId, weight, colbin);
        break;
      case 33:
        colbin = atoi(argv[5]);
        postproc_bunchfile_find_minmax(argv[4], output_file, colbin);
        break;
      case 35:
        colbin = argV[5] = atoi(argv[5]);
        preproc_remove_negatives(argv[4], output_file, colbin);
        break;
      case 36:
        pzref = atof(argv[5]);
        particleId = atoi(argv[6]);
        conv_factor = atof(argv[7]);
        frequency = atof(argv[8]);
        ref_phase = atof(argv[9]);
        convert_from_propaga_to_path(argv[4], output_file, C, pzref, particleId, conv_factor, frequency, ref_phase);
        break;
      case 37:
        convert_from_path_to_propaga(argv[4], output_file);
        break;
      case 38:
        std::cout << "Se non hai messo C = 29979245800 (cm/s) occhio che forse non funziona!" << std::endl;
        meanE = atof(argv[5]);
        descriptor = atoi(argv[6]);
        if (descriptor != 1 && descriptor != 2 && descriptor != 3 && descriptor != 4) { std::cout << "Descrittore particella non implementato" << std::endl; return -211; }
        preproc_energyCutAbove(argv[4], output_file, C, meanE, descriptor);
        break;
      case 39:
        colbin = atoi(argv[5]);
        nbins = atoi(argv[6]);
        lowbin = atof(argv[7]);
        highbin = atof(argv[8]);
        colbin2 = atoi(argv[9]);
        nbins2 = atoi(argv[10]);
        angleMin = atof(argv[11]);  //riciclo la variabile angleMin/Max
        angleMax = atof(argv[12]);
        weight = atof(argv[13]);
        postproc_bunchfile_binning_2D(argv[4], output_file, colbin, nbins, lowbin, highbin, colbin2, nbins2, angleMin, angleMax, weight);
        break;
      case 40:
        colbin = atoi(argv[5]);
        nbins = atoi(argv[6]);
        lowbin = atof(argv[7]);
        highbin = atof(argv[8]);
        weight = atof(argv[9]);
        postproc_bunchfile_binning_1D(argv[4], output_file, colbin, nbins, lowbin, highbin, weight);
        break;
      case 41:
        colbin = atoi(argv[5]);   // colonna px // variabili riciclate, non c'entrano niente col loro significato
        nbins = atoi(argv[6]);    // colonna py
        colbin2 = atoi(argv[7]);  // colonna pz
        nbins2 = atoi(argv[8]);   // colonna tipo particelle (negativo lo imposta manualmente a tutte)
        colweight = atoi(argv[9]);  // colonna peso particelle
        postproc_full_angle_and_energy(argv[4], output_file, C, colbin, nbins, colbin2, nbins2, colweight);
        break;
      case 42:
        convert_from_jasmine(argv[4], output_file);
        break;
      case 43:
        colweight = atoi(argv[5]);  // colonna peso particelle
        postproc_split_by_weight(argv[4], colweight);
        break;
      case 44:
        preproc_nullify_transverse_momentum(argv[4], output_file);
        break;
      case 45:
        colbin = atoi(argv[5]);   // colonna px // variabili riciclate, non c'entrano niente col loro significato
        nbins = atoi(argv[6]);    // colonna py
        colbin2 = atoi(argv[7]);  // colonna pz
        nbins2 = atoi(argv[8]);   // colonna tipo particelle (negativo lo imposta manualmente a tutte)
        postproc_xyzE(argv[4], output_file, C, colbin, nbins, colbin2, nbins2);
        break;
        case 46:
        C = 29979245800.0;
        frequency = atof(argv[5]);
        ref_phase = atof(argv[6]);  // riciclo variabile
        convert_from_propaga_to_binary_dst(argv[4], output_file, C, frequency, ref_phase);
        break;
      case 47:
        C = 29979245800.0;
        convert_from_binary_dst_to_ascii(argv[4], output_file, C);
        break;
      case 48:
        C = 29979245800.0;
        convert_from_binary_plt_to_ascii(argv[4], output_file, C);
        break;
      default:
        std::cout << "Modo automatico non valido" << std::endl;
        break;
      }
      break;


    default:
      std::cout << "Modo commandline non valido" << std::endl;
      break;
    }
    return 0;
  }
}

