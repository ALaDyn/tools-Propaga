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

  struct LElement {
    char type;
    double begin;
    double end;
    double par_01, par_02, par_03, par_04;
  };

  jsoncons::json parameters;

  std::vector<LElement> lattice_elements;

  std::ifstream In_dist;
  std::ofstream log_file, track_file, outz_file, minmax_file, emitt_file;

  std::string input_parameters_filename;
  std::string input_dist_filename;
  std::string scan_name;
  std::ostringstream track_filename, outz_filename, minmax_filename, emitt_filename;

  bool restart;
  int restart_step;
  int step_to_be_dumped;
  bool enable_reference_particle;
  double eMin_json, eMax_json, z_dump;
  int MPI_Size, MPI_Rank, err_check;
  int tag_max;
  int ntrack;
  
  int big_coutta;
  int diag_coutta;

  int step;
  double sim_time;
  double dt;

  void parse_json_file();
  void print_scan_input_files();
  Propaga_parameters();
};



Propaga_parameters::Propaga_parameters() {
}




void Propaga_parameters::parse_json_file() {
  try {
    parameters = jsoncons::json::parse_file(input_parameters_filename);
  }
  catch (std::exception &e) {
    log_file << e.what() << std::endl;
  }
  jsoncons::json empty_json;
  jsoncons::json json_lattice_elements = parameters.has_member("Magnetic_elements") ? parameters["Magnetic_elements"] : empty_json;

  eMin_json = parameters.has_member("emin") ? parameters["emin"].as<double>() : -1.0;
  eMax_json = parameters.has_member("emax") ? parameters["emax"].as<double>() : -1.0;
  dt = parameters.has_member("dt") ? parameters["dt"].as<double>() : 0.1;
  big_coutta = parameters.has_member("steps_between_dumps") ? parameters["steps_between_dumps"].as<int>() : 0;
  diag_coutta = parameters.has_member("steps_between_diags") ? parameters["steps_between_diags"].as<int>() : 50;

  for (auto it = json_lattice_elements.begin_elements(); it != json_lattice_elements.end_elements(); ++it) {
    LElement new_element;
    jsoncons::json& json_lattice_element = *it;

    new_element.type = json_lattice_element.has_member("type") ? json_lattice_element["type"].as<char>() : 'O';
    new_element.begin = json_lattice_element.has_member("begin") ? json_lattice_element["begin"].as<double>() : 0.0;
    new_element.end = json_lattice_element.has_member("end") ? json_lattice_element["end"].as<double>() : 0.0;
    new_element.par_01 = json_lattice_element.has_member("par_01") ? json_lattice_element["par_01"].as<double>() : 0.0;
    new_element.par_02 = json_lattice_element.has_member("par_02") ? json_lattice_element["par_02"].as<double>() : 0.0;
    new_element.par_03 = json_lattice_element.has_member("par_03") ? json_lattice_element["par_03"].as<double>() : 0.0;
    new_element.par_04 = json_lattice_element.has_member("par_04") ? json_lattice_element["par_04"].as<double>() : 0.0;
    lattice_elements.push_back(new_element);
  }

  if (eMin_json < 0.0) eMin_json = 0.0;
  if (eMax_json < 0.0) eMax_json = 1.0e+100;

  restart = false;
  restart_step = 0;
  step_to_be_dumped = -1;
  enable_reference_particle = false;
  ntrack = 0;
  z_dump = -1.0;
  MPI_Size = 0, MPI_Rank = 0, err_check = 0;
  tag_max = 1024 * 1024 + 1;  // unsafe

  step = 0;
  sim_time = 0.0;

}



void Propaga_parameters::print_scan_input_files() {
}



int main()
{
  Propaga_parameters parametri;
  parametri.input_parameters_filename = "input.json";
  parametri.scan_name = "test";
  parametri.input_dist_filename = parametri.scan_name + ".initialbunch.ppg";

  parametri.parse_json_file();
  parametri.print_scan_input_files();
  return 0;
}


