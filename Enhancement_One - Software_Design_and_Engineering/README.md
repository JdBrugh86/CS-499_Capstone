# CS499 Capstone — Enhancement One: Course Advising Program

**Author:** Jonathan Brugh
**Course:** CS499 Capstone
**File:** `jBrugh_cs499EnhancementOne.cpp`

## Overview

This program reads a CSV file of course data (course ID, title, and prerequisites), stores each course as a node in a binary search tree (BST) keyed on `courseId`, and lets a user interactively search for and display information about a specific course from the console.

It is Enhancement One of the CS499 capstone, built on top of an original artifact from a prior data structures and algorithms course. The enhancement targets the **Software Design and Engineering** category by hardening the original program against bad input, fixing a data-integrity bug in CSV parsing, trimming the class's public interface down to only what is used, and adding documentation and console UI that a program intended for real users would need.

## Features

- **Load Courses** — reads a CSV file and inserts each row into the BST.
- **Display All Courses** — performs an in-order traversal, printing every course and its prerequisites in alphanumeric order by course ID.
- **Display Course Information** — searches the BST for a single course by ID and prints its title and prerequisites.
- **Input validation and recovery** — invalid file paths, non-CSV files, and invalid menu selections are caught and the user is prompted to correct them instead of the program failing silently.

## Expected CSV Format

Each line represents one course:

```
courseId,courseTitle,prereq1,prereq2,...
```

- `courseId` and `courseTitle` are required.
- Any number of prerequisite course IDs may follow, separated by commas.
- A course with no prerequisites can either omit the trailing fields or leave them blank — empty fields are parsed and stored as `"None"` rather than being silently dropped.

Example (`CS300_ABCU_Advising_Program_Input.csv`):

```
CSCI100,Introduction to Computer Science,,
CSCI101,Introduction to Programming in C++,CSCI100
CSCI300,Introduction to Algorithms,CSCI200,MATH201
CSCI350,Operating Systems,CSCI300
```

## Building

The program is a single C++ source file with no external dependencies beyond the standard library (including `<filesystem>`, so a C++17-compatible compiler is required).

```bash
g++ -std=c++17 -o advising_program jBrugh_cs499EnhancementOne.cpp
```

## Running

```bash
./advising_program
```

You'll be shown a menu:

```
********************************
Menu:
  1. Load Courses
  2. Display All Courses
  3. Display Course Information
  9. Exit
********************************
Enter choice:
```

1. **Load Courses** — enter the path to a `.csv` file. If the file doesn't exist or isn't a `.csv`, you'll be given up to three attempts to enter a valid path before returning to the menu.
2. **Display All Courses** — prints every loaded course and its prerequisites, in order by course ID.
3. **Display Course Information** — enter a course ID to look up its title and prerequisites.
9. **Exit** — closes the program.

## Design Notes

This enhancement keeps the same core algorithm as the original artifact — a BST keyed on `courseId` with standard insert, search, and in-order traversal — but changes the engineering discipline around it.

## Known Limitations / Future Work

- The BST does not currently support deleting or updating a course after it's been loaded (`removeNode()` is declared but not yet implemented). This is intentionally deferred and documented as future work rather than silently omitted.
- The tree is not self-balancing, so search performance depends on the order courses are loaded from the CSV.

## Related Files

- `jBrugh_C499_originalArtifactCodeReview.cpp` — the original artifact, annotated with code-review comments identifying the issues this enhancement addresses.
- `jBrugh_Enhancement_One_Narrative.docx` — the written narrative explaining the rationale behind each change in detail.
- `CS300_ABCU_Advising_Program_Input.csv` — sample course data compatible with this program's expected CSV format.
