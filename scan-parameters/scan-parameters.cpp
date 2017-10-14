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
#include "jsoncons/json.hpp"


class Propaga_parameters {
public:
  Propaga_parameters();
  unsigned int nx, ny, nz, ny_targ;
};


Propaga_parameters::Propaga_parameters() {
  nx = 256, ny = 128, nz = 1, ny_targ = 120;
}

void parse_json_file(Propaga_parameters& parameters, const char * filename) {
  jsoncons::json json_parameters = jsoncons::json::parse_file(filename);

  if (json_parameters.has_member("nx")) parameters.nx = json_parameters["nx"].as<unsigned int>();
  else std::cout << "Missing nx definition, default to " << parameters.nx << "\n";

  if (json_parameters.has_member("ny")) parameters.ny = json_parameters["ny"].as<unsigned int>();
  else std::cout << "Missing ny definition, default to " << parameters.ny << "\n";

  if (json_parameters.has_member("nz")) parameters.nz = json_parameters["nz"].as<unsigned int>();
  else std::cout << "Missing nz definition, default to " << parameters.nz << "\n";

  if (json_parameters.has_member("ny_targ")) parameters.ny_targ = json_parameters["ny_targ"].as<unsigned int>();
  else std::cout << "Missing ny_targ definition, default to " << parameters.ny_targ << "\n";

}


int main()
{
  Propaga_parameters parametri;

  parse_json_file(parametri, "input.json");

  std::string inputnml = "input.nml";

  print_input_nml(parametri, inputnml);
  return 0;
}


