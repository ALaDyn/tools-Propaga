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



#include <cstdlib> 
#include <cmath> 
#include <ctime> 
#include <iostream> 
#include <iomanip>
#include <fstream> 
#include <limits>
#include "NumberGenerator.h" 

#define ALIVE_ID_FLAG 0

int main(int argc, char*argv[])
{
  int packet_type = 0;
  int i = 0, discarded = 0;
  NumberGenerator rand1;
  double packet_length = -1.0, packet_radius = -1.0, packet_openingAngle = -1.0, packet_radius2, packet_openingAngle2;
  int packet_numberOfParticles = -1;
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " output_file" << std::endl;
    return 255;
  }
  std::ofstream packet(argv[1], std::ios::app);
  double x, y, z, x_prime, y_prime, px, py, pz;

  double min_p = 0.0, max_p = 0.0;
  int selection_type = -1;
  double mean_kinetic_energy = -1.0, delta_kinetic_energy = -1.0;
  double mean_gammabeta = -1.0, delta_gammabeta = -1.0;
  double mean_p = -1.0, delta_p = -1.0;

  double massa = -1.0;
  int tipo_particella = 0;
  double particle_weight = 1.0;

  std::cout << "Protons (1), user-defined (2) or electrons (3) ?\n: ";  // this numbering scheme is chosen just to comply with Fluka/Propaga, where 1=protons, 3=electrons
  while (tipo_particella != 1 && tipo_particella != 2 && tipo_particella != 3)
  {
    std::cin >> tipo_particella;
    if (tipo_particella != 1 && tipo_particella != 2 && tipo_particella != 3)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  if (tipo_particella == 1)       massa = MP_MEV;
  else if (tipo_particella == 2)
  {
    std::cout << "Give me the particle mass [MeV/c^2]: ";
    while (massa < ONE_eV_IN_MeV)
    {
      std::cin >> massa;
      if (massa < ONE_eV_IN_MeV)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    }
  }
  else if (tipo_particella == 3)  massa = ME_MEV;

  double massa_inversa = 1. / massa;


  std::cout << "Bunch distribution?\n"
    << "1 - x: uniform  - px: uniform  (integers!)               \n"
    << "2 - x: uniform  - px: uniform                            \n"
    << "3 - x: uniform  - px: gaussian                           \n"
    << "4 - x: gaussian - px: gaussian                           \n"
    << "5 - x/x', y/y' uniform, with a hole in the x/x' distrib  \n"
    << ": ";

  while (packet_type != 1 && packet_type != 2 && packet_type != 3 && packet_type != 4 && packet_type != 5)
  {
    std::cin >> packet_type;
    if (packet_type != 1 && packet_type != 2 && packet_type != 3 && packet_type != 4 && packet_type != 5)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  std::cout << "Dimmi la lunghezza in z del pacchetto (in micrometri): ";
  while (packet_length < 0.)
  {
    std::cin >> packet_length;
    if (packet_length < 0.)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  std::cout << "Dimmi il raggio del pacchetto (in micrometri): ";
  while (packet_radius < 0.)
  {
    std::cin >> packet_radius;
    if (packet_radius < 0.)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  std::cout << "Dimmi quante particelle vuoi mettere nel pacchetto: ";
  while (packet_numberOfParticles < 0)
  {
    std::cin >> packet_numberOfParticles;
    if (packet_numberOfParticles < 0)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  std::cout << "Vuoi selezionare in energia (1), in momento (2) o in gamma*beta (3)? : ";
  while (selection_type != 1 && selection_type != 2 && selection_type != 3)
  {
    std::cin >> selection_type;
    if (selection_type != 1 && selection_type != 2 && selection_type != 3)
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  if (selection_type == 1)  // leggo l'energia e poi la converto in p
  {
    std::cout << "Dimmi l'energia cinetica media delle particelle (in MeV): ";
    while (mean_kinetic_energy < 0.)
    {
      std::cin >> mean_kinetic_energy;
      if (mean_kinetic_energy < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    }

    std::cout << "Dimmi lo spread in MeV dell'energia cinetica delle particelle: ";
    while (delta_kinetic_energy < 0. || (mean_kinetic_energy - delta_kinetic_energy) < 0.)
    {
      std::cin >> delta_kinetic_energy;
      if (delta_kinetic_energy < 0. || (mean_kinetic_energy - delta_kinetic_energy) < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    }
    mean_p = sqrt(mean_kinetic_energy*mean_kinetic_energy + 2 * mean_kinetic_energy*massa);
    delta_p = sqrt(delta_kinetic_energy*delta_kinetic_energy + 2 * delta_kinetic_energy*massa);
  }
  else if (selection_type == 3)   // leggo il gamma*beta e lo converto in p ± delta_p
  {
    std::cout << "Dimmi il gamma*beta medio delle particelle: ";
    while (mean_gammabeta < 0.)
    {
      std::cin >> mean_gammabeta;
      if (mean_gammabeta < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!!\n: ";
      }
    }

    std::cout << "Dimmi lo spread del gamma*beta delle particelle: ";
    while (delta_gammabeta < 0. || (mean_gammabeta - delta_gammabeta) < 0.)
    {
      std::cin >> delta_gammabeta;
      if (delta_gammabeta < 0. || (mean_gammabeta - delta_gammabeta) < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
      mean_p = mean_gammabeta * massa;
      delta_p = delta_gammabeta * massa;
    }
  }
  else
  {
    std::cout << "Dimmi il momento medio delle particelle (in MeV/c): ";
    while (mean_p < 0.)
    {
      std::cin >> mean_p;
      if (mean_p < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    }

    std::cout << "Dimmi lo spread in MeV/c del momento delle particelle: ";
    while (delta_p < 0. || (mean_p - delta_p) < 0.)
    {
      std::cin >> delta_p;
      if (delta_p < 0. || (mean_p - delta_p) < 0.)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    }
  }


  std::cout << "Dimmi l'angolo di apertura (in radianti, tra 0 e pi/2): ";
  while (packet_openingAngle < 0 || packet_openingAngle >(0.5*M_PI))
  {
    std::cin >> packet_openingAngle;
    if (packet_openingAngle < 0 || packet_openingAngle >(0.5*M_PI))
    {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid!\n: ";
    }
  }

  packet_openingAngle2 = 1.0 / (packet_openingAngle*packet_openingAngle);
  packet_radius2 = 1.0 / (packet_radius*packet_radius);


  switch (packet_type)
  {
  case 1:
    std::cout << "Verra' ora generato un pacchetto (int) uniforme nelle posizioni e nei momenti" << std::endl;

    while (i < packet_numberOfParticles)
    {
      x = rand1.getUniformFloat(-packet_radius, packet_radius);
      y = rand1.getUniformFloat(-packet_radius, packet_radius);
      z = rand1.getUniformFloat(0.0, packet_length);

      pz = rand1.getUniformFloat(0, max_p);

      if (packet_openingAngle < 0.0 || packet_openingAngle > 0.0) // floating-point equality and inequality comparisons are unreliable so 'packet_openingAngle !=0' is best done in this way
      {
        x_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        px = pz * tan(x_prime);
        y_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        py = pz * tan(y_prime);
      }
      else
      {
        x_prime = 0.0;
        px = 0.0;
        y_prime = 0.0;
        py = 0.0;
      }

      if ((x*x + y*y) <= packet_radius*packet_radius
        && (px*px + py*py + pz*pz) <= max_p*max_p
        && (px*px + py*py + pz*pz) >= min_p*min_p
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 <= 1.0
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 >= 0.95
#ifdef USE_STRINGENT_CHECK
        && gamma*x*x + 2 * alpha*x*x_prime + beta*x_prime*x_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*y_prime + beta*y_prime*y_prime <= epsilon
        && packet_openingAngle*packet_openingAngle*y*y + packet_radius*packet_radius*x_prime*x_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && packet_openingAngle*packet_openingAngle*x*x + packet_radius*packet_radius*y_prime*y_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && gamma*x*x + 2 * alpha*x*y_prime + beta*y_prime*y_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*x_prime + beta*x_prime*x_prime <= epsilon
        && (x_prime / packet_openingAngle)*(x_prime / packet_openingAngle) + (y_prime / packet_openingAngle)*(y_prime / packet_openingAngle) <= 1
        && x_prime <= packet_openingAngle && x_prime >= -packet_openingAngle
        && y_prime <= packet_openingAngle && y_prime >= -packet_openingAngle
#endif
        && sqrt(x_prime*x_prime + y_prime*y_prime) <= packet_openingAngle) {
        packet << (int)x << "\t" << (int)y << "\t" << (int)z << "\t" << (int)px*massa_inversa << "\t" << (int)py*massa_inversa << "\t" << (int)pz*massa_inversa
          << "\t" << tipo_particella << "\t" << particle_weight << "\t" << ALIVE_ID_FLAG << "\t" << i++ << std::endl;
      }
      else discarded++;

      if (discarded % 100000 == 0) std::cout << "Scartate finora " << discarded << " particelle e " << i << " buone" << std::endl;
    }

    break;


  case 2:
    std::cout << "Verra' ora generato un pacchetto (float) uniforme nelle posizioni e nei momenti" << std::endl;
    min_p = mean_p - delta_p;
    if (min_p < 0)
    {
      min_p = 0;
      std::cout << "Energia minima limitata a zero" << std::endl;
    }
    max_p = mean_p + delta_p;


    while (i < packet_numberOfParticles)
    {
      x = rand1.getUniformFloat(-packet_radius, packet_radius);
      y = rand1.getUniformFloat(-packet_radius, packet_radius);
      z = rand1.getUniformFloat(0.0, packet_length);

      pz = rand1.getUniformFloat(0, max_p);

      if (packet_openingAngle < 0.0 || packet_openingAngle > 0.0)
      {
        x_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        px = pz * tan(x_prime);
        y_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        py = pz * tan(y_prime);
      }
      else
      {
        x_prime = 0.0;
        px = 0.0;
        y_prime = 0.0;
        py = 0.0;
      }


      if ((x*x + y*y) <= packet_radius*packet_radius
        && (px*px + py*py + pz*pz) <= max_p*max_p
        && (px*px + py*py + pz*pz) >= min_p*min_p
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 <= 1.0
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 >= 0.95
#ifdef USE_STRINGENT_CHECK
        && gamma*x*x + 2 * alpha*x*x_prime + beta*x_prime*x_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*y_prime + beta*y_prime*y_prime <= epsilon
        && packet_openingAngle*packet_openingAngle*y*y + packet_radius*packet_radius*x_prime*x_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && packet_openingAngle*packet_openingAngle*x*x + packet_radius*packet_radius*y_prime*y_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && gamma*x*x + 2 * alpha*x*y_prime + beta*y_prime*y_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*x_prime + beta*x_prime*x_prime <= epsilon
        && (x_prime / packet_openingAngle)*(x_prime / packet_openingAngle) + (y_prime / packet_openingAngle)*(y_prime / packet_openingAngle) <= 1
        && x_prime <= packet_openingAngle && x_prime >= -packet_openingAngle
        && y_prime <= packet_openingAngle && y_prime >= -packet_openingAngle
#endif
        && sqrt(x_prime*x_prime + y_prime*y_prime) <= packet_openingAngle) {
        packet << std::setprecision(8) << x*1E-4 << "\t" << y*1E-4 << "\t" << z*1E-4 << "\t" << px*massa_inversa << "\t" << py*massa_inversa << "\t" << pz*massa_inversa
          << "\t" << tipo_particella << "\t" << particle_weight << "\t" << ALIVE_ID_FLAG << "\t" << i++ << std::endl;
      }
      else discarded++;

      if (discarded % 100000 == 0) std::cout << "Scartate finora " << discarded << " particelle e " << i << " buone" << std::endl;
    }


    break;


  case 3:
    std::cout << "Verra' ora generato un pacchetto (float) uniforme nelle posizioni e gaussiano nei momenti" << std::endl;

    while (i < packet_numberOfParticles)
    {
      x = rand1.getUniformFloat(-packet_radius, packet_radius);
      y = rand1.getUniformFloat(-packet_radius, packet_radius);
      z = rand1.getUniformFloat(0.0, packet_length);

      pz = rand1.getGaussianFloat(mean_p, delta_p);

      if (packet_openingAngle < 0.0 || packet_openingAngle > 0.0)
      {
        x_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        px = pz * tan(x_prime);
        y_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        py = pz * tan(y_prime);
      }
      else
      {
        x_prime = 0.0;
        px = 0.0;
        y_prime = 0.0;
        py = 0.0;
      }

      if ((x*x + y*y) <= packet_radius*packet_radius
        && (px*px + py*py + pz*pz) <= max_p*max_p
        && (px*px + py*py + pz*pz) >= min_p*min_p
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 <= 1.0
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 >= 0.95
#ifdef USE_STRINGENT_CHECK
        && gamma*x*x + 2 * alpha*x*x_prime + beta*x_prime*x_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*y_prime + beta*y_prime*y_prime <= epsilon
        && packet_openingAngle*packet_openingAngle*y*y + packet_radius*packet_radius*x_prime*x_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && packet_openingAngle*packet_openingAngle*x*x + packet_radius*packet_radius*y_prime*y_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && gamma*x*x + 2 * alpha*x*y_prime + beta*y_prime*y_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*x_prime + beta*x_prime*x_prime <= epsilon
        && (x_prime / packet_openingAngle)*(x_prime / packet_openingAngle) + (y_prime / packet_openingAngle)*(y_prime / packet_openingAngle) <= 1
        && x_prime <= packet_openingAngle && x_prime >= -packet_openingAngle
        && y_prime <= packet_openingAngle && y_prime >= -packet_openingAngle
#endif
        && sqrt(x_prime*x_prime + y_prime*y_prime) <= packet_openingAngle) {
        packet << std::setprecision(8) << x*1E-4 << "\t" << y*1E-4 << "\t" << z*1E-4 << "\t" << px*massa_inversa << "\t" << py*massa_inversa << "\t" << pz*massa_inversa
          << "\t" << tipo_particella << "\t" << particle_weight << "\t" << ALIVE_ID_FLAG << "\t" << i++ << std::endl;
      }
      else discarded++;

      if (discarded % 100000 == 0) std::cout << "Scartate finora " << discarded << " particelle e " << i << " buone" << std::endl;
    }

    break;




  case 4:
    std::cout << "Verra' ora generato un pacchetto (float) gaussiano nelle posizioni e nei momenti" << std::endl;

    while (i < packet_numberOfParticles)
    {
      x = rand1.getGaussianFloat(0.0, (0.60*packet_radius));
      y = rand1.getGaussianFloat(0.0, (0.60*packet_radius));
      z = rand1.getGaussianFloat(0.0, (0.60*packet_length));

      pz = rand1.getGaussianFloat(mean_p, delta_p);

      if (packet_openingAngle < 0.0 || packet_openingAngle > 0.0)
      {
        x_prime = rand1.getGaussianFloat(0.0, (0.60*packet_openingAngle));
        px = pz * tan(x_prime);
        y_prime = rand1.getGaussianFloat(0.0, (0.60*packet_openingAngle));
        py = pz * tan(y_prime);
      }
      else
      {
        x_prime = 0.0;
        px = 0.0;
        y_prime = 0.0;
        py = 0.0;
      }

      if ((x*x + y*y) <= packet_radius*packet_radius
        && (px*px + py*py + pz*pz) <= max_p*max_p
        && (px*px + py*py + pz*pz) >= min_p*min_p
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 <= 1.0
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 >= 0.95
#ifdef USE_STRINGENT_CHECK
        && gamma*x*x + 2 * alpha*x*x_prime + beta*x_prime*x_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*y_prime + beta*y_prime*y_prime <= epsilon
        && packet_openingAngle*packet_openingAngle*y*y + packet_radius*packet_radius*x_prime*x_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && packet_openingAngle*packet_openingAngle*x*x + packet_radius*packet_radius*y_prime*y_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && gamma*x*x + 2 * alpha*x*y_prime + beta*y_prime*y_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*x_prime + beta*x_prime*x_prime <= epsilon
        && (x_prime / packet_openingAngle)*(x_prime / packet_openingAngle) + (y_prime / packet_openingAngle)*(y_prime / packet_openingAngle) <= 1
        && x_prime <= packet_openingAngle && x_prime >= -packet_openingAngle
        && y_prime <= packet_openingAngle && y_prime >= -packet_openingAngle
#endif
        && sqrt(x_prime*x_prime + y_prime*y_prime) <= packet_openingAngle) {
        packet << std::setprecision(8) << x*1E-4 << "\t" << y*1E-4 << "\t" << z*1E-4 << "\t" << px*massa_inversa << "\t" << py*massa_inversa << "\t" << pz*massa_inversa
          << "\t" << tipo_particella << "\t" << particle_weight << "\t" << ALIVE_ID_FLAG << "\t" << i++ << std::endl;
      }
      else discarded++;

      if (discarded % 100000 == 0) std::cout << "Scartate finora " << discarded << " particelle e " << i << " buone" << std::endl;
    }

    break;


  case 5:
    double hole_x, hole_px;
    do
    {
      std::cout << "Dammi il raggio in x del buco (in micrometri): ";
      std::cin >> hole_x;
      if (packet_radius < hole_x)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    } while (packet_openingAngle < hole_x);
    do
    {
      std::cout << "Dammi l'apertura in x' del buco (in radianti): ";
      std::cin >> hole_px;
      if (packet_openingAngle < hole_px)
      {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid!\n: ";
      }
    } while (packet_radius < hole_px);


    std::cout << "Verra' ora generato un pacchetto K-V \"bucato\" nello spazio x-x'" << std::endl;
    min_p = mean_p - delta_p;
    if (min_p < 0)
    {
      min_p = 0;
      std::cout << "Energia minima limitata a zero" << std::endl;
    }
    max_p = mean_p + delta_p;


    while (i < packet_numberOfParticles)
    {
      x = rand1.getUniformFloat(-packet_radius, packet_radius);
      y = rand1.getUniformFloat(-packet_radius, packet_radius);
      z = rand1.getUniformFloat(0.0, packet_length);

      pz = rand1.getUniformFloat(0, max_p);

      if (packet_openingAngle < 0.0 || packet_openingAngle > 0.0)
      {
        x_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        px = pz * tan(x_prime);
        y_prime = rand1.getUniformFloat(-packet_openingAngle, packet_openingAngle);
        py = pz * tan(y_prime);
      }
      else
      {
        x_prime = 0.0;
        px = 0.0;
        y_prime = 0.0;
        py = 0.0;
      }

      double cerchio_x = x / packet_radius;
      double cerchio_px = x_prime / packet_openingAngle;
      double cerchio_x_max = hole_x / packet_radius;
      double cerchio_px_max = hole_px / packet_openingAngle;
      if ((cerchio_x*cerchio_x + cerchio_px*cerchio_px) <= (cerchio_x_max*cerchio_x_max + cerchio_px_max*cerchio_px_max)) continue;

      if ((x*x + y*y) <= packet_radius*packet_radius
        && (px*px + py*py + pz*pz) <= max_p*max_p
        && (px*px + py*py + pz*pz) >= min_p*min_p
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 <= 1.0
        && x*x*packet_radius2 + x_prime*x_prime*packet_openingAngle2 + y*y*packet_radius2 + y_prime*y_prime*packet_openingAngle2 >= 0.95
#ifdef USE_STRINGENT_CHECK
        && gamma*x*x + 2 * alpha*x*x_prime + beta*x_prime*x_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*y_prime + beta*y_prime*y_prime <= epsilon
        && packet_openingAngle*packet_openingAngle*y*y + packet_radius*packet_radius*x_prime*x_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && packet_openingAngle*packet_openingAngle*x*x + packet_radius*packet_radius*y_prime*y_prime <= packet_openingAngle*packet_openingAngle*packet_radius*packet_radius
        && gamma*x*x + 2 * alpha*x*y_prime + beta*y_prime*y_prime <= epsilon
        && gamma*y*y + 2 * alpha*y*x_prime + beta*x_prime*x_prime <= epsilon
        && (x_prime / packet_openingAngle)*(x_prime / packet_openingAngle) + (y_prime / packet_openingAngle)*(y_prime / packet_openingAngle) <= 1
        && x_prime <= packet_openingAngle && x_prime >= -packet_openingAngle
        && y_prime <= packet_openingAngle && y_prime >= -packet_openingAngle
#endif
        && sqrt(x_prime*x_prime + y_prime*y_prime) <= packet_openingAngle) {
        packet << std::setprecision(8) << x*1E-4 << "\t" << y*1E-4 << "\t" << z*1E-4 << "\t" << px*massa_inversa << "\t" << py*massa_inversa << "\t" << pz*massa_inversa
          << "\t" << tipo_particella << "\t" << particle_weight << "\t" << ALIVE_ID_FLAG << "\t" << i++ << std::endl;
      }
      else discarded++;

      if (discarded % 100000 == 0) std::cout << "Scartate finora " << discarded << " particelle e " << i << " buone" << std::endl;
    }


    break;


  default:
    std::cout << "Something really wrong happened!" << std::endl;
    break;
  }


  std::cout << "\n\nGenerazione pacchetto completata con successo!\nNel processo ho scartato " << discarded << " particelle" << std::endl;

  return 0;
}

