// built-in packages
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <algorithm>

//3rd-party packages
#include <jsoncpp/json/json.h>

std::string ensure_one_middle_char(
  std::string first_str,
  std::string second_str,
  char character)
/* Function ensures exactly one character,
 * which is given as third parameter, between
 * two strings (first and second parameters). */
{
  std::string ret;
  while (first_str.back() == character)
  {
    first_str.erase(first_str.size() - 1);
  }
  while (second_str.front() == character)
  {
    second_str.erase(0, 1);
  }
  ret = first_str + character + second_str;
  return ret;
}

std::string handle_working_dir(void)
/* Function gets current working directory 
 * and writes it to configuration file, then
 * returns the current working directory as
 * a string. */
{
  char buff[FILENAME_MAX];
  getcwd(buff, FILENAME_MAX);
  std::string current_path = buff;
  std::string line_to_conf_file = "WORKING_DIR=" + current_path;

  std::ofstream conf_file("./config", std::fstream::app);
  if (conf_file.is_open())
  {
    conf_file << line_to_conf_file << std::endl;
    conf_file.close();
  }
  return current_path;
}

std::string get_command_file_full_path(void)
{
  std::string ret_full_path = "none";
  std::ifstream conf_file("./config");
  std::string working_dir;
  bool working_dir_set = false;
  std::string commands_file;
  bool commands_file_set = false;

  /* Read configuration file line by line. */
  if (conf_file.is_open()) 
  {  
    std::string line;
    while (getline(conf_file, line))
    { 
      /* Skip empty lines and lines that start with a '#' */
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      /* Split keys and values from configuration file. */
      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      /* Set working directory from configuration file. */
      if (name == "WORKING_DIR")
      {
        working_dir = value;
        working_dir_set = true;
      }

      /* Set commands file from configuration file. */
      if (name == "COMMANDS_FILE")
      {
        commands_file = value;
        commands_file_set = true;
      }
    } // of while getline
    conf_file.close();

    /* If configuration file contained both a working
     * directory and a commands-file, read commands
     * from that file. Additionally make sure that
     * there exists exactly one forward-slash betwee
     * the the working directory path and the file name. */
    if (working_dir_set && commands_file_set)
    {
      ret_full_path = ensure_one_middle_char(working_dir, commands_file, '/');
    }
    else if (commands_file_set)
    {
      working_dir = handle_working_dir();
      ret_full_path = ensure_one_middle_char(working_dir, commands_file, '/');
    }
  } // of if conf_file is open
  return ret_full_path;
}

std::string get_alias_file(void)
/* Get name (full path) of alias
 * file, which should be in
 * configuration file. */
{
  std::string alias_file = "none";

  std::ifstream conf_file("./config");
  if (conf_file.is_open())
  {
    std::string line;
    while (getline(conf_file, line))
    {
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      /* Split keys and values from configuration file. */
      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      if (name == "ALIAS_FILE")
      {
        alias_file = value;
      }
    }
    conf_file.close();
  }
  return alias_file;
}  

std::string get_executables_dir(void)
/* Function reads configuration file
 * and retrieves a directory name
 * where executables should be placed. */
{
  std::string executables_dir = "executables";

  std::ifstream conf_file("./config");
  if (conf_file.is_open())
  {
    std::string line;
    while (getline(conf_file, line))
    {
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      /* Split keys and values from configuration file. */
      auto delimiterPos = line.find("=");
      auto name = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      if (name == "EXECUTABLES_DIR")
      {
        executables_dir = value;
      }
    }
    conf_file.close();
  }
  return executables_dir;
}

bool check_executables_dir(std::string executables_dir_name)
/* Function checks whether given parameter is
 * a directory that exists in the current
 * working directory and creates it in case
 * it does not exist. */
{
  bool all_good = true;
  std::string executables_dir = executables_dir_name;
  
  /* Check if './' if already prefixed in
   * the directory name. */
  std::string prefix = "./";
  if (!executables_dir_name.rfind(prefix, 0) == 0)
  {
    executables_dir = prefix + executables_dir;
  }

  /* Check if directory exists. */
  struct stat info;
  bool dir_exists = false;
  if (stat(executables_dir.c_str(), &info) != -1)
  {
    if (S_ISDIR(info.st_mode))
    {
      dir_exists = true;
    }
  }

  /* Create directory if it does not exist. */
  if (!dir_exists)
  {
    int is_fail = mkdir(executables_dir_name.c_str(), 0667);
    if (!is_fail)
    {
      std::cout << "Created executables directoty." << std::endl;
    }
    else
    {
      std::cout << "Unable to create executables directory." << std::endl;
      all_good = false;
    }
  }
  return all_good;
}

std::string read_file_to_string(const std::string &file_name)
/* Function receives a file name (path) as parameter
 * and reads its contents into a string which is returned. */
{
  std::ifstream ifs(file_name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  std::ifstream::pos_type file_size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  std::vector<char> bytes(file_size);
  ifs.read(bytes.data(), file_size);
  if (ifs.is_open()) { ifs.close(); }
  return std::string(bytes.data(), file_size);
}

std::string insert_lines(
  std::string contents,
  std::string alias,
  std::string command)
/* Function takes a string representing the contents
 * of a file and replaces or insert lines depending
 * on whether corresponding command lines already
 * exists on the file or not. */
{
  std::istringstream text(contents);
  std::string line;
  
  std::string line_start_comparison = "alias " + alias;
  std::string new_line;

  std::string return_str;

  bool command_inserted = false;
  while (std::getline(text, line))
  {
    if (line.rfind(line_start_comparison, 0) == 0)
    {
      std::string replacement_str = "alias " + alias + "=" + command + "\n";
      return_str += replacement_str;
      command_inserted = true;
    }
    else
    {
      std::string line_with_newline = line + "\n";
      return_str += line_with_newline;
    }
  }

  if (!command_inserted)
  {
    std::string insert_line = "alias " + alias + "=" + command + "\n";
    return_str += insert_line;
  }
  return return_str;
}

std::string get_current_dir(void)
/* Function returns current
 * working directory as a string. */
{
  char buff[FILENAME_MAX];
  getcwd(buff, FILENAME_MAX);
  std::string current_dir = buff;
  return current_dir;
}

std::string form_command(
  std::string alias,
  std::string executables_dir,
  std::string executable_name)
/* Function creates a string representing
 * a full alias file line and return it. */
{
  std::string current_dir = get_current_dir();
  while (current_dir.back() == '/')
  {
    current_dir.erase(current_dir.size() - 1);
  }

  while (executables_dir.front() == '/')
  {
    executables_dir.erase(0, 1);
  }

  std::string full_alias_line = "'" + current_dir + "/" + executables_dir + "/" + executable_name + "'\n";
  
  return full_alias_line;
}

std::string strip_newline(std::string str)
/* Function strips newline from string.  */
{
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  return str;
}

std::string strip_quotation_marks(std::string str)
/* Function strips quotation marks from string. */
{
  str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
  return str;
}

std::string prefix_join(
  std::string compilation_libraries[],
  int lib_count)
/* Function takes an array of libraries as parameter and
 * joins + prefixes them into one string which is returned. */
{
  const std::string prefix = " -";
  std::string libraries_str = "";

  for (int i = 0; i != lib_count; ++i)
  {
    std::string library_addon = prefix + strip_quotation_marks(strip_newline(compilation_libraries[i]));
    libraries_str += library_addon;
  }
  return libraries_str;
}

void compile_from_source(
  std::string source_file,
  std::string executables_dir_name,
  std::string executable_name,
  std::string libraries[] = NULL,
  int libraries_count = 0)
/* Function generates a Linux shell-compatible
 * string which compiles source files, and then
 * executes that string, resulting in source
 * file compilation. */
{
  /* Set compiler and source directory
   * for .cpp files. */
  std::string compiler = "g++";
  std::string source_directory = "./src/";

  /* Add external libraries to compilation
   * command in case any exist. */
  std::string libraries_str = "";
  if (libraries_count)
  {
    libraries_str = prefix_join(libraries, libraries_count);
  }

  /* Form the compilation commands string. */
  std::string compilation_command = compiler + " -o " +
  "./" + executables_dir_name + "/" + executable_name +
  " " + source_directory + source_file + libraries_str;
  std::cout << "Compiling " << source_file << " ...";
  
  /* Try compiling. There are many things
   * here that can go wrong, e.g. no permissions
   * on executables directory or no required
   * libraries installed. Inform errors to stdout. */
  try
  {
    std::system(compilation_command.c_str());
    std::cout << " done." << std::endl;
  }
  
  catch(const std::exception &exc)
  {
    std::cerr << exc.what();
  }
}

std::string loop_through_commands(
  std::string command_file_name,
  std::string alias_file_name,
  std::string executables_dir_name)
/* Function loops through the commands in commands
 * configuration file, and compiles executables &
 * adds aliases to alias file according to the paths
 * of the newly created executables. */
{
  /* Read command JSON file into a variable. */
  Json::Value commands_contents;
  Json::FastWriter fastWriter;
  std::ifstream commands_file(command_file_name, std::ifstream::binary);
  commands_file >> commands_contents;
  Json::Value commands_array = commands_contents["commands"];
  
  if (commands_file.is_open())
  {
    commands_file.close();
  }

  /* Read alias file contents into a string. */
  std::string alias_file_as_string = read_file_to_string(alias_file_name);

  /* Loop through commands in commands json structure. */
  for (Json::Value::ArrayIndex i = 0; i != commands_array.size(); ++i)
  {
    Json::Value command = commands_array[i];
    Json::Value enabled = strip_newline(fastWriter.write(command["enabled"]));

    /* Only process enabled commands. */
    if (enabled == "true")
    {
      std::string alias = strip_quotation_marks(strip_newline(fastWriter.write(command["alias"])));
      std::string result_executable = strip_quotation_marks(strip_newline(fastWriter.write(command["executable"])));
      std::string command_str = form_command(alias, executables_dir_name, result_executable);
      alias_file_as_string = insert_lines(alias_file_as_string, alias, command_str);
      std::string source_file_name = strip_quotation_marks(strip_newline(fastWriter.write(command["source_file"])));
      
      /* If external libraries are required for compilation,
       * provide their names to compilation function. */
      if (command.isMember("compilation_libraries"))
      {
        int library_count = command["compilation_libraries"].size();
        std::string compilation_libraries[library_count];
        for (int i = 0; i != library_count; ++i)
        {
          compilation_libraries[i] = fastWriter.write(command["compilation_libraries"][i]);
        }
        compile_from_source(source_file_name, executables_dir_name, result_executable, compilation_libraries, library_count);
      }

      /* This else-clause fires in case no external libraries are required for compilation. */
      else
      {
        compile_from_source(source_file_name, executables_dir_name, result_executable);
      }
    }
  }
  return alias_file_as_string;
}

void rewrite_alias_file(
  std::string alias_file_path,
  std::string alias_file_content)
/* Function (re)writes an alias file with given content. */
{
  std::ofstream open_file(alias_file_path, std::ofstream::trunc);
  if (open_file.is_open())
  {
    open_file << alias_file_content;
    open_file.close();
  }
}

int main(void)
{
  /* Get commands file */
  std::string command_file = get_command_file_full_path();
  if (command_file == "none")
  {
    std::cout << "Could not locate command file." << std::endl;
    return 1;
  }

  /* Get alias file */
  std::string alias_file = get_alias_file();
  if (alias_file == "none")
  {
    std::cout << "Could not locate alias file." << std::endl;
    return 1;
  }

  /* Get executables directory */
  std::string executables_dir = get_executables_dir();
  bool dir_success = check_executables_dir(executables_dir);
  if (!dir_success)
  {
    std::cout << "Error initializing executables directory." << std::endl;
    return 1;
  }

  /* Perform source file compilation and retrieve new alias file content. */
  std::string alias_file_as_string = loop_through_commands(command_file, alias_file, executables_dir);

  /* Finally make a new alias file. */
  rewrite_alias_file(alias_file, alias_file_as_string);

  return 0;
}
