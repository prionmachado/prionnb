#include <iostream>    //for cin and cout
#include <fstream>    //file handling //ifstream for reading and ofstream for writing 
#include <vector>     //Dynamic arrays used of storing contacts
#include <string>     // for string operations
#include <iomanip>    //for input/output manipulation, like formatting //setw() for setting width of the output
#include <algorithm>  // for searching,sorting and deleting
#include <regex>    //matching the pattern of the string
using namespace std;

struct Contact{
    string name;
    string phone;
    string email;
};
const string FILE_NAME = "contacts.txt";
const string CSV_FILE_NAME = "contacts.csv";

vector<Contact> lastBackup; // Stores last saved state
vector<Contact> loadContacts();
void addContact();          // function prototypes
void viewContacts();
void searchContact();
void editContact();
void deleteContact();
void sortContacts();
void saveContacts(const vector<Contact> &contacts);
bool isValidPhone(const string &phone);
bool isValidEmail(const string &email);
bool isDuplicateContact(const vector<Contact> &contacts, const Contact &newContact);
void exportToCSV(const vector<Contact> &contacts);
void autoSave();
void undoLastChange();
void backupContacts();

int main(){
    int choice;
    vector<Contact> contacts = loadContacts();
    while (true){
        cout << "\n============ Contact Manager ============\n";
        cout << "1. Add Contact\n";
        cout << "2. View Contacts\n";
        cout << "3. Search Contact\n";
        cout << "4. Edit Contact\n";
        cout << "5. Delete Contact\n";
        cout << "6. Sort Contacts by Name\n";
        cout << "7. Undo Last Change\n";
        cout << "8. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        switch (choice){
        case 1:backupContacts();addContact();break;
        case 2:backupContacts();viewContacts();break;
        case 3:backupContacts();searchContact();break;
        case 4:backupContacts();editContact();break;
        case 5:backupContacts();deleteContact();break;
        case 6:backupContacts();sortContacts();break;
        case 7:undoLastChange();break;
        case 8:autoSave();return 0;
        default: cout << "Invalid choice! Try again.\n";
        }
    } 
}
void backupContacts() {          // Backs up contacts
    lastBackup = loadContacts();
}

void addContact()  {
    Contact newContact;
    vector<Contact> contacts = loadContacts();   

    cout << "Enter Name: ";
    getline(cin, newContact.name);

    do {
        cout << "Enter Phone (10 digits only): ";
        getline(cin, newContact.phone);
    } while (!isValidPhone(newContact.phone));

    do {
        cout << "Enter Email (@gmail.com only): ";
        getline(cin, newContact.email);
    } while (!isValidEmail(newContact.email));

    if (isDuplicateContact(contacts, newContact)) {
        cout << "Error: Duplicate contact exists!\n";
    } else {
        contacts.push_back(newContact);
        saveContacts(contacts);
        exportToCSV(contacts);
        cout << "Contact added successfully!\n";
    } } 

void viewContacts() {
    vector<Contact> contacts = loadContacts();
    if (contacts.empty()) {
        cout << "No contacts available.\n";
        return;
    } 
    cout << "\n--------------------------------------------------------------------------\n";
    cout << left << setw(20) << "Name" << setw(15) << "Phone" << setw(35) << "Email" << "\n";   //set width 
    cout << "--------------------------------------------------------------------------\n";
    for (const auto &contact : contacts) {
        cout << left << setw(20) << contact.name << setw(15) << contact.phone << setw(35) << contact.email << "\n";
    }
    cout << "--------------------------------------------------------------------------\n";
} 

void saveContacts(const vector<Contact> &contacts) {
    ofstream file(FILE_NAME);     //ofstream for writing   //ifstream for reading
    if (!file) {
        cout << "Error: Unable to open file!\n";
        return;
    }
    for (const auto &contact : contacts) {
        file << contact.name << "\n" << contact.phone << "\n" << contact.email << "\n";
    }
    file.close();
} 

vector<Contact> loadContacts() { // Loads contacts from a file
    vector<Contact> contacts;   //empty vector
    ifstream file(FILE_NAME);   
    if (!file) {
        cout << "Error: Unable to open contacts file!\n";
        return {};
    }
    Contact contact;
    while (getline(file, contact.name) && getline(file, contact.phone) && getline(file, contact.email)){
        contacts.push_back(contact);
    }
    file.close();
    return contacts;
} 

void editContact() {
    vector<Contact> contacts = loadContacts();
    string name;
    cout << "Enter the name of the contact to edit: ";
    getline(cin, name);
    for (auto &contact : contacts) {
        if (contact.name == name) {
            cout << "Enter new Name: ";
            getline(cin, contact.name);
            do {
                cout << "Enter new Phone (10 digits only): ";
                getline(cin, contact.phone);
            } while (!isValidPhone(contact.phone));
            do {
                cout << "Enter new Email (@gmail.com only): ";
                getline(cin, contact.email);
            } while (!isValidEmail(contact.email));
            saveContacts(contacts);
            exportToCSV(contacts);
            cout << "Contact updated successfully!\n";
            return;
        }
    }  cout << "Contact not found!\n";
} 

void searchContact() {
    vector<Contact> contacts = loadContacts();
    string name;
    cout << "Enter name to search: ";
    getline(cin, name);
    for (const auto &contact : contacts) {
        if (contact.name == name) {
            cout << "\n----------------------------------------------------------------\n";
            cout << "Name: " << contact.name << "\nPhone: " << contact.phone << "\nEmail: " << contact.email << "\n";
            cout << "----------------------------------------------------------------\n";
            return;
        }
    }  cout << "Contact not found!\n";
} 

void deleteContact() {
    vector<Contact> contacts = loadContacts();
    string name;
    cout << "Enter name to delete: ";
    getline(cin, name);
    auto newEnd = remove_if(contacts.begin(), contacts.end(), [&](const Contact &contact){ return contact.name == name; });
    if (newEnd == contacts.end()) {
        cout << "Contact not found! No changes were made.\n";
        return;
    }
    contacts.erase(newEnd, contacts.end());

    saveContacts(contacts);
    exportToCSV(contacts);
    cout << "Contact deleted successfully!\n";
} 

void sortContacts() {
    vector<Contact> contacts = loadContacts();
    sort(contacts.begin(), contacts.end(), [](const Contact &a, const Contact &b)
         { return a.name < b.name; });
    saveContacts(contacts);
    exportToCSV(contacts);
    cout << "Contacts sorted successfully!\n";
} 

void exportToCSV(const vector<Contact> &contacts) {
    ofstream file(CSV_FILE_NAME); 
    file << "Name,Phone,Email\n";
    for (const auto &contact : contacts) {
        file << contact.name << "," << contact.phone << "," << contact.email << "\n";
    }
    file.close();
}

bool isValidPhone(const string &phone) {
    return regex_match(phone, regex("^[0-9]{10}$"));
} 

bool isValidEmail(const string &email) {
    return regex_match(email, regex(R"(^[\w.]+@xaviers\.edu\.in$)")) || regex_match(email, regex(R"(^[\w]+@gmail\.(com|edu|in)$)")); 
}     

bool isDuplicateContact(const vector<Contact> &contacts, const Contact &newContact) {
    for (const auto &contact : contacts) {
        if (contact.name == newContact.name || contact.phone == newContact.phone) {
            return true;
        }
    }  return false;
} 

void undoLastChange() {
    if (lastBackup.empty()) {
        cout << "No backup found. Cannot undo last change.\n";
        return;
    }
    saveContacts(lastBackup);
    exportToCSV(lastBackup);
    cout << "Last change undone successfully!\n";
} 

void autoSave() {
    vector<Contact> contacts = loadContacts();
    saveContacts(contacts);
    exportToCSV(contacts);
    cout << "Contacts auto-saved before exit.\n";
} 