#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <dirent.h>
using namespace std;

vector<string> parseArguments(int argc, char **argv);
void addFile(const char *directory, vector<string> &files, 
                   bool recursive);
void checkColumn(vector<string> files);
void checkTabs(vector<string> files);
void detab(string filename);

int main(int argc, char **argv)                 hjkfldsa            hjkfldsa    hkjfsdahkl  fas
{
    vector<string> files = parseArguments(argc, argv);

    checkTabs(files);
    checkColumn(files);

    return 0;
}

vector<string> parseArguments(int argc, char **argv)
{
    vector<string> files;
    bool recursive = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (toupper(argv[i++][1]) == 'r') {
                recursive = true;
            } else {
                cerr << argv[0] << ": unregonized flag" << endl;
                exit(1);
            }
        } 
        addFile(argv[i], files);
    }

    return files;
}

void addFile(const char *directory, vector<string> &files, 
                   bool recursive) 
{
    struct dirent *entry;
    DIR *dp = opendir(directory);
    string path(directory);

    if (!dp) {
        files.push_back(path);
        return;
    }

    entry = readdir(dp);
    while (entry) {
       if ((entry->d_name)[0] == '.') {
          entry = readdir(dp);
          continue;
       }
        string currentFile = path + '/' + entry->d_name;
        files.push_back(currentFile);
        entry = readdir(dp);
    }

    closedir(dp);
}

void checkColumn(vector<string> files)
{
    for (unsigned i = 0; i < files.size(); i++) {
        string filename = files[i];
        ifstream infile(filename.c_str());

        if (!infile.is_open()) {
            if (filename[0] != '*')
                cerr << "Error opening file: " << filename << endl;
            continue;
        }

        string checkLine;
        int lineNumber = 1;

        while (!getline(infile, checkLine).eof()) {
            if (checkLine.length() > 80) {
                cout << filename << ":" << lineNumber 
                     << " goes past 80 columns.\n";
            }
            lineNumber++;
        }
        infile.close();
    }
}

void checkTabs(vector<string> files)
{
    for (unsigned i = 0; i < files.size(); i++) {
        bool keepChecking = true;
        char response;
        string filename = files[i];
        ifstream infile(filename.c_str());

        // Do not check Makefile for tabs
        if (!infile.is_open() || filename == "Makefile") {
            continue;
        }

        string checkLine;
        while (keepChecking && !getline(infile, checkLine).eof()) {
            for (unsigned j = 0; j < checkLine.length(); j++) {
                if (checkLine[j] == '\t') {
                    cerr << "Tabs found in " << filename << endl;
                    cerr << "Would you like to detab this file? (Y/N) ";
                    cin >> response;

                    if (toupper(response) == 'Y') {
                        detab(filename);
                    } 

                    keepChecking = false;
                    break;
                }
            }
        }
    }
}

void detab(string filename)
{
    int numSpaces;
    cerr << "How many spaces per tab? ";
    while (!(cin >> numSpaces) || (numSpaces < 1)) {
        cin.clear();
        cin.ignore(256,'\n');
        cout << "Invalid Input. Enter a positive integer";
    }

    vector<string> lines;
    string checkLine;
    ifstream infile(filename.c_str());

    while (!getline(infile, checkLine).eof()) {
        string currentLine = "";

        for (unsigned i = 0; i < checkLine.length(); i++) {
            if (checkLine[i] == '\t') {
                for (int i = 0; i < numSpaces; i++) {
                    currentLine += ' ';
                }
            } else {
                currentLine += checkLine[i];
            }
        }

        lines.push_back(currentLine);
    }

    ofstream outfile(filename.c_str());
    for (unsigned i = 0; i < lines.size(); i++) {
        outfile << lines[i] << endl;
    }
}
