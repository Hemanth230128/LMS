# Library Management System

This project implements a Library Management System in C++ using Object-Oriented Programming (OOP) principles. It demonstrates key OOP concepts such as encapsulation, inheritance, abstraction, and polymorphism while managing common library operations. The system is designed to handle three types of users—Students, Faculty, and Librarians—with role-specific functionalities.

## Project Overview

The Library Management System supports the following operations:

- **Students** can:
  - Borrow up to 3 books simultaneously.
  - Return borrowed books.
  - Reserve a book if it is currently borrowed by someone else.
  - View available and reserved books.
  - Check and pay fines for overdue books (10 rupees per day for books overdue beyond the 15-day borrowing period).
  - **Important:** Students cannot borrow new books if they have any outstanding fines.

- **Faculty** can:
  - Borrow up to 5 books at a time.
  - Return books within a 30-day period.
  - Reserve books as needed.
  - View their borrowing status.
  - **Note:** Faculty members do not incur fines for overdue books. However, if any book is overdue by more than 60 days, further borrowing is not allowed.

- **Librarians** can:
  - Add new books to the system.
  - Remove books from the system.
  - Update existing book information.
  - Add new users (Students or Faculty) and remove users.
  - View all registered books and users.
  - **Constraint:** Every book and every user must have a unique ID. Duplicate entries are rejected.
  - **Constraint:** When a Librarian removes a user, all books borrowed by that user are marked as "Available."

## Project Structure

```
.
 ├── src/
 │   └── main.cpp    # Main C++ source file
 └── data/
     ├── books.txt   # Contains book records (ID, title, author, publisher, year, ISBN, status, reservedBy)
     ├── users.txt   # Contains user records (ID, username, password, role, name)
     └── fines.txt   # Contains loan records and fine details
```
## Usage

1. **Compilation**
   ```
   g++ -std=c++11 src/main.cpp -o library
   ```

2. **Running the Program**
   ```
   library.exe
   ```

3. **Testing time scale**
   ```
   For testing purposes, the code treats 10 seconds as 1 day.
   ```


## Features and Constraints

### Unique Identifiers
- **Books and Users:**  
  Each book and user is identified by a unique integer ID. When a librarian attempts to add a new book or user, the system checks for duplicate IDs and rejects any entries with an already existing ID.

### Borrowing and Reservation Rules

- **Students:**
  - **Borrowing Limit:** Up to 3 books simultaneously.
  - **Borrowing Period:** 15 days per book.
  - **Fines:** 10 rupees per day for overdue books.
  - **Reservation:** Can reserve a book if it is borrowed by someone else. Once returned, the book becomes available only to the reserving student.
  - **Additional Constraint:** Cannot borrow new books if any fine is outstanding.

- **Faculty:**
  - **Borrowing Limit:** Up to 5 books simultaneously.
  - **Borrowing Period:** 30 days per book.
  - **Overdue Policy:** No fines are charged for overdue returns; however, if any borrowed book is overdue by more than 60 days, new borrowing is blocked.
  - **Reservation:** Similar reservation rules as for students.

- **Librarians:**
  - **Management Operations:** Full control over the system—can add, update, or remove books and users.
  - **Note:** Librarians do not borrow books.
  - **Unique ID Check:** Ensures that duplicate IDs for books or users are not allowed.

### Data Persistence
- **File I/O:**  
  The system uses three text files (located in the `data` folder):
  - `books.txt` for book records.
  - `users.txt` for user records.
  - `fines.txt` for borrow records and fine information.
  
- **Automatic Saving:**  
  Data is saved after significant operations (e.g., borrowing or returning books, adding a user) and upon program exit.