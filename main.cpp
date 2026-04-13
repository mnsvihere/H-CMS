#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>
#include <cstdlib>

// Windows (Dev C++ / MinGW) uses <direct.h> and _mkdir
// Linux / Mac uses <sys/stat.h> and mkdir
#ifdef _WIN32
  #include <direct.h>
#else
  #include <sys/stat.h>
#endif

using namespace std;
string inputPassword() {
    string password = "";
    char ch;

    while (true) {
        ch = cin.get();

        if (ch == 13) break; // Enter key

        else if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else {
            password += ch;
            cout << "*";
        }
    }
    cout << endl;
    return password;
}

// -------------------------------------------------------------
//  CONSTANTS
// -------------------------------------------------------------
const int MAX_COMPLAINTS = 100;
const int MAX_USERS       = 50;

// Creates a "data" folder next to your executable (writable).
// _WIN32 is defined automatically by Dev C++ / MinGW on Windows.
string getDataDir() {
#ifdef _WIN32
    _mkdir("data");
    return string("data\\");
#else
    mkdir("data", 0755);
    return string("data/");
#endif
}

const string DATA_DIR        = getDataDir();
const string COMPLAINTS_FILE = DATA_DIR + "complaints.txt";
const string USERS_FILE      = DATA_DIR + "users.txt";


// -------------------------------------------------------------
//  UTILITY: get current date-time string
// -------------------------------------------------------------
string getCurrentTime() {
    time_t now = time(0);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// -------------------------------------------------------------
//  CUSTOM EXCEPTION CLASSES  (Exception Handling)
// -------------------------------------------------------------
class AppException {
protected:
    string message;
public:
    AppException(string msg) : message(msg) {}           // Constructor
    virtual string what() { return message; }            // Virtual function
    virtual ~AppException() {}                           // Virtual Destructor
};

class LoginException : public AppException {             // Inheritance
public:
    LoginException(string msg) : AppException(msg) {}
    string what() override { return "[LOGIN ERROR] " + message; }
};

class FileException : public AppException {
public:
    FileException(string msg) : AppException(msg) {}
    string what() override { return "[FILE ERROR] " + message; }
};

class InputException : public AppException {
public:
    InputException(string msg) : AppException(msg) {}
    string what() override { return "[INPUT ERROR] " + message; }
};

// -------------------------------------------------------------
//  ABSTRACT BASE CLASS: Entity  (Pure Virtual / Abstract Class)
// -------------------------------------------------------------
class Entity {
public:
    virtual void display() const = 0;           // Pure virtual function
    virtual void saveToFile() const = 0;        // Pure virtual function
    virtual string getID() const = 0;           // Pure virtual function
    virtual ~Entity() {}
};

// -------------------------------------------------------------
//  CLASS: Complaint  (Classes, Objects, Constructors, etc.)
// -------------------------------------------------------------
class Complaint : public Entity {               // Inheritance from Entity
private:
    string complaintID;
    string userID;
    string category;        // Plumbing, Electrical, Cleanliness, etc.
    string description;
    int    priorityLevel;   // 1=Low  2=Medium  3=High
    string status;          // Pending / In Progress / Resolved
    string currentSituation;
    string dateRegistered;
    string dateResolved;

public:
    // -- Default Constructor --
    Complaint() : priorityLevel(1), status("Pending"),
                  currentSituation("Awaiting review"),
                  dateRegistered(getCurrentTime()), dateResolved("N/A") {}

    // -- Parameterised Constructor --
    Complaint(string cid, string uid, string cat,
              string desc, int priority)
        : complaintID(cid), userID(uid), category(cat),
          description(desc), priorityLevel(priority),
          status("Pending"), currentSituation("Awaiting review"),
          dateRegistered(getCurrentTime()), dateResolved("N/A") {}

    // -- Destructor --
    ~Complaint() {}

    // -- Operator Overloading: == compares two complaints by ID --
    bool operator==(const Complaint& other) const {
        return complaintID == other.complaintID;
    }

    // -- Operator Overloading: > compares by priority --
    bool operator>(const Complaint& other) const {
        return priorityLevel > other.priorityLevel;
    }

    // -- Setters (Pass by Reference) --
    void setStatus(const string& s)           { status = s; }
    void setCurrentSituation(const string& s) { currentSituation = s; }
    void setDateResolved(const string& d)     { dateResolved = d; }
    void setPriority(int p)                   { priorityLevel = p; }

    // -- Getters --
    string getID()              const override { return complaintID; }
    string getUserID()          const { return userID; }
    string getCategory()        const { return category; }
    string getDescription()     const { return description; }
    string getStatus()          const { return status; }
    string getCurrentSituation()const { return currentSituation; }
    string getDateRegistered()  const { return dateRegistered; }
    string getDateResolved()    const { return dateResolved; }
    int    getPriority()        const { return priorityLevel; }

    string priorityLabel() const {
        if (priorityLevel == 3) return "HIGH";
        if (priorityLevel == 2) return "MEDIUM";
        return "LOW";
    }

    string statusSymbol() const {
        if (status == "Resolved")    return "[RESOLVED]  ";
        if (status == "In Progress") return "[IN PROGRESS]";
        return "[PENDING]   ";
    }

    // -- Pure Virtual Implementation: display() --
    void display() const override {
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  | Complaint ID   : " << left << setw(31) << complaintID  << "|\n";
        cout << "  | User ID        : " << left << setw(31) << userID       << "|\n";
        cout << "  | Category       : " << left << setw(31) << category     << "|\n";
        cout << "  | Priority       : " << left << setw(31) << priorityLabel()<< "|\n";
        cout << "  | Status         : " << left << setw(31) << statusSymbol()<< "|\n";
        cout << "  | Registered On  : " << left << setw(31) << dateRegistered<<"|\n";
        cout << "  | Resolved On    : " << left << setw(31) << dateResolved  << "|\n";
        cout << "  | Description    : " << left << setw(31) << description   << "|\n";
        cout << "  | Situation      : " << left << setw(31) << currentSituation<<"|\n";
        cout << "  +--------------------------------------------------+\n";
    }

    // -- Pure Virtual Implementation: saveToFile() --
    void saveToFile() const override {
        // Appends one complaint record as pipe-delimited line
        ofstream file(COMPLAINTS_FILE, ios::app);
        if (!file.is_open())
            throw FileException("Cannot open complaints file for writing.");
        file << complaintID   << "|"
             << userID        << "|"
             << category      << "|"
             << description   << "|"
             << status        << "|"
             << currentSituation << "|"
             << dateRegistered<< "|"
             << dateResolved  << "|"
             << priorityLevel << "\n";
        file.close();
    }

    // -- Load from a pipe-delimited string (static helper) --
    static Complaint fromString(const string& line) {
        Complaint c;
        istringstream ss(line);
        string token;
        string fields[9];
        int i = 0;
        while (getline(ss, token, '|') && i < 9)
            fields[i++] = token;
        c.complaintID      = fields[0];
        c.userID           = fields[1];
        c.category         = fields[2];
        c.description      = fields[3];
        c.status           = fields[4];
        c.currentSituation = fields[5];
        c.dateRegistered   = fields[6];
        c.dateResolved     = fields[7];
        c.priorityLevel    = stoi(fields[8].empty() ? "1" : fields[8]);
        return c;
    }

    // -- Friend function: print summary line (friend function) --
    friend void printSummaryLine(const Complaint& c);
};

// Friend function definition
void printSummaryLine(const Complaint& c) {
    cout << "  " << left
         << setw(14) << c.complaintID
         << setw(12) << c.userID
         << setw(16) << c.category
         << setw(14) << c.priorityLabel()
         << c.statusSymbol() << "\n";
}

// -------------------------------------------------------------
//  CLASS: User  (Inheritance, Access modes)
// -------------------------------------------------------------
class User : public Entity {
protected:
    string userID;
    string name;
    string password;
    string roomNumber;
    string contactNumber;

public:
    // Default Constructor
    User() {}

    // Parameterised Constructor
    User(string uid, string nm, string pwd, string room, string contact)
        : userID(uid), name(nm), password(pwd),
          roomNumber(room), contactNumber(contact) {}

    // Destructor
    virtual ~User() {}

    // Getters
    string getID()       const override { return userID; }
    string getName()     const { return name; }
    string getRoom()     const { return roomNumber; }
    string getContact()  const { return contactNumber; }

    // Pass by Reference: authenticate
    bool authenticate(const string& pwd) const {
        return password == pwd;
    }

    // Method Overloading: display with or without details
    void display() const override {
        cout << "  User ID   : " << userID       << "\n"
             << "  Name      : " << name         << "\n"
             << "  Room No.  : " << roomNumber   << "\n"
             << "  Contact   : " << contactNumber<< "\n";
    }

    void display(bool showAll) const {
        display();
        if (showAll)
            cout << "  Password  : [HIDDEN]\n";
    }

    void saveToFile() const override {
        ofstream file(USERS_FILE, ios::app);
        if (!file.is_open())
            throw FileException("Cannot open users file.");
        file << userID << "|" << name << "|" << password
             << "|" << roomNumber << "|" << contactNumber << "\n";
        file.close();
    }

    static User fromString(const string& line) {
        istringstream ss(line);
        string f[5]; int i = 0;
        string token;
        while (getline(ss, token, '|') && i < 5) f[i++] = token;
        return User(f[0], f[1], f[2], f[3], f[4]);
    }
};

// -------------------------------------------------------------
//  CLASS: Admin  (Inheritance from User)
// -------------------------------------------------------------
class Admin : public User {
private:
    string adminCode;

public:
    Admin() {}
    Admin(string uid, string nm, string pwd, string code)
        : User(uid, nm, pwd, "Admin Office", "0000000000"),
          adminCode(code) {}
    ~Admin() {}

    bool verifyAdminCode(const string& code) const {
        return adminCode == code;
    }

    void display() const override {
        cout << "  Admin ID  : " << userID << "\n"
             << "  Name      : " << name   << "\n"
             << "  Role      : ADMINISTRATOR\n";
    }

    // Admin does not persist via file (hardcoded for security)
    void saveToFile() const override { /* Admin is hardcoded */ }
};

// -------------------------------------------------------------
//  CLASS: ComplaintManager  (File Handling, Arrays, new/delete)
// -------------------------------------------------------------
class ComplaintManager {
private:
    Complaint* complaints[MAX_COMPLAINTS];   // Array of pointers (new/delete)
    int        count;

    // Generate unique complaint ID
    string generateID() {
        return "BH-5:2026:" + to_string( count + 1);
    }

public:
    // Constructor -- loads from file
    ComplaintManager() : count(0) {
        for (int i = 0; i < MAX_COMPLAINTS; i++) complaints[i] = nullptr;
        loadFromFile();
    }

    // Destructor -- delete all allocated objects
    ~ComplaintManager() {
        for (int i = 0; i < count; i++) {
            delete complaints[i];          // delete operator
            complaints[i] = nullptr;
        }
    }

    int getCount() const { return count; }

    // -- Add a new complaint (pass by reference) --
    void addComplaint(const string& userID, const string& category,
                      const string& description, int priority) {
        if (count >= MAX_COMPLAINTS)
            throw InputException("Complaint limit reached.");

        string cid = generateID();
        complaints[count] = new Complaint(cid, userID, category,
                                          description, priority); // new operator
        complaints[count]->saveToFile();
        count++;
        cout << "\n  [SUCCESS] Complaint registered! ID: " << cid << "\n";
    }

    // -- Update status of a complaint --
    bool updateComplaint(const string& cid, const string& newStatus,
                         const string& situation) {
        for (int i = 0; i < count; i++) {
            if (complaints[i]->getID() == cid) {
                complaints[i]->setStatus(newStatus);
                complaints[i]->setCurrentSituation(situation);
                if (newStatus == "Resolved")
                    complaints[i]->setDateResolved(getCurrentTime());
                rewriteAllToFile();
                return true;
            }
        }
        return false;
    }

    // -- Display complaints for a specific user --
    void displayUserComplaints(const string& userID) const {
        bool found = false;
        cout << "\n  ========== YOUR COMPLAINTS ==========\n";
        cout << "  " << left
             << setw(14) << "Complaint ID"
             << setw(12) << "User ID"
             << setw(16) << "Category"
             << setw(14) << "Priority"
             << "Status\n";
        cout << "  " << string(70, '-') << "\n";
        for (int i = 0; i < count; i++) {
            if (complaints[i]->getUserID() == userID) {
                printSummaryLine(*complaints[i]);
                found = true;
            }
        }
        if (!found) cout << "  No complaints found for your account.\n";
        cout << "\n";
    }

    // -- Display all complaints (admin) --
    void displayAll() const {
        cout << "\n  ========== ALL COMPLAINTS ==========\n";
        cout << "  " << left
             << setw(14) << "Complaint ID"
             << setw(12) << "User ID"
             << setw(16) << "Category"
             << setw(14) << "Priority"
             << "Status\n";
        cout << "  " << string(70, '-') << "\n";
        for (int i = 0; i < count; i++)
            printSummaryLine(*complaints[i]);
        cout << "\n  Total: " << count << " complaint(s)\n\n";
    }

    // -- Filter by status --
    void displayByStatus(const string& filterStatus) const {
        int found = 0;
        cout << "\n  ===== " << filterStatus << " COMPLAINTS =====\n";
        for (int i = 0; i < count; i++) {
            if (complaints[i]->getStatus() == filterStatus) {
                complaints[i]->display();
                found++;
            }
        }
        if (found == 0) cout << "  No " << filterStatus << " complaints.\n";
        cout << "  Found: " << found << " record(s)\n\n";
    }

    // -- View full detail of one complaint --
    bool viewDetail(const string& cid) const {
        for (int i = 0; i < count; i++) {
            if (complaints[i]->getID() == cid) {
                complaints[i]->display();
                return true;
            }
        }
        return false;
    }

    // -- Statistics summary --
    void showStats() const {
        int pending = 0, inprog = 0, resolved = 0;
        int hi = 0, med = 0, lo = 0;
        for (int i = 0; i < count; i++) {
            string s = complaints[i]->getStatus();
            if (s == "Pending")      pending++;
            else if (s == "In Progress") inprog++;
            else if (s == "Resolved")    resolved++;
            int p = complaints[i]->getPriority();
            if (p == 3) hi++;
            else if (p == 2) med++;
            else lo++;
        }
        cout << "\n  ======= COMPLAINT STATISTICS =======\n";
        cout << "  Total Complaints  : " << count     << "\n";
        cout << "  Pending           : " << pending   << "\n";
        cout << "  In Progress       : " << inprog    << "\n";
        cout << "  Resolved          : " << resolved  << "\n";
        cout << "  ---------------------------------\n";
        cout << "  High Priority     : " << hi        << "\n";
        cout << "  Medium Priority   : " << med       << "\n";
        cout << "  Low Priority      : " << lo        << "\n";
        cout << "  ====================================\n\n";
    }

    // -- Load complaints from file (File Handling) --
    void loadFromFile() {
        ifstream file(COMPLAINTS_FILE);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && count < MAX_COMPLAINTS) {
            if (line.empty()) continue;
            try {
                complaints[count] = new Complaint(Complaint::fromString(line));
                count++;
            } catch (...) { /* skip malformed lines */ }
        }
        file.close();
    }

    // -- Rewrite entire file after update --
    void rewriteAllToFile() const {
        ofstream file(COMPLAINTS_FILE, ios::trunc);
        if (!file.is_open())
            throw FileException("Cannot rewrite complaints file.");
        for (int i = 0; i < count; i++) {
            file << complaints[i]->getID()              << "|"
                 << complaints[i]->getUserID()          << "|"
                 << complaints[i]->getCategory()        << "|"
                 << complaints[i]->getDescription()     << "|"
                 << complaints[i]->getStatus()          << "|"
                 << complaints[i]->getCurrentSituation()<< "|"
                 << complaints[i]->getDateRegistered()  << "|"
                 << complaints[i]->getDateResolved()    << "|"
                 << complaints[i]->getPriority()        << "\n";
        }
        file.close();
    }
};

// -------------------------------------------------------------
//  CLASS: UserManager  (File Handling for users)
// -------------------------------------------------------------
class UserManager {
private:
    User* users[MAX_USERS];
    int   count;

    // Method Overloading: findUser by ID or by name
    int findUser(const string& id) const {
        for (int i = 0; i < count; i++)
            if (users[i]->getID() == id) return i;
        return -1;
    }

    int findUser(const string& field, bool /*byName*/) const {
        for (int i = 0; i < count; i++)
            if (users[i]->getName() == field) return i;
        return -1;
    }

public:
    UserManager() : count(0) {
        for (int i = 0; i < MAX_USERS; i++) users[i] = nullptr;
        loadFromFile();
    }

    ~UserManager() {
        for (int i = 0; i < count; i++) { delete users[i]; users[i] = nullptr; }
    }

    bool registerUser(const string& uid, const string& name,
                      const string& pwd, const string& room,
                      const string& contact) {
        if (findUser(uid) != -1)
            throw InputException("User ID '" + uid + "' already exists.");
        if (count >= MAX_USERS)
            throw InputException("User limit reached.");
        users[count] = new User(uid, name, pwd, room, contact);
        users[count]->saveToFile();
        count++;
        return true;
    }

    // Pass by Reference: returns User pointer or nullptr
    User* login(const string& uid, const string& pwd) {
        int idx = findUser(uid);
        if (idx == -1)
            throw LoginException("User ID not found.");
        if (!users[idx]->authenticate(pwd))
            throw LoginException("Incorrect password.");
        return users[idx];
    }

    bool userExists(const string& uid) const {
        return findUser(uid) != -1;
    }

    void displayAll() const {
        cout << "\n  ======= REGISTERED USERS =======\n";
        for (int i = 0; i < count; i++) users[i]->display(true);
        cout << "  Total: " << count << " user(s)\n\n";
    }

    void loadFromFile() {
        ifstream file(USERS_FILE);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && count < MAX_USERS) {
            if (line.empty()) continue;
            try {
                users[count] = new User(User::fromString(line));
                count++;
            } catch (...) {}
        }
        file.close();
    }
};

// -------------------------------------------------------------
//  UI HELPER FUNCTIONS
// -------------------------------------------------------------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    cout << "\n  Press ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void printBanner() {
    cout << "\n";
    cout << "  +=========================================================+\n";
    cout << "  |        BOYS' HOSTEL 5 COMPLAINT MANAGEMENT SYSTEM       |\n";
    cout << "  |                Version 1.0  |  C++ OOP                  |\n";
    cout << "  +=========================================================+\n\n";
}

string getInput(const string& prompt) {
    string val;
    cout << "  " << prompt;
    getline(cin, val);
    if (val.empty()) throw InputException(prompt + " cannot be empty.");
    return val;
}

string getOptionalInput(const string& prompt) {
    string val;
    cout << "  " << prompt;
    getline(cin, val);
    return val;
}

int getIntInput(const string& prompt, int lo, int hi) {
    int val;
    cout << "  " << prompt;
    if (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw InputException("Please enter a number.");
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (val < lo || val > hi)
        throw InputException("Enter a value between " + to_string(lo)
                              + " and " + to_string(hi) + ".");
    return val;
}

// -------------------------------------------------------------
//  USER MENU
// -------------------------------------------------------------
void userMenu(User* user, ComplaintManager& cm) {
    int choice = 0;
    do {
        clearScreen();
        cout << "\n";
        cout << "  +======================================+\n";
        cout << "  |   Welcome, " << left << setw(26) << user->getName() << "|\n";
        cout << "  |   Room: " << left << setw(29) << user->getRoom()   << "|\n";
        cout << "  +======================================+\n";
        cout << "  |  1. Register New Complaint           |\n";
        cout << "  |  2. View My Complaints               |\n";
        cout << "  |  3. View Complaint Detail            |\n";
        cout << "  |  4. View My Profile                  |\n";
        cout << "  |  0. Logout                           |\n";
        cout << "  +======================================+\n";
        cout << "  Enter choice: ";
        if (!(cin >> choice)) { cin.clear(); }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        try {
            if (choice == 1) {
                // -- Register complaint --
                clearScreen();
                cout << "\n  ===== REGISTER NEW COMPLAINT =====\n\n";
                cout << "  Categories:\n";
                cout << "  1. Plumbing      2. Electrical\n";
                cout << "  3. Cleanliness   4. Furniture\n";
                cout << "  5. Internet/WiFi 6. Security\n";
                cout << "  7. Food/Mess     8. Other\n";
                int cat = getIntInput("Select category (1-8): ", 1, 8);
                string categories[] = {"Plumbing","Electrical","Cleanliness",
                                       "Furniture","Internet/WiFi","Security",
                                       "Food/Mess","Other"};
                string category = categories[cat - 1];

                string desc = getInput("Describe your complaint: ");

                cout << "\n  Priority:  1=Low   2=Medium   3=High\n";
                int priority = getIntInput("Select priority (1-3): ", 1, 3);

                cm.addComplaint(user->getID(), category, desc, priority);
                pause();

            } else if (choice == 2) {
                clearScreen();
                cm.displayUserComplaints(user->getID());
                pause();

            } else if (choice == 3) {
                clearScreen();
                string cid = getInput("Enter Complaint ID: ");
                if (!cm.viewDetail(cid))
                    cout << "  [NOT FOUND] Complaint ID not found.\n";
                pause();

            } else if (choice == 4) {
                clearScreen();
                cout << "\n  ===== YOUR PROFILE =====\n";
                user->display(true);
                pause();
            }
        } catch (InputException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (FileException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (AppException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        }

    } while (choice != 0);
}

// -------------------------------------------------------------
//  ADMIN MENU
// -------------------------------------------------------------
void adminMenu(ComplaintManager& cm, UserManager& um) {
    int choice = 0;
    do {
        clearScreen();
        cout << "\n";
        cout << "  +======================================+\n";
        cout << "  |        ADMIN CONTROL PANEL           |\n";
        cout << "  +======================================+\n";
        cout << "  |  1. View All Complaints              |\n";
        cout << "  |  2. View Pending Complaints          |\n";
        cout << "  |  3. View In-Progress Complaints      |\n";
        cout << "  |  4. View Resolved Complaints         |\n";
        cout << "  |  5. View Complaint Detail            |\n";
        cout << "  |  6. Update Complaint Status          |\n";
        cout << "  |  7. View All Users                   |\n";
        cout << "  |  8. Complaint Statistics             |\n";
        cout << "  |  0. Logout                           |\n";
        cout << "  +======================================+\n";
        cout << "  Enter choice: ";
        if (!(cin >> choice)) { cin.clear(); }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        try {
            if (choice == 1) {
                clearScreen();
                cm.displayAll();
                pause();

            } else if (choice == 2) {
                clearScreen();
                cm.displayByStatus("Pending");
                pause();

            } else if (choice == 3) {
                clearScreen();
                cm.displayByStatus("In Progress");
                pause();

            } else if (choice == 4) {
                clearScreen();
                cm.displayByStatus("Resolved");
                pause();

            } else if (choice == 5) {
                clearScreen();
                string cid = getInput("Enter Complaint ID: ");
                if (!cm.viewDetail(cid))
                    cout << "  [NOT FOUND] No complaint with that ID.\n";
                pause();

            } else if (choice == 6) {
                // -- Update complaint --
                clearScreen();
                cout << "\n  ===== UPDATE COMPLAINT STATUS =====\n\n";
                string cid = getInput("Enter Complaint ID to update: ");

                cout << "\n  New Status:\n";
                cout << "  1. Pending\n";
                cout << "  2. In Progress\n";
                cout << "  3. Resolved\n";
                int sc = getIntInput("Select status (1-3): ", 1, 3);
                string statuses[] = {"Pending", "In Progress", "Resolved"};
                string newStatus  = statuses[sc - 1];

                string situation;
                if (newStatus == "Resolved") {
                    situation = "Issue has been fully resolved.";
                } else {
                    situation = getInput("Describe current situation: ");
                }

                if (cm.updateComplaint(cid, newStatus, situation))
                    cout << "\n  [SUCCESS] Complaint updated successfully!\n";
                else
                    cout << "\n  [NOT FOUND] Complaint ID not found.\n";
                pause();

            } else if (choice == 7) {
                clearScreen();
                um.displayAll();
                pause();

            } else if (choice == 8) {
                clearScreen();
                cm.showStats();
                pause();
            }

        } catch (InputException& e) {
            cout << "\n  " << e.what() << "\n"; pause();
        } catch (FileException& e) {
            cout << "\n  " << e.what() << "\n"; pause();
        } catch (AppException& e) {
            cout << "\n  " << e.what() << "\n"; pause();
        }

    } while (choice != 0);
}

// -------------------------------------------------------------
//  MAIN: Entry point with main menu
// -------------------------------------------------------------
int main() {
    // Hardcoded admin (not stored in file -- more secure)
    Admin admin("WARDEN", "Hostel Warden", "Warden@5", "SECURE2026");
    Admin clerk("Clerk", "Hostel Clerk", "Clerk@5", "SECURE2026");
    Admin p("P", "Hostel Prez", "p@5", "SECURE2026");

    // Dynamic managers using new/delete internally
    ComplaintManager cm;
    UserManager      um;

    int choice = 0;
    do {
        clearScreen();
        printBanner();
        cout << "  +======================================+\n";
        cout << "  |           MAIN MENU                  |\n";
        cout << "  +======================================+\n";
        cout << "  |  1. User Login                       |\n";
        cout << "  |  2. New User Registration            |\n";
        cout << "  |  3. Admin Login                      |\n";
        cout << "  |  0. Exit                             |\n";
        cout << "  +======================================+\n";
        cout << "  Enter choice: ";
        if (!(cin >> choice)) { cin.clear(); }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        try {
            // -- USER LOGIN --
            if (choice == 1) {
                clearScreen();
                cout << "\n  ===== USER LOGIN =====\n\n";
                string uid = getInput("Enter User ID   : ");
                cout << "  Enter Password  : ";
                string pwd = inputPassword();
                User* u = um.login(uid, pwd);
                cout << "\n  [SUCCESS] Welcome, " << u->getName() << "!\n";
                pause();
                userMenu(u, cm);
            }

            // -- NEW REGISTRATION --
            else if (choice == 2) {
                clearScreen();
                cout << "\n  ===== NEW USER REGISTRATION =====\n\n";
                string uid     = getInput("Choose a User ID  : ");
                string name    = getInput("Full Name         : ");
                //string pwd     = getInput("Set Password      : ");
                cout << "  Set Password  : ";
                string pwd = inputPassword();
               cout << " Confirm Password  : ";
                string confirm = inputPassword();
                if (pwd != confirm)
                    throw InputException("Passwords do not match.");
                string room    = getInput("Room Number       : ");
                string contact = getInput("Contact Number    : ");

                um.registerUser(uid, name, pwd, room, contact);
                cout << "\n  [SUCCESS] Registration complete! You can now login.\n";
                pause();
            }

            // -- ADMIN LOGIN --
            else if (choice == 3) {
                clearScreen();
                cout << "\n  ===== ADMIN LOGIN =====\n\n";
                string uid  = getInput("Admin ID   : ");
                cout << "  Enter Password  : ";
                string pwd = inputPassword();
                string code = getInput("Admin Code : ");

                Admin* current = nullptr;

            // Identify who is logging in
            if (uid == admin.getID())
                 current = &admin;
            else if (uid == clerk.getID())
                 current = &clerk;
            else if (uid == p.getID())
                 current = &p;
            else
            throw LoginException("Invalid ID.");

// Authenticate that specific person
            if (!current->authenticate(pwd))
            throw LoginException("Invalid password.");

             cout << "\n  [SUCCESS] " << current->getName() << " access granted.\n";
            pause();
            adminMenu(cm, um);
                pause();
                adminMenu(cm, um);
            }

            else if (choice != 0) {
                cout << "\n  Invalid option. Try again.\n";
                pause();
            }

        } catch (LoginException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (InputException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (FileException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (AppException& e) {
            cout << "\n  " << e.what() << "\n";
            pause();
        } catch (...) {
            cout << "\n  [ERROR] An unexpected error occurred.\n";
            pause();
        }

    } while (choice != 0);

    clearScreen();
    printBanner();
    cout << "  Thank you for using Hostel CMS. Have a Good Day!\n\n";
    return 0;
}
