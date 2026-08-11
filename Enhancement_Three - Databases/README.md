# CS499 Capstone — Enhancement Three: Course Advising Program

**Author:** Jonathan Brugh
**Course:** CS499 Capstone
**File:** `jBrugh_enhancementThree.cpp`

## Overview

This program reads a CSV file of course data (course ID, title, and prerequisites), stores it in a MongoDB collection, and lets a user interactively seed the database, search for, add, update, and delete courses from the console.

It is Enhancement Three of the CS499 capstone, and represents a fundamentally different kind of change from the two prior revisions. Enhancement One hardened the console program around its data structure, and Enhancement Two corrected and completed the in-memory binary search tree (BST) itself, but in both versions the course catalog lived and died with the process — closing the program erased it. Enhancement Three removes the BST entirely and replaces it with a MongoDB-backed `CourseRepository` class built on the official `mongocxx` driver. The course catalog is now a persistent collection of BSON documents that exists independently of any single run of the program, targeting the capstone's **Databases** category: schema design in a document model, index-enforced data integrity, atomic upsert semantics, bulk-write behavior, and defensive handling of a data store that can be unreachable, slow, or already holding conflicting data.

## Features

- **Seed Database With Courses From CSV** — reads a CSV file and bulk-inserts every row into the `courses` collection in one `insert_many()` call, reporting how many rows were actually saved versus skipped (e.g., due to a duplicate course ID).
- **Display All Courses** — queries the collection with `find({})`, sorted by `courseId`, and prints every course and its prerequisites.
- **Display Course Information** — looks up a single course by ID with `find_one()` and prints its title and prerequisites.
- **Add or Update a Course** — prompts for a course ID, title, and prerequisites, then performs a single atomic `replace_one(...upsert(true))` call: if the ID already exists its document is replaced, otherwise a new document is inserted. The program reports whether the course was added, updated, or already up to date.
- **Delete Course** — removes a course document from the collection by ID.
- **Startup connectivity check** — pings the MongoDB server before the main menu loads and reports whether the connection succeeded.
- **Unique index enforcement** — a unique index on `courseId` is created on the collection at startup, so the database itself rejects any duplicate course ID, regardless of which code path attempted the write.
- **Input validation and sanitization** — invalid/unsafe CSV paths and invalid menu selections are caught and the user is prompted to correct them; every field is stripped of control characters and length-limited before it's stored.
- **Case-insensitive course IDs** — every course ID is upper-cased before it's inserted, searched, updated, or deleted.

## Prerequisites

Unlike Enhancements One and Two, this version is **not self-contained** — it depends on external services and libraries:

- A running **MongoDB** instance reachable at `mongodb://localhost:27017` (the URI is hard-coded in `main()`).
- The **mongocxx** and **bsoncxx** C++ driver libraries installed and available to the compiler/linker.

## Expected CSV Format

Each line represents one course:

```
courseId,courseTitle,prereq1,prereq2,...
```

- `courseId` and `courseTitle` are required.
- Any number of prerequisite course IDs may follow, separated by commas.
- A course with no prerequisites can either omit the trailing fields or leave them blank — empty fields are stored as `"None"` rather than being dropped.
- Course IDs are automatically upper-cased on load.

Example (`CS300_ABCU_Advising_Program_Input.csv`):

```
CSCI100,Introduction to Computer Science,,
CSCI101,Introduction to Programming in C++,CSCI100
CSCI300,Introduction to Algorithms,CSCI200,MATH201
CSCI350,Operating Systems,CSCI300
```

## Building

Requires a C++17-compatible compiler and the mongocxx/bsoncxx driver headers and libraries installed and discoverable by your build system. A typical build (paths will vary by platform/package manager):

```bash
g++ -std=c++17 jBrugh_enhancementThree.cpp -o advising_program \
    $(pkg-config --cflags --libs libmongocxx libbsoncxx)
```

If `pkg-config` isn't set up for the driver on your system, consult your MongoDB C++ driver installation's documentation for the correct include/library paths.

## Running

Make sure MongoDB is running locally and reachable at `mongodb://localhost:27017`, then:

```bash
./advising_program
```

On startup the program pings the database and reports the connection result, then ensures the unique index on `courseId` exists before showing the menu:

```
********************************
Menu:
  1. Seed Database With Courses From CSV
  2. Display All Courses
  3. Display Course Information
  4. Add new Course Or Update Existing Course Information
  5. Delete Course
  9. Exit
********************************
Enter choice:
```

1. **Seed Database With Courses From CSV** — enter the path to a `.csv` file. The path must be relative and cannot contain `..` or an absolute prefix. Valid rows are inserted in bulk; rows that collide with an existing course ID are skipped rather than aborting the whole load, and the program reports how many of the file's rows were actually saved.
2. **Display All Courses** — prints every course in the database, sorted by course ID.
3. **Display Course Information** — enter a course ID to look up its title and prerequisites.
4. **Add or Update a Course** — enter a course ID, title, and comma-separated prerequisites (or `"none"`). The database decides atomically whether to insert or update.
5. **Delete Course** — enter a course ID to remove that course's document from the collection.
9. **Exit** — closes the program.

## Design Notes

Enhancement Three re-derives the program's data-access logic around what a database is actually good at, rather than simply persisting the same in-memory structure to disk.

## Known Limitations / Future Work

- This version requires a running MongoDB instance and the mongocxx/bsoncxx driver libraries, which the prior, self-contained BST versions did not need. That dependency is the cost of persistence and is a normal trade-off for a database-backed application, but Enhancement Three cannot run stand-alone the way Enhancements One and Two could.
- The MongoDB connection URI (`mongodb://localhost:27017`) is hard-coded in `main()`; pointing the program at a different host, port, or authenticated instance requires editing the source.

## Related Files

- `jBrugh_C499_originalArtifactCodeReview.cpp` — the original artifact, annotated with code-review comments identifying the initial design issues.
- `jBrugh_cs499EnhancementOneArchive.cpp` — Enhancement One (input hardening and interface cleanup around the BST).
- `jBrugh_enhancementTwo.cpp` — Enhancement Two (a corrected, complete, smart-pointer-based BST), which this revision replaces with a database-backed repository.
- `Enhancement_Three_Narrative.docx` — the written narrative explaining the rationale behind each change in detail.
- `CS300_ABCU_Advising_Program_Input.csv` — sample course data compatible with this program's expected CSV format.
