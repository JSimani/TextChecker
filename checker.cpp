#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <dirent.h>
#include "wordWrap.h"
using namespace std;

struct Flags {
    bool tabs;
    bool columns;
    bool brackets;
    bool readHidden;
    bool recursive;
};

void printHelp(char **argv);
vector<string> parseArguments(int argc, char **argv, Flags &cFlags);
void addFile(string path, vector<string> &files, bool recursive, 
             bool readHidden);
void checkColumns(vector<string> files);
void checkTabs(vector<string> files);
void detab(string filename);
void checkBrackets(string filename);

int main(int argc, char **argv) 
{
    unsigned i;
    Flags cFlags = {false, false, false, false, false};
    vector<string> files = parseArguments(argc, argv, cFlags);

    if (files.empty()) {
        printHelp(argv);
    }

    if (cFlags.tabs){
        checkTabs(files);
    }

    if (cFlags.columns) {
        checkColumns(files);
    }

    if (cFlags.brackets) {
        for (i = 0; i < files.size(); i++) {
            checkBrackets(files[i]);
        }
    }

    return 0;
}

void printHelp(char **argv)
{
    stringstream ss;
    ss << "usage: " << argv[0] << " [-abcrt] [--all] [--bracket] "
       << "[--column] [--tab] [--recursive] [file ...]";
    wordWrap(ss, cerr, 0);

    ss << "-a, --all";
    wordWrap(ss, cerr, 4);

    ss << "Include directory entries whose names begin with a dot (.) in "
       << "check.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-b, --bracket";
    wordWrap(ss, cerr, 4);

    ss << "Check for bracket, quotation, and parenthesis mismatch";
    wordWrap(ss, cerr, 8);
    ss << "NOTE: This feature is only fully functional in C++ files, as it "
       << "ignores characters within C++ styled comments";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-c, --column";
    wordWrap(ss, cerr, 4); 

    ss << "Check that the given file does not contain a line of text going "
       << "past 80 characters.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-r, --recursive";
    wordWrap(ss, cerr, 4); 

    ss << "Recursively check a directory and its subdirectories for "
       << "files to check.";
    wordWrap(ss, cerr, 8);
    cerr << endl;

    ss << "-t, --tab";
    wordWrap(ss, cerr, 4); 

    ss << "Check that the given file does not contain any tab characters, and "
       << "gives the option to replace those tabs with spaces.";
    wordWrap(ss, cerr, 8);

    exit(1);
}

vector<string> parseArguments(int argc, char **argv, Flags &cFlags)
{
    vector<string> files;
    int i;
    unsigned j;
    vector<string> paths;
    string currentArg;
    stringstream ss;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            currentArg = argv[i];
            if (currentArg.size() == 1) {
                ss << argv[0] << ": unregonized flag \'" << argv[i]
                   << "\'";
                wordWrap(ss, cerr, 0);
                printHelp(argv);
            }
            if (currentArg.substr(1, currentArg.length() - 1) == "-all") {
                cFlags.readHidden = true;
                continue;
            } else if (currentArg.substr(1, currentArg.length() - 1) == 
                       "-bracket") {
                cFlags.brackets = true;
                continue;
            } else if (currentArg.substr(1, currentArg.length() - 1) == 
                       "-column") {
                cFlags.columns = true;
                continue;
            } else if (currentArg.substr(1, currentArg.length() - 1) == 
                       "-recursive") {
                cFlags.recursive = true;
                continue;
            } else if (currentArg.substr(1, currentArg.length() - 1) == 
                       "-tab") {
                cFlags.tabs = true;
                continue;
            }
            for (j = 1; j < strlen(argv[i]); j++) {
                if (argv[i][j] == 'a') {
                    cFlags.readHidden = true;
                } else if (argv[i][j] == 'b') {
                    cFlags.brackets = true;
                } else if (argv[i][j] == 'c') {
                    cFlags.columns = true;
                } else if (argv[i][j] == 'r') {
                    cFlags.recursive = true;
                } else if (argv[i][j] == 't') {
                    cFlags.tabs = true;
                } else {
                    ss << argv[0] << ": unregonized flag \'" << argv[i][j]
                       << "\'";
                    wordWrap(ss, cerr, 0);
                    printHelp(argv);
                }
            }
            continue;
        } 

        paths.push_back(argv[i]);
    }

    for (i = 0; (size_t)i < paths.size(); i++) {
        addFile(paths[i], files, cFlags.recursive, cFlags.readHidden);
    }

    return files;
}

void addFile(string path, vector<string> &files, bool recursive, 
             bool readHidden) 
{
    struct dirent *entry;
    DIR *dp = opendir(path.c_str());
    string newPath;
    string currentEntry;

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
        currentEntry = entry->d_name;
        
        if (currentEntry == "." || currentEntry == "..") {
            entry = readdir(dp);
            continue;
        } else if (currentEntry[0] == '.' && !readHidden) {
            entry = readdir(dp);
            continue;
        }

        newPath = path + '/' + currentEntry;
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
    stringstream ss;

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
                    ss << "Tabs found in " << filename
                       << ":" << lineNumber;
                    wordWrap(ss, cerr, 0);
                    ss << "Would you like to detab this file? ";
                    wordWrap(ss, cerr, 0);
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
    stringstream ss;

    ss << "How many spaces per tab? ";
    wordWrap(ss, cerr, 0);
    while (!(cin >> numSpaces) || (numSpaces < 1)) {
        cin.clear();
        cin.ignore(256,'\n');
        cerr << "Invalid Input. Enter a positive integer. ";
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

void checkBrackets(string filename)
{
    stack<char> s;
    ifstream infile(filename.c_str());
    stringstream ss;
    char currentChar;
    char topStack;
    string currentLine;
    int lineNumber = 1;
    bool singleQuote = false;
    bool doubleQuote = false;
    bool commentBlock = false;
    bool commentLine = false;

    while (!getline(infile, currentLine).eof()) {
        for (size_t i = 0; i < currentLine.length(); i++) {
            currentChar = currentLine[i];
            switch (currentChar) {
                case '{': 
                    if (!singleQuote && !doubleQuote && !commentBlock &&
                        !commentLine) {
                        s.push(currentChar);
                    }
                    break;
                case '}':
                    if (!singleQuote && !doubleQuote && !commentBlock &&
                        !commentLine) {
                        if (s.empty() || s.top() != '{') {
                            ss << filename << ':' << lineNumber 
                               << " Bracket mismatch \'" << currentChar 
                               << "\'";
                            wordWrap(ss, cerr, 0);
                        } else {
                            s.pop();
                        }
                    }
                    break;
                case '\\': 
                    if (!commentBlock && !commentLine) {
                        i++;
                    }
                    break;
                case '/':
                    if (i + 1 < currentLine.length()) {
                        if (currentLine[i + 1] == '/') {
                            commentLine = true;
                        } else if (currentLine[i + 1] == '*') {
                            commentBlock = true;
                        }
                    }
                    break;
                case '*':
                    if (!singleQuote && !doubleQuote && 
                        i + 1 < currentLine.length()) {
                        if (currentLine[i + 1] == '/' && commentLine) {
                            commentLine = false;
                        } else if (currentLine[i + 1] == '/' && !commentLine) {
                            ss << filename << ':' << lineNumber 
                               << " Comment mismatch '*/'";
                            wordWrap(ss, cerr, 0);
                        } 
                    }
                    break;
                case '\'':
                    if (doubleQuote || commentBlock || commentLine) {
                        break;
                    } else if (singleQuote) {
                        try {
                            topStack = s.top();
                            if (topStack != '\'') {
                                throw runtime_error("Single quote mismatch");
                            }
                            s.pop();
                            singleQuote = false;
                        } catch (...) {
                            ss << filename << ':' << lineNumber 
                               << " Quotation mismatch \'" << currentChar 
                               << "\'";
                            wordWrap(ss, cerr, 0);
                        }
                    } else {
                        s.push(currentChar);
                        singleQuote = true;
                    }
                    break;
                case '\"':
                    if (singleQuote || commentBlock || commentLine) {
                        break;
                    } else if (doubleQuote) {
                        try {
                            topStack = s.top();
                            if (topStack != '\"') {
                                throw runtime_error("Double quote mismatch");
                            }
                            s.pop();
                            doubleQuote = false;
                        } catch (...) {
                            ss << filename << ':' << lineNumber 
                               << " Quotation mismatch \'" << currentChar 
                               << "\'";
                            wordWrap(ss, cerr, 0);
                        }
                    } else {
                        s.push(currentChar);
                        doubleQuote = true;
                    }
                    break;
                case '[':
                    if (!singleQuote && !doubleQuote && !commentBlock &&
                        !commentLine) {
                        s.push(currentChar);
                    }
                    break;
                case ']':
                    if (!singleQuote && !doubleQuote && !commentBlock && 
                        !commentLine) {
                        if (s.empty() || s.top() != '[') {
                            ss << filename << ':' << lineNumber 
                               << " Bracket mismatch \'" << currentChar 
                               << "\'";
                            wordWrap(ss, cerr, 0);
                        } else {
                            s.pop();
                        }
                    }
                    break;
                case '(':
                    if (!singleQuote && !doubleQuote && !commentBlock &&
                        !commentLine) {
                        s.push(currentChar);
                    }
                    break;
                case ')':
                    if (!singleQuote && !doubleQuote && !commentBlock && 
                        !commentLine) {
                        if (s.empty() || s.top() != '(') {
                            ss << filename << ':' << lineNumber 
                               << " Bracket mismatch \'" << currentChar 
                               << "\'";
                            wordWrap(ss, cerr, 0);
                        } else {
                            s.pop();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        commentLine = false;
        lineNumber++;
    }
        
    infile.close();
}




