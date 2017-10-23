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
#include "physycom/combexplosion.hpp"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

#define number_of_parameters_per_magnetic_element 6

void my_exit(int exit_code) {
  std::cerr << "Exiting... code: " << exit_code << std::endl << std::flush;
  exit(exit_code);
}

void urgent_message(std::string message) {
  std::cout << message << std::endl << std::flush;
}

std::vector<double> to_double(std::vector<std::string> all)
{
	std::vector<std::string> strng;
	std::vector<double> dble;
	for(const auto &seq : all)
	{
		boost::split(strng, seq, boost::is_any_of(separator), boost::token_compress_off);
		for( auto &val : strng) dble.push_back(stoanything<double>(val));
	}
	return dble;
}


struct LElement {
  std::string type;
  std::vector<double> begin_values;
  std::vector<double> begin_values_exploded;
  std::vector<double> length_values;
  std::vector<double> length_values_exploded;
  //std::vector<double> end_values;
  //std::vector<double> end_values_exploded;
  std::vector<double> par_01_values, par_02_values, par_03_values, par_04_values;
  std::vector<double> par_01_values_exploded, par_02_values_exploded, par_03_values_exploded, par_04_values_exploded;
};


class Propaga_parameters {
public:

  jsoncons::json parameters;
  std::string input_parameters_filename;
  std::string scan_name;
  std::string input_dist_filename;

  double emin, emax;
  double dt;
  int big_coutta, diag_coutta;
  std::vector<LElement> magnetic_elements;
  std::vector<std::string> scan;

  void parse_json_file();
  void print_scan_input_files();
  void debug_scan();
};



void Propaga_parameters::parse_json_file() {
  try {
    parameters = jsoncons::json::parse_file(input_parameters_filename);
  }
  catch (std::exception &e) {
    std::cerr << "Parsing file exception: " << e.what() << std::endl;
    my_exit(-1);
  }
  jsoncons::json empty_json;

  try {
    emin = parameters.has_member("emin") ? parameters["emin"].as<double>() : 0.0;
    emax = parameters.has_member("emax") ? parameters["emax"].as<double>() : 1.0e+100;
    dt = parameters.has_member("dt") ? parameters["dt"].as<double>() : 0.01;
    big_coutta = parameters.has_member("steps_between_dumps") ? parameters["steps_between_dumps"].as<int>() : 0;
    diag_coutta = parameters.has_member("steps_between_diags") ? parameters["steps_between_diags"].as<int>() : 50;
  }
  catch (std::exception &e) {
    std::cerr << "Basic parameters exception: " << e.what() << std::endl;
    my_exit(-1);
  }

  jsoncons::json lattice_elements = parameters.has_member("Magnetic_elements") ? parameters["Magnetic_elements"] : empty_json;

  for (const auto& lattice_element : lattice_elements.array_range()) {
    LElement new_element;

    try {
      new_element.type = lattice_element.has_member("type") ? lattice_element["type"].as<std::string>() : "O";
      jsoncons::json begin_array = lattice_element.has_member("begin_range") ? lattice_element["begin_range"] : empty_json;
      for (const auto& begin_data : begin_array.array_range()) {
        new_element.begin_values.push_back(begin_data.as_double());
      }
      /*
      jsoncons::json end_array = lattice_element.has_member("end_range") ? lattice_element["end_range"] : empty_json;
      for (const auto& end_data : end_array.array_range()) {
        new_element.end_values.push_back(end_data.as_double());
      }
      */
      jsoncons::json length_array = lattice_element.has_member("length_range") ? lattice_element["length_range"] : empty_json;
      for (const auto& length_data : length_array.array_range()) {
        new_element.length_values.push_back(length_data.as_double());
      }
      jsoncons::json par_01_array = lattice_element.has_member("par_01_range") ? lattice_element["par_01_range"] : empty_json;
      for (const auto& par_01_data : par_01_array.array_range()) {
        new_element.par_01_values.push_back(par_01_data.as_double());
      }
      jsoncons::json par_02_array = lattice_element.has_member("par_02_range") ? lattice_element["par_02_range"] : empty_json;
      for (const auto& par_02_data : par_02_array.array_range()) {
        new_element.par_02_values.push_back(par_02_data.as_double());
      }
      jsoncons::json par_03_array = lattice_element.has_member("par_03_range") ? lattice_element["par_03_range"] : empty_json;
      for (const auto& par_03_data : par_03_array.array_range()) {
        new_element.par_03_values.push_back(par_03_data.as_double());
      }
      jsoncons::json par_04_array = lattice_element.has_member("par_04_range") ? lattice_element["par_04_range"] : empty_json;
      for (const auto& par_04_data : par_04_array.array_range()) {
        new_element.par_04_values.push_back(par_04_data.as_double());
      }

      /*
      if (new_element.begin_values.size() != 3 || new_element.end_values.size() != 3 || new_element.par_01_values.size() != 3) {
        std::cerr << "Missing scan values!" << std::endl;
        exit(-1);
      }
      */

      if (new_element.begin_values.size() != 3 || new_element.length_values.size() != 3 || new_element.par_01_values.size() != 3) {
        std::cerr << "Missing scan values!" << std::endl;
        exit(-1);
      }

      if (new_element.begin_values.back() <= 0.) new_element.begin_values.back() = 1.0;
      for (double exploded_value = new_element.begin_values[0]; exploded_value <= new_element.begin_values[1]; exploded_value += new_element.begin_values[2]) {
        new_element.begin_values_exploded.push_back(exploded_value);
      }

      /*
      if (new_element.end_values.back() <= 0.) new_element.end_values.back() = 1.0;
      for (double exploded_value = new_element.end_values[0]; exploded_value <= new_element.end_values[1]; exploded_value += new_element.end_values[2]) {
        new_element.end_values_exploded.push_back(exploded_value);
      }
      */

      if (new_element.length_values.back() <= 0.) new_element.length_values.back() = 1.0;
      for (double exploded_value = new_element.length_values[0]; exploded_value <= new_element.length_values[1]; exploded_value += new_element.length_values[2]) {
        new_element.length_values_exploded.push_back(exploded_value);
      }

      if (new_element.par_01_values.back() <= 0.) new_element.par_01_values.back() = 1.0;
      for (double exploded_value = new_element.par_01_values[0]; exploded_value <= new_element.par_01_values[1]; exploded_value += new_element.par_01_values[2]) {
        new_element.par_01_values_exploded.push_back(exploded_value);
      }

      if (new_element.par_02_values.size()) {
        if (new_element.par_02_values.size() == 3) {
          if (new_element.par_02_values.back() <= 0.) new_element.par_02_values.back() = 1.0;
          for (double exploded_value = new_element.par_02_values[0]; exploded_value <= new_element.par_02_values[1]; exploded_value += new_element.par_02_values[2]) {
            new_element.par_02_values_exploded.push_back(exploded_value);
          }
        }
        else {
          std::cerr << "Missing scan values!" << std::endl;
          my_exit(-1);
        }
      }

      if (new_element.par_03_values.size()) {
        if (new_element.par_03_values.size() == 3) {
          if (new_element.par_03_values.back() <= 0.) new_element.par_03_values.back() = 1.0;
          for (double exploded_value = new_element.par_03_values[0]; exploded_value <= new_element.par_03_values[1]; exploded_value += new_element.par_03_values[2]) {
            new_element.par_03_values_exploded.push_back(exploded_value);
          }
        }
        else {
          std::cerr << "Missing scan values!" << std::endl;
          my_exit(-1);
        }
      }

      if (new_element.par_04_values.size()) {
        if (new_element.par_04_values.size() == 3) {
          if (new_element.par_04_values.back() <= 0) new_element.par_04_values.back() = 1.0;
          for (double exploded_value = new_element.par_04_values[0]; exploded_value <= new_element.par_04_values[1]; exploded_value += new_element.par_04_values[2]) {
            new_element.par_04_values_exploded.push_back(exploded_value);
          }
        }
        else {
          std::cerr << "Missing scan values!" << std::endl;
          my_exit(-1);
        }
      }
    }
    catch (std::exception &e) {
      std::cerr << "Scan parameters exception: " << e.what() << std::endl;
      my_exit(-1);
    }

    magnetic_elements.push_back(new_element);
  }

  if (emin < 0.0) emin = 0.0;
  if (emax < 0.0) emax = 1.0e+100;
}



void Propaga_parameters::debug_scan() {
  std::ofstream debug_log("debug.log");
  for (auto comb : scan) {
    debug_log << comb << std::endl;
  }
  debug_log.close();
}


void Propaga_parameters::print_scan_input_files() {
  size_t file_counter = 0;
  std::vector<double> scan_d = to_double(scan);
  for (size_t index_comb = 0; index_comb < scan_d.size(); /**/)
  {
    jsoncons::json scan_parameters;
    scan_parameters["emin"] = emin;
    scan_parameters["emax"] = emax;
    scan_parameters["dt"] = dt;
    scan_parameters["steps_between_dumps"] = big_coutta;
    scan_parameters["steps_between_diags"] = diag_coutta;
    jsoncons::json mel = jsoncons::json::array();
    for (size_t i = 0; i < magnetic_elements.size(); i++) {
      jsoncons::json lattice_element;
      try {
        lattice_element["type"] = magnetic_elements[i].type;
        lattice_element["begin_value"] = scan_d[index_comb];
        lattice_element["end_value"] = scan_d[index_comb] + scan_d[index_comb+1];
        index_comb+=2;
        lattice_element["par_01"] = scan_d[index_comb++];
        lattice_element["par_02"] = scan_d[index_comb++];
        lattice_element["par_03"] = scan_d[index_comb++];
        lattice_element["par_04"] = scan_d[index_comb++];
      }
      catch (std::exception &e) {
        std::cerr << "Lattice Element Exception: " << e.what() << std::endl;
        my_exit(-1);
      }
      mel.add(lattice_element);
    }
    scan_parameters["Magnetic_elements"] = mel;

    std::string output_name = scan_name + "_" + std::to_string(file_counter) + ".json";
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    try {
      output_file << jsoncons::pretty_print(scan_parameters) << std::endl;
    }
    catch (std::exception &e) {
      std::cerr << "Unable to write to file " << output_name << ". Exception: " << e.what() << std::endl;
      my_exit(-1);
    }
    output_file.close();
    file_counter++;
  }
}



int main()
{
  std::cout << "scan-parameters v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
  Propaga_parameters parametri;
  parametri.input_parameters_filename = "input.json";
  parametri.scan_name = "test";
  parametri.input_dist_filename = parametri.scan_name + ".initialbunch.ppg";

  parametri.parse_json_file();

  if (!parametri.magnetic_elements.size()) {
    std::cerr << "No magnetic elements found, scan interrupted" << std::endl;
    my_exit(-1);
  }

  std::cout << "Starting combinatorial explosion" << std::endl;

  parametri.scan = combo(
    parametri.magnetic_elements.front().begin_values_exploded,
    //parametri.magnetic_elements.front().end_values_exploded,
    parametri.magnetic_elements.front().length_values_exploded,
    parametri.magnetic_elements.front().par_01_values_exploded,
    parametri.magnetic_elements.front().par_02_values_exploded,
    parametri.magnetic_elements.front().par_03_values_exploded,
    parametri.magnetic_elements.front().par_04_values_exploded
  );

  urgent_message("Found " + std::to_string(parametri.scan.size()) + " total combinations to explore");

  for (size_t i = 1; i < parametri.magnetic_elements.size(); i++) {
    parametri.scan = combo(
      parametri.scan,
      parametri.magnetic_elements[i].begin_values_exploded,
      //parametri.magnetic_elements[i].end_values_exploded,
      parametri.magnetic_elements[i].length_values_exploded,
      parametri.magnetic_elements[i].par_01_values_exploded,
      parametri.magnetic_elements[i].par_02_values_exploded,
      parametri.magnetic_elements[i].par_03_values_exploded,
      parametri.magnetic_elements[i].par_04_values_exploded
    );
    urgent_message("Found " + std::to_string(parametri.scan.size()) + " total combinations to explore");
  }

  //parametri.debug_scan();
  parametri.print_scan_input_files();
  return 0;
}
