#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>  // for isdigit, isalpha

using namespace std;

// ===== Function to clear screen =====
void clearScreen() {
#ifdef _WIN32
    system("cls");   // Windows
#else
    system("clear"); // Linux / Mac
#endif
}

// ===== Validation Helpers =====
bool isValidISBN(const string &isbn) {
    if (isbn.empty()) return false;
    bool hasDigit = false;
    for (char c : isbn) {
        if (!isdigit(c) && c != '-') return false; // only digits and '-'
        if (isdigit(c)) hasDigit = true; // ensure at least one digit
    }
    return hasDigit;
}

bool isValidName(const string &name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(c) && !isspace(c) && c != '.' && c != '&' && c != '-' && c != '\'')
            return false;
    }
    return true;
}

// ===== Book Class =====
class Book {
    string title, author, isbn;
    bool available;
public:
    Book(string t, string a, string i, bool avail = true)
        : title(t), author(a), isbn(i), available(avail) {}

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }
    void setAvailable(bool status) { available = status; }

    string toFileString() const {
        return title + "|" + author + "|" + isbn + "|" + (available ? "1" : "0");
    }
};

// ===== User Class =====
class LibraryUser {
    string userID, name;
    vector<string> borrowedBooks;
public:
    LibraryUser(string id, string n) : userID(id), name(n) {}

    string getUserID() const { return userID; }
    string getName() const { return name; }
    vector<string> getBorrowedBooks() const { return borrowedBooks; }

    void borrowBook(string isbn) { borrowedBooks.push_back(isbn); }
    void returnBook(string isbn) {
        borrowedBooks.erase(remove(borrowedBooks.begin(), borrowedBooks.end(), isbn), borrowedBooks.end());
    }

    string toFileString() const {
        string books;
        for (auto &b : borrowedBooks) books += b + ",";
        return userID + "|" + name + "|" + books;
    }
};

// ===== Borrow/Return Record =====
struct Record {
    string userID;
    string isbn;
    string action; // "Borrowed" or "Returned"
};

// ===== Library Class =====
class Library {
    vector<Book> books;
    vector<LibraryUser> users;
    vector<Record> history; // store borrow/return history

    void loadBooks() {
        ifstream file("books.txt");
        string title, author, isbn, status;
        while (getline(file, title, '|') && getline(file, author, '|') && getline(file, isbn, '|') && getline(file, status)) {
            books.emplace_back(title, author, isbn, status == "1");
        }
    }

    void loadUsers() {
        ifstream file("users.txt");
        string id, name, borrowed;
        while (getline(file, id, '|') && getline(file, name, '|') && getline(file, borrowed)) {
            LibraryUser user(id, name);
            size_t pos = 0;
            while ((pos = borrowed.find(',')) != string::npos) {
                string book = borrowed.substr(0, pos);
                if (!book.empty()) user.borrowBook(book);
                borrowed.erase(0, pos + 1);
            }
            users.push_back(user);
        }
    }

    void saveBooks() {
        ofstream file("books.txt");
        for (auto &b : books) file << b.toFileString() << "\n";
    }

    void saveUsers() {
        ofstream file("users.txt");
        for (auto &u : users) file << u.toFileString() << "\n";
    }

public:
    Library() { loadBooks(); loadUsers(); }
    ~Library() { saveBooks(); saveUsers(); }

    void addBook(string t, string a, string i) { books.emplace_back(t, a, i); }
    void registerUser(string id, string n) { users.emplace_back(id, n); }

    void borrowBook(string isbn, string userID) {
        for (auto &b : books) {
            if (b.getISBN() == isbn && b.isAvailable()) {
                b.setAvailable(false);
                for (auto &u : users) {
                    if (u.getUserID() == userID) {
                        u.borrowBook(isbn);
                        history.push_back({userID, isbn, "Borrowed"});
                        cout << "Book borrowed successfully!\n";
                        return;
                    }
                }
            }
        }
        cout << "Book not available or user not found.\n";
    }

    void returnBook(string isbn, string userID) {
        for (auto &u : users) {
            if (u.getUserID() == userID) {
                u.returnBook(isbn);
                for (auto &b : books) {
                    if (b.getISBN() == isbn) {
                        b.setAvailable(true);
                        history.push_back({userID, isbn, "Returned"});
                        cout << "Book returned successfully!\n";
                        return;
                    }
                }
            }
        }
        cout << "Return failed.\n";
    }

    void displayAllBooks() {
        clearScreen();
        cout << "\nBooks in Library:\n";
        cout << "--------------------------------------------------------------------------------------\n";
        cout << "| " << left << setw(30) << "Title"
             << "| " << left << setw(20) << "Author"
             << "| " << left << setw(13) << "ISBN"
             << "| " << left << setw(10) << "Status" << "|\n";
        cout << "--------------------------------------------------------------------------------------\n";

        for (auto &b : books) {
            cout << "| " << left << setw(30) << b.getTitle()
                 << "| " << left << setw(20) << b.getAuthor()
                 << "| " << left << setw(13) << b.getISBN()
                 << "| " << left << setw(10) << (b.isAvailable() ? "Available" : "Borrowed") << "|\n";
        }

        cout << "--------------------------------------------------------------------------------------\n";
    }

    void displayAllUsers() {
        clearScreen();
        cout << "\nLibrary Users:\n";
        cout << "---------------------------------------------------------------------\n";
        cout << "| " << left << setw(10) << "User ID"
             << "| " << left << setw(20) << "Name"
             << "| " << left << setw(29) << "Borrowed Books" << "|\n";
        cout << "---------------------------------------------------------------------\n";

        for (auto &u : users) {
            string borrowedList;
            for (auto &isbn : u.getBorrowedBooks()) borrowedList += isbn + " ";
            if (borrowedList.empty()) borrowedList = "None";

            cout << "| " << left << setw(10) << u.getUserID()
                 << "| " << left << setw(20) << u.getName()
                 << "| " << left << setw(29) << borrowedList << "|\n";
        }

        cout << "---------------------------------------------------------------------\n";
    }

    void deleteBook(string isbn) {
        auto it = remove_if(books.begin(), books.end(),
                            [&](Book &b) { return b.getISBN() == isbn; });
        if (it != books.end()) {
            books.erase(it, books.end());
            cout << "Book deleted successfully!\n";
        } else {
            cout << "Book not found!\n";
        }
    }

    void deleteUser(string userID) {
        auto it = remove_if(users.begin(), users.end(),
                            [&](LibraryUser &u) { return u.getUserID() == userID; });
        if (it != users.end()) {
            users.erase(it, users.end());
            cout << "User deleted successfully!\n";
        } else {
            cout << "User not found!\n";
        }
    }

    void displayHistory() {
        clearScreen();
        cout << "\nBorrowed/Returned Books History:\n";
        cout << "-------------------------------------------------------\n";
        cout << "| " << setw(10) << "User ID"
             << "| " << setw(15) << "ISBN"
             << "| " << setw(12) << "Action" << "|\n";
        cout << "-------------------------------------------------------\n";

        for (auto &rec : history) {
            cout << "| " << setw(10) << rec.userID
                 << "| " << setw(15) << rec.isbn
                 << "| " << setw(12) << rec.action << "|\n";
        }

        cout << "-------------------------------------------------------\n";
    }
};

// ===== Main Menu =====
int main() {
    Library library;
    int choice;
    string t, a, i, id, n;

    do {
        cout << "\nLibrary Management System\n";
        cout << "1. Add Book\n2. Register User\n3. Borrow Book\n4. Return Book\n";
        cout << "5. Display All Books\n6. Display All Users\n";
        cout << "7. Delete Book\n8. Delete User\n";
        cout << "9. Display Borrowed/Returned Books History\n";
        cout << "0. Exit\nChoice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input! Please enter a number.\n";
            continue;
        }

        if (choice < 0 || choice > 9) {
            cout << "Invalid choice! Please select between 0 and 9.\n";
            continue;
        }

        cin.ignore();

        switch (choice) {
        case 1:
            cout << "Title: "; getline(cin, t);

            do {
                cout << "Author : "; getline(cin, a);
                if (!isValidName(a)) cout << "Invalid input! Please enter a valid name.\n";
            } while (!isValidName(a));

            do {
                cout << "ISBN : "; getline(cin, i);
                if (!isValidISBN(i)) cout << "Invalid ISBN! Please use digits and dashes only.\n";
            } while (!isValidISBN(i));

            library.addBook(t, a, i);
            break;
        case 2:
            cout << "User ID: "; getline(cin, id);
            do {
                cout << "Name : "; getline(cin, n);
                if (!isValidName(n)) cout << "Invalid input! Please enter a valid name.\n";
            } while (!isValidName(n));

            library.registerUser(id, n);
            break;
        case 3:
            do {
                cout << "ISBN : "; getline(cin, i);
                if (!isValidISBN(i)) cout << "Invalid ISBN! Please use digits and dashes only.\n";
            } while (!isValidISBN(i));
            cout << "User ID: "; getline(cin, id);
            library.borrowBook(i, id);
            break;
        case 4:
            do {
                cout << "ISBN: "; getline(cin, i);
                if (!isValidISBN(i)) cout << "Invalid ISBN! Please use digits and dashes only.\n";
            } while (!isValidISBN(i));
            cout << "User ID: "; getline(cin, id);
            library.returnBook(i, id);
            break;
        case 5:
            library.displayAllBooks();
            cout << "Press Enter to go back to menu..."; cin.ignore();
            break;
        case 6:
            library.displayAllUsers();
            cout << "Press Enter to go back to menu..."; cin.ignore();
            break;
        case 7:
            do {
                cout << "Enter ISBN of the book to delete : ";
                getline(cin, i);
                if (!isValidISBN(i)) cout << "Invalid ISBN! Please use digits and dashes only.\n";
            } while (!isValidISBN(i));
            library.deleteBook(i);
            break;
        case 8:
            cout << "Enter User ID to delete: ";
            getline(cin, id);
            library.deleteUser(id);
            break;
        case 9:
            library.displayHistory();
            cout << "Press Enter to go back to menu..."; cin.ignore();
            break;
        }
    } while (choice != 0);

    return 0;
}
