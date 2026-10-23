# Database Storage & Indexing Engine

A low-level **Database Management System (DBMS) Storage & Indexing Engine** implemented in **C++**. Developed as a collaborative team project for the *Implementation of Database Management Systems* course (Winter Semester 2020-2021)  at the **Department of Informatics and Telecommunications, National and Kapodistrian University of Athens (NKUA)**.

The engine manages low-level block I/O across fixed-size 512-byte disk blocks with binary record serialization. It combines **Heap File**  organization with **Primary and Secondary Static Hash Indexing** to achieve $O(1)$ average lookup complexity.


## Key Features

* **Block-Level I/O Management**: Built on top of a custom Block File (`BF`) driver operating on fixed 512-byte disk blocks.
* **Heap File Storage** (`HP`): Manages fixed-size records (96 bytes) using slot tracking and soft deletion.
* **Primary Static Hash Index** (`HT`): Implements static hashing over unique primary keys (`id`) using Heap Files for bucket overflow chaining.
* **Secondary Hash Index** (`SHT`): Enables fast lookups on non-unique string fields (`surname`) via secondary index references to the primary table.
* **Object-Oriented C++ Core**: Built with an extensible C++ class hierarchy (`AbstractFile` → `AbstractHashtable` → `Hashtable` / `SecondaryHashtable`) accessed through simple C-style functions.
* **Indexing Analytics** (`HashStatistics`): Tracks bucket usage, record distribution, and overflow blocks.


## Building & Execution

### Requirements
* **C++ Compiler**: `g++` (supporting C++11 or higher)
* **Build System**: `make` 
* **Architecture**: 64-bit Linux environment (required for `lib/BF_64.a`)

---

### Compilation

```bash
# Standard Build
make

# Testing Build
make MODE=TESTING

# Clean build artifacts
make clean
```

---

### Execution

Ensure that `records15K.txt` is located in the working directory before executing:

```bash
# Run via Makefile
make run

# Direct Execution
./ysbd
```


## Authors

* **Sergios - Anestis Kefalidis** ([@SKefalidis](https://github.com/SKefalidis))
* **Eleftheria Lagkoni** ([@elagkoni](https://github.com/elagkoni))