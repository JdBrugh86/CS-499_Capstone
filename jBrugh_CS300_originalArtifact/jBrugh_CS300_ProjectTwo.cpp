/**
 * Jonathan Brugh
 * CS300 DSA: Analysis and Design
 * Dr. Thoma
 * Project Two
 * April 20, 2025
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

using namespace std;

struct Course {
    string courseId;
    string courseTitle;
    vector<string> preReqs;
};

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

class BinarySearchTree {

    private:
        Node* root;
    
        void addNode(Node* node, Course course);
        void deleteBST(Node* node);
        void inOrder(Node* node);
        void postOrder(Node* node);
        void preOrder(Node* node);
        Node* removeNode(Node* node, string courseId);
    
    public:
        BinarySearchTree();
        virtual ~BinarySearchTree();
        void InOrder();
        void PostOrder();
        void PreOrder();
        void Insert(Course course);
        Course Search(string courseId);
    };

/**
 * Default constructor
 */
BinarySearchTree::BinarySearchTree() {
    // initialize root of BST
	root = nullptr;
}

/**
 * Destructor
 */
BinarySearchTree::~BinarySearchTree() {
    // recurse from root deleting every node
	deleteBST(root);
}

void BinarySearchTree::deleteBST(Node* node) {
    // recursively deletes nodes from bottom of BST
    if (node != nullptr) {
	
        // traverse left subtree
        deleteBST(node->left);

        // traverse right subtree
        deleteBST(node->right);

        // deletes node
        delete node;
		
		// address dangling pointer
		node = nullptr;
    }
}

/**
 * Traverse the tree in order
 */
void BinarySearchTree::InOrder() {
    // print in alphanumeric order from root
	inOrder(root);
}

/**
 * Insert a bid
 */
void BinarySearchTree::Insert(Course course) {
    // inserting a course into the tree
	if (root == nullptr) {
		Node* newNode = new Node(course);
		root = newNode;
	}
	
    else {
		addNode(root, course);
	}
}

/**
 * Search for a course
 */
Course BinarySearchTree::Search(string courseId) {
    // start at root
	Node* curNode = root;
	Course course;

    // keep looping downwards until bottom reached or matching courseId found
	while (curNode != nullptr) {
	
		// if match found, return current course
		if (curNode->course.courseId == courseId) {
			course = curNode->course;
			return course;
		}
		
		// if courseId is smaller than current node then traverse left
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
 * Add a course to some node (recursive)
 *
 * @param node Current node in tree
 * @param Course course to be added
 */
void BinarySearchTree::addNode(Node* node, Course course) {
    // inserting a course into the tree

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
			
			// call addNode: left node and bid
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

			// call addNode: right node and bid
			addNode(node, course);
		}
    }
}

/**
 * prints entire tree in alphanumeric order
 */
void BinarySearchTree::inOrder(Node* node) {
    
    if (node != nullptr) {
	
        // traverse left subtree
        inOrder(node->left);

        // print node courses
        cout << node->course.courseId << ": " << node->course.courseTitle << " | Prerequisites: ";
        if (!node->course.preReqs.empty()) {
            for (string i : node->course.preReqs) {
                if (i == node->course.preReqs.back()) {
                    cout << i;
                }
                
                else {
                    cout << i << ", ";
                }
            }
        }

        else {
            cout << "None.";
        }

        cout << endl;

        // traverse right subtree
        inOrder(node->right);
    }
}

/** 
 * loads csv given:
 *  @param path to file of courses
 *  @param BST tree to add course files to
 */
void loadCSV(string csvPath, BinarySearchTree* bst) {
    cout << "Loading CSV file " << csvPath << endl;

    ifstream file;
    file.open(csvPath);

    try{
        // maintenance variables file
        string line;
        string word;
        int wordCount;

        // while file has a line to get
        while (getline(file, line)) {

            // maintenance variables for each line
            Course course;
            stringstream str(line);

            // used to find where prerequisites begin
            wordCount = 0;

            // first two inputs are courseId and courseTitle
            while (wordCount < 2) {
                getline(str, word, ',');

                if (wordCount == 0) {
                    course.courseId = word;
                }

                else {
                    course.courseTitle = word;
                }
                
                wordCount++;
            }

            // after the second input, remaining inputs are prerequisites
            while (getline(str, word, ',')) {
                
                course.preReqs.push_back(word);
            }

            // add course to bst
            bst->Insert(course);
        }        
    } catch (ifstream::failure &e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * prints individual course info
*/
void displayCourseInfo(Course aCourse) {
    Course course = aCourse;
    cout << course.courseId << ": " << course.courseTitle << " | Prerequisites: ";
    
    // print prerequisites, if there are any
    if (!course.preReqs.empty()) {
        for (string i : course.preReqs) {

            // if last element in preReqs, print without a comma
            if (i == course.preReqs.back()) {
                cout << i;
            }
            
            // if not last element in preReqs, print with comma
            else {
                cout << i << ", ";
            }
        }
    }

    // if no preReqs, print None.
    // Fixme 1: Nonfunctional. Believed to be how the CSV is formatted.
    else {
        cout << "None.";
    }
    cout << endl;
}

/**
 * The one and only main() method
 */
int main() {

    // Define a binary search tree to hold all bids
    BinarySearchTree* bst;
    bst = new BinarySearchTree();
    Course course;
    string filePath;
    string courseToFind;

    int choice = 0;
    while (choice != 9) {
        cout << "Menu:" << endl;
        cout << "  1. Load Courses" << endl;
        cout << "  2. Display All Courses" << endl;
        cout << "  3. Display Course Information" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        
        /** Error code below found on geeksforgeeks:
         * https://www.geeksforgeeks.org/
         * how-to-handle-wrong-data-type-input-in-cpp/
         * 
         * Ensures only numbers are valid user inputs.
         */
        if (cin.fail()) {
            // Clear the error flags on the input stream.
            cin.clear();

            // leave the rest of the line
            cin.ignore(numeric_limits<streamsize>::max(),
                       '\n');

            // Ask the user to enter a valid int number only
            cout << "Invalid input, please enter 1, 2, 3, or 9." << endl;
        }

        switch (choice) {

            case 1:
                
                cout << "Please specify file path." << endl;

                cin >> filePath;

                loadCSV(filePath, bst);
                break;

            case 2:

                bst->InOrder();
                break;

            case 3:

                cout << "Please enter course ID" << endl;

                cin >> courseToFind;

                course = bst->Search(courseToFind);

                if (!course.courseId.empty()) {
                    displayCourseInfo(course);
                }
                
                else {
                    cout << "Course Id " << courseToFind << " not found." << endl;
                }
                break;
        }
    }

    cout << "Good bye." << endl;

    return 0;
}
