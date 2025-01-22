#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <set>
#include <regex>
#include <algorithm>
using namespace std;

const string progressFile = "../data/user_progress.txt";
const string leaderboardFile = "../data/leaderboard.txt";
const int FIXED_ROOT_VALUE = 10;
string username;
int TotalScore = 0;

struct Node {
    string levelName;
    void (*quizFunction)();
    Node* next;

    Node(string name, void (*func)()) : levelName(name), quizFunction(func), next(nullptr) {}
};

struct TreeNode {
    string username;
    int totalPoints; // Total score across all attempts
    int bestScore;   // Best score in any single attempt
    int attempts;    // Tracks total attempts made by user
    TreeNode* left;
    TreeNode* right;

    TreeNode(string uname, int score)
        : username(uname), totalPoints(score), bestScore(score), attempts(1), left(nullptr), right(nullptr) {}
};

void checkLibraryIncludes(const string& code, vector<string>& errors);
void checkSemicolons(const string& code, vector<string>& errors);
void checkVariableNames(const string& code, vector<string>& errors);
void saveUserProgress(const string& username, const string& level);
string getUserProgress(const string& username);

class User {
public:
    string userID, password;

    void registerUser() {
        system("cls");
        cout << "\t\t\t Please Register Yourself:\n";
        cout << "\t\t\t USERNAME: ";
        cin >> userID;
        username = userID;
        cout << "\t\t\t PASSWORD: ";
        cin >> password;

        ofstream f1("../data/records.txt", ios::app);
        f1 << userID << ' ' << password << endl;
        f1.close();
        system("cls");
        cout << "\t\t\t Successfully Registered!\n";
        loginUser();
    }

    bool loginUser() {
        string id, pass;
        system("cls");
        cout << "\t\t\t Please Login Your Account\n";
        cout << "\t\t\t USERNAME: ";
        cin >> userID;
        username = userID;
        cout << "\t\t\t PASSWORD: ";
        cin >> password;

        ifstream input("../data/records.txt");
        while (input >> id >> pass) {
            if (userID == id && password == pass) {
                input.close();
                system("cls");
                cout << "\t\t\t Successfully Logged In!\n\n";
                return true;
            }
        }
        input.close();
        cout << "\t\t\t INVALID USERNAME OR PASSWORD!\n\n";
        return false;
    }
};

class LeaderboardBST {
private:
    TreeNode* root;

    TreeNode* insert(TreeNode* current, const string& username, int score) {
        if (!current) {
            return new TreeNode(username, score);
        }

        if (username == current->username) {
            // Update existing user's stats
            current->totalPoints += score;
            current->attempts += 1;
            current->bestScore = max(current->bestScore, score);
        } else if (score < current->bestScore) {
            current->left = insert(current->left, username, score);
        } else {
            current->right = insert(current->right, username, score);
        }
        return current;
    }

    void inOrderTraversal(TreeNode* current, vector<TreeNode*>& results) {
        if (!current) return;
        inOrderTraversal(current->left, results);
        results.push_back(current);
        inOrderTraversal(current->right, results);
    }

    TreeNode* findUser(TreeNode* current, const string& username) {
        if (!current) return nullptr;
        if (username == current->username) return current;
        if (username < current->username) return findUser(current->left, username);
        return findUser(current->right, username);
    }

    void saveToFile(TreeNode* current, ofstream& file) {
        if (!current) return;
        saveToFile(current->left, file);
        file << current->username << " " << current->totalPoints << " " 
             << current->bestScore << " " << current->attempts << endl;
        saveToFile(current->right, file);
    }

    TreeNode* loadFromFile(ifstream& file) {
        string uname;
        int total, best, attempts;
        TreeNode* newRoot = nullptr;
        while (file >> uname >> total >> best >> attempts) {
            newRoot = insert(newRoot, uname, best);
        }
        return newRoot;
    }

public:
    LeaderboardBST() : root(nullptr) {}

    void insert(const string& username, int score) {
        root = insert(root, username, score);
    }

    void displayLeaderboard() {
        vector<TreeNode*> results;
        inOrderTraversal(root, results);

        // Sort by bestScore descending to show highest scores first
        stable_sort(results.begin(), results.end(), [](TreeNode* a, TreeNode* b) {
            return a->bestScore > b->bestScore;
        });

        cout << "\nLEADERBOARD:" << endl;
        int rank = 1;
        for (auto user : results) {
            cout << "Rank " << rank++ << ": " << user->username
                 << " | Best Score: " << user->bestScore
                 << " | Total Points: " << user->totalPoints
                 << " | Attempts: " << user->attempts << endl;
        }
    }

    void findUser(const string& username) {
        TreeNode* user = findUser(root, username);
        if (user) {
            cout << "\nStats for " << username << ":" << endl;
            cout << "Total Points: " << user->totalPoints << endl;
            cout << "Best Score: " << user->bestScore << endl;
            cout << "Attempts: " << user->attempts << endl;
        } else {
            cout << "User '" << username << "' not found in the leaderboard." << endl;
        }
    }

    void saveLeaderboard() {
        ofstream file(leaderboardFile);
        if (!file.is_open()) {
            cerr << "Error: Unable to save leaderboard data." << endl;
            return;
        }
        saveToFile(root, file);
        file.close();
    }

    void loadLeaderboard() {
        ifstream file(leaderboardFile);
        if (!file.is_open()) {
            cerr << "Warning: No leaderboard data to load." << endl;
            return;
        }
        root = loadFromFile(file);
        file.close();
    }
};

class QuizLevels {
private:
    Node* head;
    Node* currentLevel;
    LeaderboardBST leaderboard;
    

public:
	QuizLevels() : head(nullptr), currentLevel(nullptr) {}
    QuizLevels(LeaderboardBST& lb) : head(nullptr), currentLevel(nullptr), leaderboard(lb) {}

    void addLevel(const string& name, void (*func)()) {
        Node* newNode = new Node(name, func);
        if (head == nullptr) {
            head = newNode;
            currentLevel = head;
        } else {
            Node* temp = head;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }
                                                      
    void playLevels() {
    // Skip to the user's current progress level
    string userProgress = getUserProgress(username);

    while (currentLevel != nullptr && currentLevel->levelName != userProgress) {
        currentLevel = currentLevel->next;
    }

    // Play from the current level
    while (currentLevel != nullptr) {
        cout << "Starting Level: " << currentLevel->levelName << " for "<< username<< endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cin.sync();
        currentLevel->quizFunction(); // Call the quiz function for this level
    
			// system("cls");	
			cout<<endl<< currentLevel->levelName << " level Completed!\n\n";
			currentLevel = currentLevel->next; // Move to next level
		
        
        
        // Save progress after each level
        if (currentLevel != nullptr) {
            saveUserProgress(username, currentLevel->levelName);
        }
    }

    // Check if all levels are completed
    if (currentLevel == nullptr) {
    	saveUserProgress(username, "Beginner");
        cout << "\nAll levels completed! Congratulations!\n";
		leaderboard.insert(username, TotalScore);
		leaderboard.saveLeaderboard();
    }
}
};

void saveUserProgress(const string& username, const string& level) {
    ifstream fileIn(progressFile);
    ofstream tempFile("temp.txt");

    bool found = false;
    string line, user, currentLevel;
    while (getline(fileIn, line)) {
        size_t delimiter = line.find(':');
        if (delimiter != string::npos) {
            user = line.substr(0, delimiter);
            currentLevel = line.substr(delimiter + 1);
            if (user == username) {
                tempFile << username << ":" << level << endl;
                found = true;
            } else {
                tempFile << line << endl;
            }
        }
    }

    if (!found) {
        tempFile << username << ":" << level << endl;
    }

    fileIn.close();
    tempFile.close();
    remove(progressFile.c_str());
    rename("temp.txt", progressFile.c_str());
}

string getUserProgress(const string& username) {
    ifstream file(progressFile);
    if (!file.is_open()) return "Beginner";

    string line, user, level;
    while (getline(file, line)) {
        size_t delimiter = line.find(':');
        if (delimiter != string::npos) {
            user = line.substr(0, delimiter);
            level = line.substr(delimiter + 1);
            if (user == username) {
                file.close();
                return level;
            }
        }
    }
    file.close();
    return "Beginner";
}

// Queue-based Quiz System
bool processQuiz(queue<pair<string, string>> questions) {
    string answer;
    while (!questions.empty()) {
        auto question = questions.front();
        questions.pop();

        cout << question.first << " ";
        getline(cin, answer);

       if (answer == question.second) {
            cout << "Correct Answer!\n";
            TotalScore = TotalScore + 5;
        } else {
            cout << "Wrong Answer!\n";
            cout << "Expected Answer: " << question.second << endl;
            TotalScore = TotalScore - 2;
        }
    }
    
    return true;
}

// Quiz Functions
void beginnerQuiz() {
    cout << "\n[Beginner Level Quiz]\n";

    queue<pair<string, string>> questions;
    questions.push({"1. C++ was developed by?", "Bjarne"});
    questions.push({"2. What is the size of int in C++ (on most compilers)?", "4"});
    
    if (!processQuiz(questions)) {
        exit(0);
    }
}

void intermediateQuiz() {
    cout << "\n[Intermediate Level Quiz]\n";
	
    queue<pair<string, string>> questions;
    questions.push({"1. How to declare a variable in C++?", "datatype variablename = value;"});
    questions.push({"2. Which OOP principle does C++ support (inheritance/polymorphism)?", "inheritance"});
    
    if (!processQuiz(questions)) {
        exit(0);
    }
}

void advancedQuiz() {
    cout << "\n[Advanced Level Quiz]\n";

    string userProgramPath;
    int retryCount = 3;
    bool success = false;
     while (retryCount-- > 0) {
        cout << "Enter the path to the user's program file: ";
        getline(cin, userProgramPath);

        ifstream file(userProgramPath);

        if (!file.is_open()) {
            cerr << "Error: Could not open the file! Please check the file path and try again." << endl;
            if (retryCount == 0) {
            cout << "Max retries reached. Exiting quiz level.\n";
            break;
        	}
            continue;
        }

     string code(( istreambuf_iterator<char>(file)),  istreambuf_iterator<char>());
    file.close();

     vector< string> errors;

    checkVariableNames(code, errors);
	checkSemicolons(code, errors);
    checkLibraryIncludes(code, errors);

    if (errors.empty()) {
        cout << "Your program is great!" <<  endl;
        success = true;
        break;
    } else {
        for (const auto& error : errors) {
             cout << error <<  endl;
        }
    }
    }
    if (!success) {
        cout << "Advanced quiz failed.\n";
    }
}

class QuizApp {
public:
    void run() {
        int choice;
        leaderboard.loadLeaderboard();         
        while (true) {
            showMenu();
            cin >> choice;
            switch (choice) {
                case 1:
                    if (user.loginUser()) {
                        startQuiz();
                    }
                    break;
                case 2:
                    user.registerUser();
                    startQuiz();
                    break;
                case 3:
                    system("cls");
                    cout << "\t\t\t THANK YOU! \n\n";
                    return;
                case 4:
    			system("cls");
    			leaderboard.displayLeaderboard();
    			break;
    			case 5: {
    			string searchUser;
    			cout << "Enter username to search stats: ";
    			cin >> searchUser;
    			system("cls");
    			leaderboard.findUser(searchUser);
    			break;
				}
                default:
                system("cls");
                cout << "\t\t\t Please select a valid option.\n\n";
            }
        }
    }

private:
    User user;
    LeaderboardBST leaderboard;

    void showMenu() {
        cout << "\t\t\t_____________________________________________________\n\n\n";
        cout << "\t\t\t            Welcome to the Login Page                \n\n\n";
        cout << "\t\t\t________________     MENU   _________________________\n\n\n";
        cout << "                                                             \n\n";
        cout << "\t| Press 1 to LOGIN            |\n";
        cout << "\t| Press 2 to REGISTER         |\n";
        cout << "\t| Press 3 to EXIT             |\n";
   		cout << "\t| Press 4 to VIEW LEADERBOARD |\n";
   		cout << "\t| Press 5 to FIND USER STATS  |\n";
        cout << "\n\t\t\t Please Enter your Choice : ";
    }

    void startQuiz() {
        QuizLevels quiz;
        //ScoreManager manager;
        quiz.addLevel("Beginner", beginnerQuiz);
        quiz.addLevel("Intermediate", intermediateQuiz);
        quiz.addLevel("Advanced", advancedQuiz);
        quiz.playLevels();
    }
};

void checkVariableNames(const string& code, vector<string>& errors) {
    istringstream stream(code);
    string word;
    int lineNumber = 1;
    regex variablePattern(R"((int|float|double|char|string)\s+([a-zA-Z_]\w*))");

    while ( getline(stream, word)) {
         smatch matches;
        if ( regex_search(word, matches, variablePattern)) {
             string variableName = matches[2];
            if (! isalpha(variableName[0])) {
                errors.push_back("Error on line " +  to_string(lineNumber) + ": Variable '" + variableName + "' does not start with an alphabetic letter.");
            }
        }
        lineNumber++;
    }
};

void checkSemicolons(const  string& code,  vector< string>& errors) {
     istringstream stream(code);
     string word;
    int lineNumber = 1;

    while ( getline(stream, word)) {
         string trimmed = word;
        trimmed.erase(trimmed.find_last_not_of(" \n\r\t") + 1); // Trim trailing whitespace
        if (!trimmed.empty() && trimmed.back() != ';' && 
            (trimmed.find("int ") !=  string::npos || 
             trimmed.find("float ") !=  string::npos || 
             trimmed.find("double ") !=  string::npos || 
             trimmed.find("char ") !=  string::npos || 
             trimmed.find(" string ") !=  string::npos || 
             trimmed.find("return") !=  string::npos)) {
            errors.push_back("Error on line " +  to_string(lineNumber) + ": Missing semicolon.");
        }
        lineNumber++;
    }
};

void checkLibraryIncludes(const  string& code,  vector< string>& errors) {
     set< string> requiredLibraries = {"<iostream>", "<vector>", "<string>"};
     istringstream stream(code);
     string line;
     set< string> includedLibraries;
    int lineNumber = 1;

    while ( getline(stream, line)) {
         regex includePattern(R"(^\s*#include\s*(<[^>]+>|"[^"]+")\s*)");
         smatch matches;
        if ( regex_search(line, matches, includePattern)) {
            includedLibraries.insert(matches[1]);
        }
        lineNumber++;
    }

    for (const auto& lib : requiredLibraries) {
        if (includedLibraries.find(lib) == includedLibraries.end()) {
            errors.push_back("Error: Missing required library " + lib);
        }
    }
};

int main() {
    QuizApp app;
    app.run();
    return 0;
 }