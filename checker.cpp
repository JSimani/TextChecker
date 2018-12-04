#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <dirent.h>
#include "wordWrap.h"
using namespace std;

void printHelp(char **argv);
vector<string> parseArguments(int argc, char **argv, bool &tabs, 
                              bool &columns);
void addFile(string path, vector<string> &files, bool recursive, 
             bool readHidden);
void checkColumns(vector<string> files);
void checkTabs(vector<string> files);
void detab(string filename);

int main(int argc, char **argv) 
{
    bool tabs = false, columns = false;
    vector<string> files = parseArguments(argc, argv, tabs, columns);

    if (files.empty()) {
        printHelp(argv);
    }

    if (tabs){
        checkTabs(files);
    }

    if (columns) {
        checkColumns(files);
    }

    return 0;
}

void printHelp(char **argv)
{
    stringstream ss;
    ss << "usage: " << argv[0] << " [-AaCcRrTt] [file ...]";
    wordWrap(ss, cerr, 0);

    ss << "-A, -a";
    wordWrap(ss, cerr, 4);

    ss << "Include directory entries whose names "
       << "begin with a dot (.) in check.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-C, -c";
    wordWrap(ss, cerr, 4); 

    ss << "Check that the given file does not contain a line of text going "
       << "past 80 characters.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-R, -r";
    wordWrap(ss, cerr, 4); 

    ss << "\t\tRecursively check a directory and its subdirectories for "
       << "files to check.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-T, -t";
    wordWrap(ss, cerr, 4); 

    ss << "Check that the given file does not contain any tab characters, and "
       << "gives the option to replace those tabs with spaces.";
    wordWrap(ss, cerr, 8);

    exit(1);
}

vector<string> parseArguments(int argc, char **argv, bool &tabs, 
                              bool &columns)
{
    vector<string> files;
    bool recursive = false, readHidden = false;
    int i;
    unsigned j;
    string path;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (j = 1; j < strlen(argv[i]); j++) {
                if (toupper(argv[i][j]) == 'A') {
                    readHidden = true;
                } else if (toupper(argv[i][j]) == 'C') {
                    columns = true;
                } else if (toupper(argv[i][j]) == 'T') {
                    tabs = true;
                } else if (toupper(argv[i][j]) == 'R') {
                    recursive = true;
                } else {
                    cerr << argv[0] << ": unregonized flag \'" << argv[i][j]
                         << "\'" << endl;
                    printHelp(argv);
                }
            }
            continue;
        } 

        path = argv[i];
        addFile(path, files, recursive, readHidden);
    }

    return files;
}

void addFile(string path, vector<string> &files, bool recursive, 
             bool readHidden) 
{
    struct dirent *entry;
    DIR *dp = opendir(path.c_str());
    string newPath;

    if (!dp) {
        files.push_back(path);
        return;
    }

    if (!recursive) {
        cerr << path << " is a directory" << endl;
        closedir(dp);
        return;
    }

    entry = readdir(dp);
    while (entry) {
        // If the file is a naturally hidden file, don't add it
        if ((entry->d_name)[0] == '.') {
            entry = readdir(dp);
            continue;
        }

        newPath = path + '/' + entry->d_name;
        addFile(newPath, files, recursive, readHidden);
        entry = readdir(dp);
    }

    closedir(dp);
}

void checkColumns(vector<string> files)
{
    unsigned i, j, lineNumber = 1;
    string filename, checkLine;
    ifstream infile;

    for (i = 0; i < files.size(); i++) {
        j = 0;
        filename = files[i];
        infile.open(filename.c_str());

        if (!infile.is_open()) {
            if (filename[0] != '*')
                cerr << "Error opening file: " << filename << endl;
            continue;
        }

        while (!getline(infile, checkLine).eof()) {
            if (checkLine.length() > 80) {
                if (j < 4) {
                    cout << filename << ":" << lineNumber 
                         << " goes past 80 columns." << endl;
                    j++;
                } else {
                    cout << "More than 4 lines go past 80 columns in \'" 
                         << filename << "\'..." << endl;
                    break;
                }
            }
            lineNumber++;
        }
        infile.close();
    }
}

void checkTabs(vector<string> files)
{
    unsigned i, j, lineNumber;
    bool keepChecking;
    string response, filename, checkLine;
    ifstream infile;

    for (i = 0; i < files.size(); i++) {
        keepChecking = true;
        filename = files[i];
        infile.open(filename.c_str());

        if (!infile.is_open()) {
            cerr << "Error opening file \'" << filename << "\'" << endl;
            continue;
        }

        lineNumber = 1;
        while (keepChecking && !getline(infile, checkLine).eof()) {
            for (j = 0; j < checkLine.length(); j++) {
                if (checkLine[j] == '\t') {
                    cout << "Tabs found in " << filename
                         << ":" << lineNumber << endl;
                    cout << "Would you like to detab this file? ";
                    cin >> response;

                    if (toupper(response[0]) == 'Y') {
                        detab(filename);
                    } 

                    keepChecking = false;
                    break;
                }
            }
            lineNumber++;
        }
        infile.close();
    }
}

void detab(string filename)
{
    int numSpaces;
    unsigned i, j;
    vector<string> lines;
    string checkLine, currentLine;
    ifstream infile(filename.c_str());
    ofstream outfile;

    cout << "How many spaces per tab? ";
    while (!(cin >> numSpaces) || (numSpaces < 1)) {
        cin.clear();
        cin.ignore(256,'\n');
        cout << "Invalid Input. Enter a positive integer. ";
    }    

    while (!getline(infile, checkLine).eof()) {
        currentLine = "";

        for (i = 0; i < checkLine.length(); i++) {
            if (checkLine[i] == '\t') {
                for (j = 0; j < (unsigned)numSpaces; j++) {
                    currentLine += ' ';
                }
            } else {
                currentLine += checkLine[i];
            }
        }

        lines.push_back(currentLine);
    }

    outfile.open(filename.c_str());
    for (unsigned i = 0; i < lines.size(); i++) {
        outfile << lines[i] << endl;
    }

    infile.close();
    outfile.close();
}
