# CS499 Capstone — Enhancement Two: Course Advising Program

**Author:** Jonathan Brugh
**Course:** CS499 Capstone
**File:** `jBrugh_enhancementTwo.cpp`

## Overview

This program reads a CSV file of course data (course ID, title, and prerequisites), stores each course as a node in a binary search tree (BST) keyed on `courseId`, and lets a user interactively search for, add, update, and delete courses from the console.

It is Enhancement Two of the CS499 capstone, built directly on top of Enhancement One (`jBrugh_cs499EnhancementOneArchive.cpp`). Where Enhancement One focused on hardening input handling and interface cleanup *around* the BST, Enhancement Two targets the **Algorithms and Data Structures** category by rewriting the BST itself: it implements the delete operation that both prior versions only declared, fixes a duplicate-key bug that could silently corrupt the tree, normalizes keys so course IDs are case-insensitive, replaces manual pointer management with `std::unique_ptr`-based ownership, and moves course data through the tree instead of copying it.

## Features

- **Load Courses** — reads a CSV file and inserts each row into the BST.
- **Display All Courses** — performs an in-order traversal, printing every course and its prerequisites in alphanumeric order by course ID.
- **Display Course Information** — searches the BST for a single course by ID and prints its title and prerequisites.
- **Add or Update a Course** — prompts for a course ID, title, and prerequisites; if the ID already exists in the tree its data is updated in place, otherwise a new course is inserted.
- **Delete Course** — removes a course from the tree by ID using a full recursive BST deletion (handles the no-child, one-child, and two-child cases via an in-order successor).
- **Input validation and recovery** — invalid file paths, unsafe paths, non-CSV files, and invalid menu selections are caught and the user is prompted to correct them instead of the program failing silently.
- **Input sanitization** — every field read from a CSV or typed by the user is stripped of control characters and truncated to a maximum length before it's stored.
- **Case-insensitive course IDs** — every course ID is upper-cased before it's inserted, searched, updated, or deleted, so `CS300` and `cs300` are always treated as the same course.

## Expected CSV Format

Each line represents one course:

```
courseId,courseTitle,prereq1,prereq2,...
```

- `courseId` and `courseTitle` are required.
- Any number of prerequisite course IDs may follow, separated by commas.
- A course with no prerequisites can either omit the trailing fields or leave them blank — empty fields are parsed and stored as `"None"` rather than being silently dropped.
- Course IDs are automatically upper-cased on load, so mixed-case input in the source file is fine.

Example (`CS300_ABCU_Advising_Program_Input.csv`):

```
CSCI100,Introduction to Computer Science,,
CSCI101,Introduction to Programming in C++,CSCI100
CSCI300,Introduction to Algorithms,CSCI200,MATH201
CSCI350,Operating Systems,CSCI300
```

## Building

The program is a single C++ source file with no external dependencies beyond the standard library (including `<filesystem>` and `<memory>`, so a C++17-compatible compiler is required).

```bash
g++ -std=c++17 -o advising_program jBrugh_enhancementTwo.cpp
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
  4. Add new Course Or Update Existing Course Information
  5. Delete Course
  9. Exit
********************************
Enter choice:
```

1. **Load Courses** — enter the path to a `.csv` file. The path must be relative and cannot contain `..` or an absolute prefix. If the file doesn't exist, isn't openable, or fails the safety check, you'll be given up to three attempts to enter a valid path before returning to the menu.
2. **Display All Courses** — prints every loaded course and its prerequisites, in order by course ID.
3. **Display Course Information** — enter a course ID to look up its title and prerequisites.
4. **Add or Update a Course** — enter a course ID, title, and comma-separated prerequisites (or `"none"`). If the ID already exists, its data is updated; otherwise a new course is added.
5. **Delete Course** — enter a course ID to remove that course from the tree.
9. **Exit** — closes the program.

## Design Notes

Enhancement Two keeps the overall program structure from Enhancement One but rewrites the BST's internals to be correct, safe, and efficient under the operations a real advising tool would actually need — repeated inserts, deletions that require restructuring, and searches whose correctness depends on every node obeying the same ordering rule.

## Known Limitations / Future Work

- The tree remains an **unbalanced BST**, so search, insert, and delete run in O(h) time, where h is the tree's height. In the worst case — courses loaded in already-sorted order — that height can degrade toward O(n). This enhancement does not add self-balancing (e.g., AVL or red-black rebalancing).
- A planned future enhancement will replace the in-memory BST with a database management system, addressing the balancing/performance limitation from a different angle.

## Related Files

- `jBrugh_C499_originalArtifactCodeReview.cpp` — the original artifact, annotated with code-review comments identifying the initial design issues.
- `jBrugh_cs499EnhancementOneArchive.cpp` — Enhancement One, which this revision builds on directly.
- `jBrugh_Enhancement_Two_Narrative.docx` — the written narrative explaining the rationale behind each change in detail.
- `CS300_ABCU_Advising_Program_Input.csv` — sample course data compatible with this program's expected CSV format.
