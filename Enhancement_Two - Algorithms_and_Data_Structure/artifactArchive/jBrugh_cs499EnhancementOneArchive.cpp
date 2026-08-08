/*
 * Jonathan Brugh
 * CS499 Capstone
 * Mr. K
 * Enhancement One: Software Design and Engineering
 * July 19, 2026
 */

/*
 * This application recieves in a CSV of SNHU's courses and prerequisites,
 * sorts and stores the information in a BST held in memory, and allows a
 * user to search for a course. It prints the infomation to a console.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <filesystem>
#include <algorithm>
#include <iomanip>

// Struct for holding course information; methods in BST class use courseId
struct Course {
    std::string courseId;
    std::string courseTitle;
    std::vector<std::string> preReqs;
};

// Struct building the nodes
struct Node {
    Course course;
    Node* left;
    Node* right;

    Node() {
        left = nullptr;
        right = nullptr;
    }

    Node (Course aCourse) : Node() {
        course = aCourse;
    }
};

// The BST class holds the CRUD operations and builds the BST in memory
class BinarySearchTree {            // fixMe: add remove/delete method, add update method

    private:
        Node* root;
    
        void addNode(Node* node, Course course);
        void deleteBST(Node* node);
        void inOrder(Node* node);
        Node* removeNode(Node* node, std::string courseId); // Implement remove/delete method
    
    public:
        BinarySearchTree();
        virtual ~BinarySearchTree();
        void InOrder();
        void Insert(Course course);
        Course Search(std::string courseId);
    };

// Default constructor
BinarySearchTree::BinarySearchTree() {
    // initialize root of BST
	root = nullptr;
}

// Destructor
BinarySearchTree::~BinarySearchTree() {
    // recurse from root deleting every node
	deleteBST(root);
}

// Destroys the BST in memory after program end has been called.
// Recursively deletes nodes from bottom of BST
void BinarySearchTree::deleteBST(Node* node) {
    // If BST is not empty
    if (node != nullptr) {
	
        // Traverse left subtree
        deleteBST(node->left);

        // Traverse right subtree
        deleteBST(node->right);

        delete node;
		
		// Addresses dangling pointer
		node = nullptr;
    }
}

// Traverse the tree in alphanumeric order
void BinarySearchTree::InOrder() {
	inOrder(root);
    std::cout << std::endl;
}

// Insert a course into the tree
void BinarySearchTree::Insert(Course course) {
    // If tree is empty, insert node as the root node
	if (root == nullptr) {
		Node* newNode = new Node(course);
		root = newNode;
	}
	
    // Else call addNode to find proper placement
    else {
		addNode(root, course);
	}
}

// Search for a course by the courseId
Course BinarySearchTree::Search(std::string courseId) {
    // start at root
	Node* curNode = root;
	Course course;

    // keep looping downwards until bottom is reached or matching 
    // courseId is found
	while (curNode != nullptr) {
	
		// if current node courseId matches, return current course
		if (curNode->course.courseId == courseId) {
			course = curNode->course;
			return course;
		}
		
		// if courseId is smaller than current node courseId, traverse left
		else if (curNode->course.courseId > courseId) {
			curNode = curNode->left;
		}
        
        // else larger so traverse right
		else {
			curNode = curNode->right;
		}
    }
	return course;
}

/**
 * Adds a course to some node (recursive)
 * Uses courseId to find placement
 *
 * @param node Current node in tree
 * @param course course to be added
 */
void BinarySearchTree::addNode(Node* node, Course course) {
    // if current node courseId is larger than new courseId
	if (course.courseId < node->course.courseId) {
	
        // if no left node
		if (node->left == nullptr) {
		
            // newNode course becomes left
			Node* newNode = new Node(course);
			node->left = newNode;
	    }
		
        // recurse down the left node
		else {
			node = node->left;
			
			// call addNode with left node and course 
            // to continue finding placement
			addNode(node, course);
		}
    }
	
    // if current node courseId is smaller than new courseId
	else {
	
		// if no right node
		if (node->right == nullptr) {

            // newNode course node becomes right
			Node* newNode = new Node(course);
			node->right = newNode;
	    }
		
        // recurse down the right node
        else {
			node = node->right;

			// call addNode with right node and course
            // to continue finding placement
			addNode(node, course);
		}
    }
}

// prints entire tree in alphanumeric order
void BinarySearchTree::inOrder(Node* node) {
    
    // If BST is not empty
    if (node != nullptr) {
	
        // traverse left subtree
        inOrder(node->left);

        // print node courses
        std::cout << node->course.courseId << ": " << node->course.courseTitle
                     << "| Prerequisites: ";

        // Checks for prerequisites to print
        for (std::string i : node->course.preReqs) {

            // Checks if element is the last in the vector
            if (i == node->course.preReqs.back()) {
                std::cout << i;
            }
            
            // Prints a comma if not last element in vector
            else {
                std::cout << i << ", ";
            }
        }
        std::cout << std::endl;

        // traverse right subtree
        inOrder(node->right);
    }
}

/** 
 * loads csv given and inserts into the BST:
 * @param csvPath to file of courses
 * @param BST tree to add course files to
 */
void loadCSV(std::string csvPath, BinarySearchTree* bst) {

    std::cout << "Loading CSV file " << csvPath << std::endl << std::endl;

    std::ifstream file;

    // try-catch block to catch file opening errors
    try{
        // Checks if file exists
        // Does not open file
        if (!std::filesystem::exists(csvPath)) {

            // Keeps application running until valid file name is entered
            // or 3 attempts are made
            int attempts = 1;
            while ((!std::filesystem::exists(csvPath) && attempts < 3)) {
                std::string relativeFilePath;
                std::string formatted_relativeFilePath;
                std::string checkForCSV;

                // Increments attempt count
                attempts++;

                // Clears cin to recieve new input
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                    '\n');

                // Error message output
                std::cout << "File does not exist."
                            <<" Please enter a valid file name" << std::endl;

                // Receive new file name from user
                std::cin >> relativeFilePath;
                csvPath = relativeFilePath;

                // Checks if user input is a CSV file
                checkForCSV = ".csv";
                int result = csvPath.find(checkForCSV);

                // Parse the input if CSV file
                if (result != std::string::npos) {
                    csvPath = relativeFilePath;
                }

                // Parse empty string if not CSV file 
                // to allow additional attempts
                else {
                    csvPath = "";
                }

                std::cout << "Loading CSV file " << csvPath << std::endl 
                            << std::endl;
            }

            // After 3 attempts are made, returns to main menu
            if (attempts >= 3) {
                std::cout << "Too many attempts to load a valid CSV file." 
                            << std::endl;
                return;
            }
        }

        // If file exists, opens file and processes information
        file.open(csvPath);

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

                // Sets the first word as the courseId
                if (wordCount == 0) {
                    course.courseId = word;
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
        
    } catch (std::ifstream::failure &e) {
        std::cerr << e.what() << std::endl;
    }
    
    // Closes the open file
    file.close();
}

// prints individual course info
void displayCourseInfo(Course aCourse) {
    Course course = aCourse;
    std::cout << course.courseId << ": " << course.courseTitle 
                << " | Prerequisites: ";
    
    // Loop for printing all prerequisites
    for (std::string i : course.preReqs) {

        // if last element in preReqs, print without a comma
        if (i == course.preReqs.back()) {
            std::cout << i;
        }
        
        // if not last element in preReqs, print with a comma
        else {
            std::cout << i << ", ";
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
    std::cout << "  9. Exit" << std::endl;
    topBottomMenuBorder();
    std::cout << "Enter choice: ";
}

// Checks if file is a valid CSV file
bool checkCSV(std::string file) {
    std::string fileToCheck = file;
    std::string checkForCSV = ".csv";
    int result = fileToCheck.find(checkForCSV);

    // Returns true if file is a CSV
    if (result != std::string::npos) {
        return(1);
    }

    // Returns false if file is not a CSV
    else {
        return(0);
    }
}

int main() {
    // Displays all output in green for better contrast to improve visibility
    std::cout << "\033[32m";

    // Add delete and update options

    // Defines a binary search tree to hold all courses
    BinarySearchTree* bst = new BinarySearchTree();
    Course course;
    std::string userFileInput;
    std::string relativeFilePath;
    std::string courseToFind;

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
            std::cout << "Invalid input, please enter 1, 2, 3, or 9." 
                        << std::endl;
        }

        switch (choice) {

            // Case 1 loads CSV file parsed by user
            case 1:                
                std::cout << "Please specify CSV file name." << std::endl;

                std::cin >> userFileInput;

                relativeFilePath = userFileInput;

                // Checks if input is a CSV file
                if (checkCSV(relativeFilePath)) {
                    loadCSV(relativeFilePath, bst);
                }

                else {
                    std::cout << "File is not a CSV." << std::endl << std::endl;
                }
                break;

            // Case 2 prints all courses loaded from CSV file
            case 2:
                std::cout << "Course List and Prerequisites:" << std::endl;

                bst->InOrder();
                break;

            // Case 3 searches for and prints a user specified course 
            // and its prereqs, if any
            case 3:
                std::cout << "Please enter course ID:" << std::endl;

                std::cin >> courseToFind;

                std::cout << std::endl;

                course = bst->Search(courseToFind);

                if (!course.courseId.empty()) {
                    displayCourseInfo(course);
                }
                
                else {
                    std::cout << "Course Id " << courseToFind << " not found." << std::endl;
                }
                break;

            // Default case handles inappropriate input
            default:
                std::cout << "Please enter a valid option." << std::endl << std::endl;
        }
    }
    std::cout << "Good bye." << std::endl;

    return 0;
}
