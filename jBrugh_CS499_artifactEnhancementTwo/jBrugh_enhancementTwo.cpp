/*
 * Jonathan Brugh
 * CS499 Capstone
 * Mr. K
 * Enhancement Two: Algorithms and Data Structure
 * July 26, 2026
 */

/*
 * This application recieves in a CSV of SNHU's courses and prerequisites,
 * sorts and stores the information in a BST held in memory, and allows a
 * user to search for a course. It prints the infomation to a console.
 */

 /*
  * This revision implements a delete operation, fixes minor bugs,
  * and adds better flow controls, such as checking for duplicate 
  * CSV entries, Course IDs are now stored in all caps, the virtual 
  * ~BinarySearchTree() is removed, and resetting the terminal color
  * after the program exits. 
  */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <cctype>
#include <algorithm>

// Maximum length allowed for any single field read from user input or a
// CSV file. Prevents unbounded memory growth from malformed/malicious input.
constexpr size_t MAX_FIELD_LENGTH = 200;

// Strips control characters from a field and truncates it to MAX_FIELD_LENGTH.
// Applied to every piece of course data before it is stored or displayed.
std::string sanitize(const std::string& input) {
    std::string cleaned;
    cleaned.reserve(input.size());

    for (unsigned char c : input) {
        // Keeps space and all other printable ASCII characters; drops
        // control characters (0x00-0x1F) and DEL (0x7F).
        if (c == ' ' || (c >= 0x20 && c != 0x7F)) {
            cleaned += static_cast<char>(c);
        }
    }

    if (cleaned.size() > MAX_FIELD_LENGTH) {
        cleaned.resize(MAX_FIELD_LENGTH);
    }

    return cleaned;
}

std::string ToUpper(std::string word) {
    std::transform(word.begin(), word.end(), word.begin(), toupper);
    return word;
}

// Struct for holding course information; methods in BST class use courseId
struct Course {
    std::string courseId = "";
    std::string courseTitle = "";
    std::vector<std::string> preReqs;
};

// Struct building the nodes.
// Left/right are smart pointers so node memory is freed automatically.
struct Node {
    Course course;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    Node() = default;

    explicit Node(Course aCourse) : course(std::move(aCourse)) {}
};

// The BST class holds the CRUD operations and builds the BST in memory
class BinarySearchTree {

    private:
        std::unique_ptr<Node> root;

        void addNode(Node* node, Course course);
        void inOrder(Node* node);
        std::unique_ptr<Node> removeNode(std::unique_ptr<Node> node, std::string courseId);
        Node* getSuccessor(Node* node);
        void updateCourse(Course course);

    public:
        BinarySearchTree() = default;
        void InOrder();
        void Insert(Course course);
        Course Search(std::string courseId);
        void DeleteCourse(std::string courseId);
        void UpdateCourse(Course course);
    };

// Traverse the tree in alphanumeric order
void BinarySearchTree::InOrder() {
	inOrder(root.get());
    std::cout << std::endl;
}

// Insert a course into the tree
void BinarySearchTree::Insert(Course course) {

    // if tree is empty, insert node as the root node
	if (root == nullptr) {
		root = std::make_unique<Node>(std::move(course));
	}

    // else call addNode to find proper placement
    else {
		addNode(root.get(), std::move(course));
	}
}

void BinarySearchTree::UpdateCourse(Course course) {
    updateCourse(course);
}

// Search for a course by the courseId
Course BinarySearchTree::Search(std::string courseId) {
    // Search for node if it exists
	Node* curNode = root.get();

    while (curNode != nullptr) {

        if (curNode->course.courseId == courseId) {
            return curNode->course;
        }

        else if (curNode->course.courseId > courseId) {
            curNode = curNode->left.get();
        }

        else {
            curNode = curNode->right.get();
        }
    }

    // Return empty course if node not found
	return Course();
}

void BinarySearchTree::DeleteCourse(std::string courseId) {
    root = removeNode(std::move(root), courseId);
    std::cout << "Course " << courseId << " deleted." << std::endl;
}

/**
 * Adds a course to some node (recursive)
 * Uses courseId to find placement
 *
 * @param node Current node in tree
 * @param course course to be added
 */
void BinarySearchTree::addNode(Node* node, Course course) {

    // if duplicate course is found, overwrites old node data
    if (course.courseId == node->course.courseId) {
        node->course = std::move(course);  // update instead of duplicating
        return;
    }

    // if current node courseId is larger than new courseId
	if (course.courseId < node->course.courseId) {
	
        // if no left node
		if (node->left == nullptr) {
		
            // newNode course becomes left
			node->left = std::make_unique<Node>(std::move(course));
	    }
		
        // recurse down the left node
		else {
			// call addNode with left node and course 
            // to continue finding placement
			addNode(node->left.get(), std::move(course));
		}
    }
	
    // if current node courseId is smaller than new courseId
	else {
	
		// if no right node
		if (node->right == nullptr) {

            // newNode course node becomes right
			node->right = std::make_unique<Node>(std::move(course));
	    }
		
        // recurse down the right node
        else {
			// call addNode with right node and course
            // to continue finding placement
			addNode(node->right.get(), std::move(course));
		}
    }
}

// prints entire tree in alphanumeric order
void BinarySearchTree::inOrder(Node* node) {
    
    // if BST is not empty
    if (node != nullptr) {
	
        // traverse left subtree
        inOrder(node->left.get());

        // print node courses
        std::cout << node->course.courseId << ": " << node->course.courseTitle
                     << "| Prerequisites: ";

        // Prints prequisites
        for (size_t i = 0; i < node->course.preReqs.size(); ++i) {

            std::cout << node->course.preReqs[i];

            if (i + 1 < node->course.preReqs.size()) {
                std::cout << ", ";
            } 
        }
        std::cout << std::endl;

        // traverse right subtree
        inOrder(node->right.get());
    }
}

std::unique_ptr<Node> BinarySearchTree::removeNode(std::unique_ptr<Node> node, std::string courseId) {

    // No node found
    if (node == nullptr) {
        return nullptr;
    }

    // Traverse left subtree
    if (node->course.courseId > courseId) {
        node->left = removeNode(std::move(node->left), courseId);
    }

    // Traverse right subtree
    else if (node->course.courseId < courseId) {
        node->right = removeNode(std::move(node->right), courseId);
    }

    // Node to delete found
    else {

        if (node->left == nullptr) {
            return std::move(node->right);
        }

        if (node->right == nullptr) {
            return std::move(node->left);
        }

        Node* successor = getSuccessor(node->right.get());
        node->course = successor->course;
        node->right = removeNode(std::move(node->right), successor->course.courseId);
    }
    return node;
}

Node* BinarySearchTree::getSuccessor(Node* successorNode) {

    // Find successor node if it exists
    while (successorNode != nullptr && successorNode->left.get() != nullptr) {
        successorNode = successorNode->left.get();
    }
    return successorNode;
}

void BinarySearchTree::updateCourse(Course course) {

    // Search for node if it exists
	Node* curNode = root.get();

    while (curNode != nullptr) {
        if (curNode->course.courseId == course.courseId) {
            curNode->course.courseTitle = course.courseTitle;
            curNode->course.preReqs = course.preReqs;
            return;
        }

        else if (curNode->course.courseId > course.courseId) {
            curNode = curNode->left.get();
        }

        else {
            curNode = curNode->right.get();
        }
    }
}

// Checks that a user supplied path is safe to open. Rejects absolute
// paths and any path containing "..".
bool isPathSafe(const std::string& userInput) {

    std::filesystem::path inputPath(userInput);

    if (inputPath.is_absolute()) {
        return false;
    }

    for (const auto& part : inputPath) {
        if (part == "..") {
            return false;
        }
    }

    return true;
}

// Checks if file is a valid, safe CSV file path
bool isValidCsvPath(const std::string& file) {

    std::string checkForCSV = ".csv";

    if (file.find(checkForCSV) == std::string::npos) {
        return false;
    }

    return isPathSafe(file);
}

/** 
 * loads csv given and inserts into the BST:
 * @param csvPath to file of courses
 * @param BST tree to add course files to
 */
void loadCSV(std::string csvPath, BinarySearchTree* bst) {

    std::cout << "Loading CSV file " << csvPath << std::endl << std::endl;

    std::ifstream file;

    // Attempt to open the file.
    file.open(csvPath);

    // Keeps application running until valid, openable file is entered
    // or 3 attempts are made
    int attempts = 1;
    while (!file.is_open() && attempts < 3) {

        attempts++;

        // Clears cin to recieve new input
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Could not open file. Please enter a valid CSV file name."
                    << std::endl;

        std::string relativeFilePath;
        std::cin >> relativeFilePath;

        // Security checks for new user input
        if (isValidCsvPath(relativeFilePath)) {
            csvPath = relativeFilePath;
            file.open(csvPath);
        } 
        
        else {
            std::cout << "Invalid or unsafe file path." << std::endl;
        }

        std::cout << "Loading CSV file " << csvPath << std::endl
                    << std::endl;
    }

    // After 3 attempts are made, returns to main menu
    if (!file.is_open()) {
        std::cout << "Unable to open a valid CSV file after multiple attempts."
                    << std::endl;
        return;
    }

    // maintenance variables for file
    std::string line;
    std::string word;
    int wordCount;

    // while file has a line to get
    while (getline(file, line)) {

        // maintenance variables for each line
        Course course;
        std::stringstream str(line);

        // used to find where prerequisites begin
        wordCount = 0;

        // first two inputs are courseId and courseTitle
        while (wordCount < 2) {

            getline(str, word, ',');
            word = sanitize(word);

            // Sets the first word as the courseId
            if (wordCount == 0) {
                course.courseId = ToUpper(sanitize(word));
            }

            // Sets second word as courseTitle
            else {
                course.courseTitle = word;
            }
            wordCount++;
        }

        // After the second input, remaining inputs are prerequisites
        // Pushes remaining inputs to preReqs vector
        while (getline(str, word, ',')) {

            word = sanitize(word);

            if (word != "") {
                course.preReqs.push_back(word);
            }

            else {
                course.preReqs.push_back("None");
            }
        }

        // add course to bst
        bst->Insert(course);
    }

    // Closes the open file
    file.close();
}

// prints individual course info
void displayCourseInfo(const Course& aCourse) {

    std::cout << aCourse.courseId << ": " << aCourse.courseTitle 
                << " | Prerequisites: ";
    
    // Loop for printing all prerequisites
    for (size_t i = 0; i < aCourse.preReqs.size(); ++i) {

        std::cout << aCourse.preReqs[i];

        if (i + 1 < aCourse.preReqs.size()) {
            std::cout << ", ";
        } 
    }
    
    std::cout << std::endl;
}

// Creates a line of 32 '*', used for top and bottom menu borders
void topBottomMenuBorder() {
    std::cout << std::setfill('*') << std::setw(32) << "" << std::endl;
}

// Prints the main user menu
void userMenu() {
    topBottomMenuBorder();
    std::cout << "Menu:" << std::endl;
    std::cout << "  1. Load Courses" << std::endl;
    std::cout << "  2. Display All Courses" << std::endl;
    std::cout << "  3. Display Course Information" << std::endl;
    std::cout << "  4. Add new Course Or Update Existing Course Information"
                 << std::endl;
    std::cout << "  5. Delete Course" << std::endl;
    std::cout << "  9. Exit" << std::endl;
    topBottomMenuBorder();
    std::cout << "Enter choice: ";
}

int main() {
    // Displays all output in green for better contrast to improve visibility
    std::string greenTextOutput = "\033[32m";
    std::cout << greenTextOutput;

    /* Defines a binary search tree to hold all courses.
     * Owned by a unique_ptr so it is guaranteed to be cleaned up.
     * Removes the need for explicit contructor and destructor.
     */
    std::unique_ptr<BinarySearchTree> bst = std::make_unique<BinarySearchTree>();
    
    int choice = 0;
    while (choice != 9) {

        userMenu();
        
        std::cin >> choice;
        std::cout << std::endl;
        
        // Handles alphabetic character entry 
        if (std::cin.fail()) {
            // Clear the error flags on the input stream.
            std::cin.clear();

            // leave the rest of the line
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                       '\n');

            // Ask the user to enter a valid int number only
            std::cout << "Invalid input, please enter 1, 2, 3, 4, 5, or 9." 
                        << std::endl;
        }

        switch (choice) {

            // Case 1 loads CSV file parsed by user.
            case 1:
            {
                std::string userFileInput;

                std::cout << "Please specify CSV file name." << std::endl;

                std::cin >> userFileInput;

                // Checks if input is a CSV file and safe to open
                if (isValidCsvPath(userFileInput)) {
                    loadCSV(userFileInput, bst.get());
                }

                else {
                    std::cout << "File is not a valid, safe CSV path."
                            << std::endl << std::endl;
                }
            }
                break;

            // Case 2 prints all courses loaded from CSV file.
            case 2:
            {
                std::cout << "Course List and Prerequisites:" << std::endl;

                bst->InOrder();
            }
                break;

            // Case 3 searches for and prints a user specified course 
            // and its prereqs, if any.
            case 3:
            {
                Course course;
                std::string courseIdToFind;

                std::cout << "Please enter course ID:" << std::endl;

                std::cin >> courseIdToFind;
                courseIdToFind = sanitize(courseIdToFind);
                courseIdToFind = ToUpper(courseIdToFind);

                std::cout << std::endl;

                course = bst->Search(courseIdToFind);

                if (!course.courseId.empty()) {
                    displayCourseInfo(course);
                }
                
                else {
                    std::cout << "Course Id " << courseIdToFind << " not found." 
                            << std::endl;
                }
            }
                break;

            // Case 4 inesrts a new course into the BST.
            // Also used to update existing courses in BST.
            case 4:
            {
                Course course;
                Course courseCheck;

                std::string prereqInput;
                std::string word;

                // Building the new Course
                // Getting and setting the course ID
                std::cout << "Enter Course ID:" << std::endl;
                std::cin >> course.courseId;
                course.courseId = sanitize(course.courseId);
                course.courseId = ToUpper(course.courseId);

                // Getting and setting course title
                std::cout << "Enter Course Title:" << std::endl;
                std::cin.ignore();
                std::getline(std::cin, course.courseTitle);
                course.courseTitle = sanitize(course.courseTitle);

                // Getting and setting prerequisites vector
                std::cout << "Enter the course prerequisites separated " 
                        << "by commas or enter \"none\":" << std::endl;

                std::getline(std::cin, prereqInput);
                std::stringstream str(prereqInput);

                while (std::getline(str, word, ',')) {

                    word = sanitize(word);

                    if (word != "") {
                        course.preReqs.push_back(word);
                    }

                    else {
                        course.preReqs.push_back("None");
                    }
                }

                // Search for existing Course and update the information
                // if found.
                courseCheck = bst->Search(course.courseId);
                if (courseCheck.courseId != "") {
                    bst->UpdateCourse(course);
                }

                // If existing course is not found, inserts a new Course.
                else {
                    bst->Insert(course);
                }
            }
                break;

            case 5:
            {
                std::string deleteCourseID;
                std::cout << "Enter courseID to delete:" << std::endl;
                std::cin >> deleteCourseID;
                deleteCourseID = sanitize(deleteCourseID);
                deleteCourseID = ToUpper(deleteCourseID);

                bst->DeleteCourse(deleteCourseID);
            }
                break;

            case 9:
            {
                std::cout << "Good bye." << std::endl;
            }
                break;

            // Default case handles inappropriate input
            default:
                std::cout << "Please enter a valid option." << std::endl 
                        << std::endl;
        }
    }
    // Reverts to default terminal text style
    std::string defaultText = "\033[0m";
    std::cout << defaultText;
    return 0;
}
