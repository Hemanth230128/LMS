#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>

using namespace std;

const int STUDENT_MAX_BORROW = 3;
const int STUDENT_BORROW_PERIOD = 15;
const int STUDENT_FINE_PER_DAY = 10;

const int FACULTY_MAX_BORROW = 5;
const int FACULTY_BORROW_PERIOD = 30;
const int FACULTY_OVERDUE_LIMIT = 60;

struct BorrowRecord
{
    int bookId;
    time_t borrowTime;
};

// Account Class
class Account
{
private:
    vector<BorrowRecord> borrowedRecords;
    double fine;

public:
    Account() : fine(0) {}

    vector<BorrowRecord> &getRecords() { return borrowedRecords; }
    const vector<BorrowRecord> &getRecords() const { return borrowedRecords; }
    double getFine() const { return fine; }

    void addRecord(const BorrowRecord &rec) { borrowedRecords.push_back(rec); }
    bool removeRecord(int bookId)
    {
        for (auto it = borrowedRecords.begin(); it != borrowedRecords.end(); ++it)
        {
            if (it->bookId == bookId)
            {
                borrowedRecords.erase(it);
                return true;
            }
        }
        return false;
    }
    void addFine(double amount) { fine += amount; }
    void clearFine() { fine = 0; }
};

// Book Class
class Book
{
private:
    int id;
    string title;
    string author;
    string publisher;
    string isbn;
    int year;
    string status;
    int reservedBy;

public:
    Book() : id(0), year(0), status("Available"), reservedBy(-1) {}
    Book(int id, string title, string author, string publisher, int year, string isbn, string status = "Available", int reservedBy = -1)
        : id(id), title(title), author(author), publisher(publisher), isbn(isbn), year(year), status(status), reservedBy(reservedBy) {}

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    int getYear() const { return year; }
    string getIsbn() const { return isbn; }
    string getStatus() const { return status; }
    int getReservedBy() const { return reservedBy; }

    void setTitle(const string &t) { title = t; }
    void setStatus(const string &s) { status = s; }
    void setReservedBy(int uid) { reservedBy = uid; }
    void clearReservation() { reservedBy = -1; }

    void display() const
    {
        cout << id << ": " << title << " by " << author << " (" << year << ") - " << status;
        if (reservedBy != -1)
            cout << " [Reserved by User ID " << reservedBy << "]";
        cout << endl;
    }
};

// Forward Declaration of Library
class Library;

// User Base Class
class User
{
private:
    int id;
    string username;
    string password;
    string name;
    string role;
    Account account;

public:
    User() : id(0) {}
    User(int id, string username, string password, string name, string role)
        : id(id), username(username), password(password), name(name), role(role) {}

    int getId() const { return id; }
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getName() const { return name; }
    string getRole() const { return role; }
    Account &getAccount() { return account; }
    const Account &getAccount() const { return account; }

    virtual void dashboard(Library &lib) = 0;
    virtual ~User() {}
};

// Derived Classes
class Student : public User
{
public:
    Student(int id, string username, string password, string name)
        : User(id, username, password, name, "Student") {}
    void dashboard(Library &lib) override;
};

class Faculty : public User
{
public:
    Faculty(int id, string username, string password, string name)
        : User(id, username, password, name, "Faculty") {}
    void dashboard(Library &lib) override;
};

class Librarian : public User
{
public:
    Librarian(int id, string username, string password, string name)
        : User(id, username, password, name, "Librarian") {}
    void dashboard(Library &lib) override;
};

// Library Class
class Library
{
private:
    vector<Book> books;
    vector<User *> users;

    // Save Borrow records and fines to a file
    void saveLoanData()
    {
        ofstream loanOut("./data/fines.txt");
        if (!loanOut)
        {
            cout << "Error: Cannot open fines.txt for writing.\n";
            return;
        }
        for (auto user : users)
        {
            for (auto &rec : user->getAccount().getRecords())
            {
                loanOut << user->getId() << "," << rec.bookId << "," << rec.borrowTime << "\n";
            }
            loanOut << user->getId() << ",FINE," << user->getAccount().getFine() << "\n";
        }
        loanOut.close();
    }

    void loadLoanData()
    {
        ifstream loanFile("./data/fines.txt");
        if (!loanFile)
        {
            cout << "fines.txt not found. Starting with empty loan records.\n";
            return;
        }
        string line;
        while (getline(loanFile, line))
        {
            if (line.empty())
                continue;
            istringstream iss(line);
            vector<string> tokens;
            string token;
            while (getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (tokens.size() < 3)
                continue;
            int userId = stoi(tokens[0]);
            User *foundUser = nullptr;
            for (auto u : users)
            {
                if (u->getId() == userId)
                {
                    foundUser = u;
                    break;
                }
            }
            if (foundUser == nullptr)
                continue;
            if (tokens[1] == "FINE")
            {
                foundUser->getAccount().clearFine();
                foundUser->getAccount().addFine(stod(tokens[2]));
            }
            else
            {
                BorrowRecord rec;
                rec.bookId = stoi(tokens[1]);
                rec.borrowTime = (time_t)stoll(tokens[2]);
                foundUser->getAccount().getRecords().push_back(rec);
            }
        }
        loanFile.close();
    }

public:
    Library() {}
    ~Library()
    {
        for (auto u : users)
            delete u;
    }

    vector<Book> &getBooks() { return books; }
    vector<User *> &getUsers() { return users; }

    void displayAvailableBooksForUser(int userId)
    {
        cout << "\nAvailable Books:\n";
        for (auto &book : books)
        {
            if (book.getStatus() == "Available" ||
                (book.getStatus() == "Reserved" && book.getReservedBy() == userId))
                book.display();
        }
    }

    void displayAvailableBooks()
    {
        cout << "\nAvailable Books:\n";
        for (auto &book : books)
        {
            if (book.getStatus() == "Available")
                book.display();
        }
    }

    Book *findBookById(int id)
    {
        for (auto &book : books)
        {
            if (book.getId() == id)
                return &book;
        }
        return nullptr;
    }

    bool bookExists(int bid)
    {
        for (auto &book : books)
        {
            if (book.getId() == bid)
                return true;
        }
        return false;
    }

    bool userExists(int uid)
    {
        for (auto user : users)
        {
            if (user->getId() == uid)
                return true;
        }
        return false;
    }

    void loadData()
    {
        // Load books
        ifstream infile("./data/books.txt");
        if (infile)
        {
            string line;
            while (getline(infile, line))
            {
                if (line.empty())
                    continue;
                istringstream iss(line);
                vector<string> tokens;
                string token;
                while (getline(iss, token, ','))
                {
                    tokens.push_back(token);
                }
                if (tokens.size() == 8)
                {
                    int id = stoi(tokens[0]);
                    int year = stoi(tokens[4]);
                    int reserved = stoi(tokens[7]);
                    books.push_back(Book(id, tokens[1], tokens[2], tokens[3], year, tokens[5], tokens[6], reserved));
                }
                else if (tokens.size() == 7)
                {
                    int id = stoi(tokens[0]);
                    int year = stoi(tokens[4]);
                    books.push_back(Book(id, tokens[1], tokens[2], tokens[3], year, tokens[5], tokens[6], -1));
                }
            }
            infile.close();
        }
        else
        {
            cout << "books.txt not found. Starting with empty book list.\n";
        }

        // Load users
        ifstream ufile("./data/users.txt");
        if (ufile)
        {
            string line;
            while (getline(ufile, line))
            {
                if (line.empty())
                    continue;
                istringstream iss(line);
                vector<string> tokens;
                string token;
                while (getline(iss, token, ','))
                {
                    tokens.push_back(token);
                }
                if (tokens.size() != 5)
                    continue;
                int id = stoi(tokens[0]);
                string urole = tokens[3];
                if (urole == "Student")
                    users.push_back(new Student(id, tokens[1], tokens[2], tokens[4]));
                else if (urole == "Faculty")
                    users.push_back(new Faculty(id, tokens[1], tokens[2], tokens[4]));
                else if (urole == "Librarian")
                    users.push_back(new Librarian(id, tokens[1], tokens[2], tokens[4]));
            }
            ufile.close();
        }
        else
        {
            cout << "users.txt not found. Starting with empty user list.\n";
        }

        loadLoanData();
    }

    void saveData()
    {
        ofstream outfile("./data/books.txt");
        if (outfile)
        {
            for (auto &book : books)
            {
                outfile << book.getId() << "," << book.getTitle() << "," << book.getAuthor() << ","
                        << book.getPublisher() << "," << book.getYear() << "," << book.getIsbn() << ","
                        << book.getStatus() << "," << book.getReservedBy() << "\n";
            }
            outfile.close();
        }

        ofstream uoutfile("./data/users.txt");
        if (uoutfile)
        {
            for (auto user : users)
            {
                uoutfile << user->getId() << "," << user->getUsername() << "," << user->getPassword() << ","
                         << user->getRole() << "," << user->getName() << "\n";
            }
            uoutfile.close();
        }
        saveLoanData();
    }
};

// Student Dashboard
void Student::dashboard(Library &lib)
{
    int choice;
    do
    {
        cout << "\n--- STUDENT DASHBOARD (" << getName() << ") ---\n";
        cout << "1. View Available Books\n";
        cout << "2. Borrow a Book\n";
        cout << "3. Return a Book\n";
        cout << "4. View Borrowed Books\n";
        cout << "5. Check Fine Amount\n";
        cout << "6. Pay Fine\n";
        cout << "7. Reserve a Book\n";
        cout << "8. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1)
        {
            lib.displayAvailableBooksForUser(getId());
        }
        else if (choice == 2)
        {
            if (getAccount().getRecords().size() >= STUDENT_MAX_BORROW)
            {
                cout << "Borrowing limit reached.\n";
            }
            else if (getAccount().getFine() > 0)
            {
                cout << "Outstanding fine. Please clear your fine before borrowing.\n";
            }
            else
            {
                int bid;
                cout << "Enter Book ID to borrow: ";
                cin >> bid;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                Book *book = lib.findBookById(bid);
                if (book)
                {
                    if (book->getStatus() == "Available" ||
                        (book->getStatus() == "Reserved" && book->getReservedBy() == getId()))
                    {
                        book->setStatus("Borrowed");
                        book->clearReservation();
                        BorrowRecord rec;
                        rec.bookId = bid;
                        rec.borrowTime = time(0);
                        getAccount().addRecord(rec);
                        cout << "Book borrowed successfully.\n";
                    }
                    else
                    {
                        cout << "Book is not available for borrowing.\n";
                    }
                }
                else
                {
                    cout << "Book not found.\n";
                }
            }
        }
        else if (choice == 3)
        {
            int bid;
            cout << "Enter Book ID to return: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            bool found = false;
            auto &records = getAccount().getRecords();
            for (auto it = records.begin(); it != records.end(); ++it)
            {
                if (it->bookId == bid)
                {
                    time_t now = time(0);
                    double days = difftime(now, it->borrowTime) / (10);
                    if (days > STUDENT_BORROW_PERIOD)
                    {
                        int overdue = days - STUDENT_BORROW_PERIOD;
                        getAccount().addFine(overdue * STUDENT_FINE_PER_DAY);
                        cout << "Book returned overdue by " << overdue
                             << " days. Fine incurred: " << overdue * STUDENT_FINE_PER_DAY << " rupees.\n";
                    }
                    else
                    {
                        cout << "Book returned on time.\n";
                    }
                    records.erase(it);
                    Book *book = lib.findBookById(bid);
                    if (book)
                    {
                        if (book->getReservedBy() != -1)
                            book->setStatus("Reserved");
                        else
                            book->setStatus("Available");
                    }
                    found = true;
                    break;
                }
            }
            if (!found)
                cout << "Book not found in your borrowed list.\n";
        }
        else if (choice == 4)
        {
            cout << "Your Borrowed Books:\n";
            for (auto &rec : getAccount().getRecords())
            {
                cout << "Book ID: " << rec.bookId << " Borrowed on: " << ctime(&rec.borrowTime);
            }
        }
        else if (choice == 5)
        {
            cout << "Total fine: " << getAccount().getFine() << " rupees.\n";
        }
        else if (choice == 6)
        {
            if (getAccount().getFine() > 0)
            {
                cout << "Paying fine of " << getAccount().getFine() << " rupees. Fine cleared.\n";
                getAccount().clearFine();
            }
            else
            {
                cout << "No fine to pay.\n";
            }
        }
        else if (choice == 7)
        {
            int bid;
            cout << "Enter Book ID to reserve: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Book *book = lib.findBookById(bid);
            if (book)
            {
                if (book->getStatus() == "Borrowed" && book->getReservedBy() == -1)
                {
                    book->setReservedBy(getId());
                    cout << "Book reserved successfully. Upon return, it will be available only for you.\n";
                }
                else if (book->getReservedBy() != -1)
                {
                    cout << "Book is already reserved.\n";
                }
                else
                {
                    cout << "Book is available. No need to reserve.\n";
                }
            }
            else
            {
                cout << "Book not found.\n";
            }
        }
        lib.saveData();
    } while (choice != 8);
}

// Faculty Dashboard
void Faculty::dashboard(Library &lib)
{
    int choice;
    do
    {
        cout << "\n--- FACULTY DASHBOARD (" << getName() << ") ---\n";
        cout << "1. View Available Books\n";
        cout << "2. Borrow a Book\n";
        cout << "3. Return a Book\n";
        cout << "4. View Borrowed Books\n";
        cout << "5. Check Borrowing Status\n";
        cout << "6. Reserve a Book\n";
        cout << "7. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1)
        {
            lib.displayAvailableBooksForUser(getId());
        }
        else if (choice == 2)
        {
            bool canBorrow = true;
            time_t now = time(0);
            for (auto &rec : getAccount().getRecords())
            {
                double days = difftime(now, rec.borrowTime) / (10);
                if (days > FACULTY_OVERDUE_LIMIT)
                {
                    canBorrow = false;
                    break;
                }
            }
            if (!canBorrow)
            {
                cout << "One of your loans is overdue by more than 60 days. Cannot borrow new books.\n";
            }
            else if (getAccount().getRecords().size() >= FACULTY_MAX_BORROW)
            {
                cout << "Borrowing limit reached.\n";
            }
            else
            {
                int bid;
                cout << "Enter Book ID to borrow: ";
                cin >> bid;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                Book *book = lib.findBookById(bid);
                if (book)
                {
                    if (book->getStatus() == "Available" ||
                        (book->getStatus() == "Reserved" && book->getReservedBy() == getId()))
                    {
                        book->setStatus("Borrowed");
                        book->clearReservation();
                        BorrowRecord rec;
                        rec.bookId = bid;
                        rec.borrowTime = time(0);
                        getAccount().addRecord(rec);
                        cout << "Book borrowed successfully.\n";
                    }
                    else
                    {
                        cout << "Book is not available for borrowing.\n";
                    }
                }
                else
                {
                    cout << "Book not found.\n";
                }
            }
        }
        else if (choice == 3)
        {
            int bid;
            cout << "Enter Book ID to return: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            bool found = false;
            auto &records = getAccount().getRecords();
            for (auto it = records.begin(); it != records.end(); ++it)
            {
                if (it->bookId == bid)
                {
                    time_t now = time(0);
                    double days = difftime(now, it->borrowTime) / (10);
                    if (days > FACULTY_BORROW_PERIOD)
                    {
                        cout << "Book returned late. Overdue by " << (int)(days - FACULTY_BORROW_PERIOD) << " days.\n";
                    }
                    else
                    {
                        cout << "Book returned on time.\n";
                    }
                    records.erase(it);
                    Book *book = lib.findBookById(bid);
                    if (book)
                    {
                        if (book->getReservedBy() != -1)
                            book->setStatus("Reserved");
                        else
                            book->setStatus("Available");
                    }
                    found = true;
                    break;
                }
            }
            if (!found)
                cout << "Book not found in your borrowed list.\n";
        }
        else if (choice == 4)
        {
            cout << "Your Borrowed Books:\n";
            for (auto &rec : getAccount().getRecords())
            {
                cout << "Book ID: " << rec.bookId << " Borrowed on: " << ctime(&rec.borrowTime);
            }
        }
        else if (choice == 5)
        {
            cout << "Currently borrowed: " << getAccount().getRecords().size() << " books.\n";
        }
        else if (choice == 6)
        {
            int bid;
            cout << "Enter Book ID to reserve: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Book *book = lib.findBookById(bid);
            if (book)
            {
                if (book->getStatus() == "Borrowed" && book->getReservedBy() == -1)
                {
                    book->setReservedBy(getId());
                    cout << "Book reserved successfully. Upon return, it will be available only for you.\n";
                }
                else if (book->getReservedBy() != -1)
                {
                    cout << "Book is already reserved.\n";
                }
                else
                {
                    cout << "Book is available. No need to reserve.\n";
                }
            }
            else
            {
                cout << "Book not found.\n";
            }
        }
        lib.saveData();
    } while (choice != 7);
}

// Librarian Dashboard
void Librarian::dashboard(Library &lib)
{
    int choice;
    do
    {
        cout << "\n--- LIBRARIAN DASHBOARD (" << getName() << ") ---\n";
        cout << "1. Add a New Book\n";
        cout << "2. Remove a Book\n";
        cout << "3. Update Book Information\n";
        cout << "4. Add a New User\n";
        cout << "5. Remove a User\n";
        cout << "6. View All Books\n";
        cout << "7. View All Users\n";
        cout << "8. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (choice)
        {
        case 1:
        {
            cout << "Enter new book details:\n";
            int bid, pubYear;
            string title, author, publisher, isbn;
            cout << "ID: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (lib.bookExists(bid))
            {
                cout << "Book with ID " << bid << " already exists. Cannot add duplicate book.\n";
                break;
            }
            cout << "Title: ";
            getline(cin, title);
            cout << "Author: ";
            getline(cin, author);
            cout << "Publisher: ";
            getline(cin, publisher);
            cout << "Year: ";
            cin >> pubYear;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "ISBN: ";
            getline(cin, isbn);
            lib.getBooks().push_back(Book(bid, title, author, publisher, pubYear, isbn, "Available", -1));
            cout << "New book added: " << title << "\n";
            break;
        }
        case 2:
        {
            int bid;
            cout << "Enter Book ID to remove: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            bool removed = false;
            for (auto it = lib.getBooks().begin(); it != lib.getBooks().end(); ++it)
            {
                if (it->getId() == bid)
                {
                    lib.getBooks().erase(it);
                    removed = true;
                    cout << "Book " << bid << " removed.\n";
                    break;
                }
            }
            if (!removed)
                cout << "Book not found.\n";
            break;
        }
        case 3:
        {
            int bid;
            cout << "Enter Book ID to update: ";
            cin >> bid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            string newTitle;
            cout << "Enter new title: ";
            getline(cin, newTitle);
            bool updated = false;
            for (auto &bk : lib.getBooks())
            {
                if (bk.getId() == bid)
                {
                    bk.setTitle(newTitle);
                    updated = true;
                    cout << "Book " << bid << " updated to: " << newTitle << "\n";
                    break;
                }
            }
            if (!updated)
                cout << "Book not found.\n";
            break;
        }
        case 4:
        {
            cout << "Enter new user details:\n";
            int uid;
            string uname, pwd, urole, fname;
            cout << "ID: ";
            cin >> uid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (lib.userExists(uid))
            {
                cout << "User with ID " << uid << " already exists. Cannot add duplicate user.\n";
                break;
            }
            cout << "Username: ";
            getline(cin, uname);
            cout << "Password: ";
            getline(cin, pwd);
            cout << "Role (Student/Faculty): ";
            getline(cin, urole);
            cout << "Name: ";
            getline(cin, fname);
            bool added = false;
            if (urole == "Student")
            {
                lib.getUsers().push_back(new Student(uid, uname, pwd, fname));
                added = true;
            }
            else if (urole == "Faculty")
            {
                lib.getUsers().push_back(new Faculty(uid, uname, pwd, fname));
                added = true;
            }
            else
            {
                cout << "Invalid role. User not added.\n";
            }
            if (added)
                cout << "New user added: " << uname << " (" << urole << ")\n";
            break;
        }
        case 5:
        {
            int uid;
            cout << "Enter User ID to remove: ";
            cin >> uid;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            bool removed = false;
            for (auto it = lib.getUsers().begin(); it != lib.getUsers().end(); ++it)
            {
                if ((*it)->getId() == uid)
                {
                    for (auto rec : (*it)->getAccount().getRecords())
                    {
                        Book *book = lib.findBookById(rec.bookId);
                        if (book)
                        {
                            book->setStatus("Available");
                            book->clearReservation();
                        }
                    }
                    delete *it;
                    lib.getUsers().erase(it);
                    removed = true;
                    cout << "User " << uid << " removed. All books borrowed by this user are now marked as available.\n";
                    break;
                }
            }
            if (!removed)
                cout << "User not found.\n";
            break;
        }
        case 6:
        {
            cout << "All Books:\n";
            for (auto &bk : lib.getBooks())
                bk.display();
            break;
        }
        case 7:
        {
            cout << "All Users:\n";
            for (auto usr : lib.getUsers())
            {
                cout << usr->getId() << " - " << usr->getUsername() << " (" << usr->getRole() << ") - " << usr->getName() << "\n";
            }
            break;
        }
        case 8:
        {
            cout << "Logging out...\n";
            break;
        }
        default:
            cout << "Invalid option. Try again.\n";
            break;
        }
        lib.saveData();
    } while (choice != 8);
}

// Main Function
int main()
{
    Library library;
    library.loadData();

    int choice;
    do
    {
        cout << "\n--- LIBRARY MANAGEMENT SYSTEM ---\n";
        cout << "1. Login as Student\n";
        cout << "2. Login as Faculty\n";
        cout << "3. Login as Librarian\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1)
        {
            int uid;
            string pwd;
            cout << "Enter user id: ";
            cin >> uid;
            cout << "Enter password: ";
            cin >> pwd;
            bool loggedIn = false;
            for (auto user : library.getUsers())
            {
                if (user->getRole() == "Student" && user->getId() == uid && user->getPassword() == pwd)
                {
                    cout << "Login successful. Welcome " << user->getName() << "!\n";
                    user->dashboard(library);
                    loggedIn = true;
                    break;
                }
            }
            if (!loggedIn)
                cout << "Invalid credentials.\n";
        }
        else if (choice == 2)
        {
            int uid;
            string pwd;
            cout << "Enter user id: ";
            cin >> uid;
            cout << "Enter password: ";
            cin >> pwd;
            bool loggedIn = false;
            for (auto user : library.getUsers())
            {
                if (user->getRole() == "Faculty" && user->getId() == uid && user->getPassword() == pwd)
                {
                    cout << "Login successful. Welcome " << user->getName() << "!\n";
                    user->dashboard(library);
                    loggedIn = true;
                    break;
                }
            }
            if (!loggedIn)
                cout << "Invalid credentials.\n";
        }
        else if (choice == 3)
        {
            int uid;
            string pwd;
            cout << "Enter user id: ";
            cin >> uid;
            cout << "Enter password: ";
            cin >> pwd;
            bool loggedIn = false;
            for (auto user : library.getUsers())
            {
                if (user->getRole() == "Librarian" && user->getId() == uid && user->getPassword() == pwd)
                {
                    cout << "Login successful. Welcome " << user->getName() << "!\n";
                    user->dashboard(library);
                    loggedIn = true;
                    break;
                }
            }
            if (!loggedIn)
                cout << "Invalid credentials.\n";
        }
        else if (choice == 4)
        {
            cout << "Exiting program.\n";
            break;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    } while (true);

    library.saveData();
    return 0;
}
