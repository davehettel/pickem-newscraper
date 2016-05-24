#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <regex>
#include "filefunctions.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
using namespace std;

// Flags to use when compiling:
/*
 -lxml2 -lcurl -lsqlite3
 */

static void get_player_info(xmlNode* node) {
  
}

static void recurse_tree(xmlNode* node) {
  xmlNode* cur_node = NULL;
  
  for(cur_node = node; cur_node; cur_node = cur_node->next) {
    if(cur_node->type == XML_ELEMENT_NODE && to_string(cur_node->name) == "tr")
    {
      get_player_info(cur_node);
    } else
      recurse_tree(cur_node->children);
  }
}

void parse_game(string file, bool verbose) {
  string boxscore = read_file(file);
  
  regex expression("<!DOCTYPE([\\s\\S]*?)<div class=\"yom-mod yom-data");
  boxscore = regex_replace(boxscore, expression, "<document><div class=\"yom-mod yom-data");
  
  expression = "</dl></div></div></div>([\\s\\S]*)-->";
  boxscore = regex_replace(boxscore, expression, "</dl></div></div></div></document>");
  
  expression = "<colgroup>(.*?)</colgroup>";
  boxscore = regex_replace(boxscore, expression, "");
  
  expression = "<thead>([\\s\\S]*?)</thead>";
  boxscore = regex_replace(boxscore, expression, "");
  
  expression = "<img(.*?)>";
  boxscore = regex_replace(boxscore, expression, "");
  
  write_file(file, boxscore);
  
  // Now we hand it off to the XML parser. This bit initializes
  // it and loads the file.
  LIBXML_TEST_VERSION;
  xmlDoc *tree = xmlReadFile(file.c_str(), NULL, 0);
  if (tree == NULL)
    cerr << "Failed to parse " << file << "\n";
  else if (verbose)
    cout << "Successfully parsed " << file << "\n";
  
  xmlNode *root_element = xmlDocGetRootElement(tree);
  
  recurse_tree(root_element->children);
}

int main (int argc, char** argv) {
  vector<string> scores;
  bool verbose_mode = (argc > 1 && strncmp(argv[1], "verbose", 7) == 0);
  
  string scorefile = "http://sports.yahoo.com/mlb/scoreboard/?date=2015-05-12";
  download(scorefile.c_str(), "./files/scoreboard.html", verbose_mode);
  
  string scoreContents = read_file("./files/scoreboard.html");

  regex expression("<!DOCTYPE([\\s\\S]*?)data-url=\"");
  scoreContents = regex_replace(scoreContents, expression, "");
  
  expression = "/\"([\\s\\S]*?)data-url=\"";
  scoreContents = regex_replace(scoreContents, expression, "\n");
  
  expression = "/\"([\\s\\S]*)-->";
  scoreContents = regex_replace(scoreContents, expression, "");
  
  write_file("./files/game_list.txt", scoreContents);
  
  remove("./files/scoreboard.html");
  
  ifstream file_list("./files/game_list.txt", ifstream::in);
  
  if(file_list.is_open()) {
    string indiv_line;
    
    while(file_list.good()) {
      getline(file_list, indiv_line);
      
      string web_file = "http://sports.yahoo.com" + indiv_line + "/";
      string download_location = "./files" + indiv_line;
      download_location.erase(7, 4);
      download_location += ".html";
      
      download(web_file.c_str(), download_location.c_str(), verbose_mode);
      scores.push_back(download_location);
    }
  }
  
  remove("./files/game_list.txt");
  
  for(int i = 0; i < scores.size(); i++)
    parse_game(scores[i], verbose_mode);
    
  return 0;
}