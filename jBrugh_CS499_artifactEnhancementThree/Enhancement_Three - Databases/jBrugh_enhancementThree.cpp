/*
 * Jonathan Brugh
 * CS499 Capstone
 * Mr. K
 * Enhancement Two: Algorithms and Data Structure
 * August 09, 2026
 */

/*
 * This application recieves in a CSV of SNHU's courses and prerequisites
 * and stores the information in a mongoDB database w/ full CRUD operations.
 * It prints the infomation to a console.
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

// MongoDB
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/options/update.hpp>

// For building the MongoDB documents
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::array;

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

// Converts strings to uppercase.
std::string ToUpper(std::string word) {
    std::transform(word.begin(), word.end(), word.begin(), toupper);
    return word;
}

// Struct for holding course information
struct Course {
    std::string courseId = "";
    std::string courseTitle = "";
    std::vector<std::string> preReqs;
};

// This class holds the CRUD operations.
class CourseRepository {

    private:
        mongocxx::collection collection;

        void printCourseDocument(const bsoncxx::document::view& doc);

    public:
        explicit CourseRepository(mongocxx::collection coll) : 
                                    collection(std::move(coll)) {}
        
        void CreateUniqueIndex();
        bool DeleteCourse(std::string courseId);
        bool DisplayCourseInfo(std::string& courseId);
        void PrintAllCourses();
        void SeedDbFromCSV(std::string& csvPath);
        bool UpsertCourse(const Course& course);
};

// Builds a MongoDB document from a Course
bsoncxx::document::value courseToDocument(const Course& c) {
    array prereqArr;
    for (const auto& p : c.preReqs) {
        prereqArr.append(p);
    }

    document doc;
    doc.append(kvp("courseId", c.courseId));
    doc.append(kvp("courseTitle", c.courseTitle));
    doc.append(kvp("preReqs", prereqArr));

    return doc.extract();
}

void CourseRepository::CreateUniqueIndex() {
    try {
        mongocxx::options::index indexOptions{};
        indexOptions.unique(true);

        collection.create_index(
            bsoncxx::builder::stream::document{} << "courseId" << 1
                << bsoncxx::builder::stream::finalize,
            indexOptions
        );
    } catch (const mongocxx::exception& e) {
        std::cout << "Failed to create index: " << e.what() << std::endl;
    }
}

// Deletes a single course.
bool CourseRepository::DeleteCourse(std::string courseId) {

    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "courseId" << courseId
            << bsoncxx::builder::stream::finalize;

        auto result = collection.delete_one(filter.view());

        if (result && result->deleted_count() > 0) {
            std::cout << "Course " << courseId << " deleted." << std::endl;
            return true;
        }
        
        else {
            std::cout << "Course " << courseId << " not found." << std::endl;
            return false;
        }
    }
    
    catch (const mongocxx::exception& e) {
        std::cout << "Failed to delete course: " << e.what() << std::endl;
        return false;
    }
}

// prints individual course info
bool CourseRepository::DisplayCourseInfo(std::string& courseId) {

    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "courseId" << courseId
            << bsoncxx::builder::stream::finalize;

        auto result = collection.find_one(filter.view());

        if (!result) {
            std::cout << "Course Id " << courseId << " not found."
                    << std::endl;
            return false;
        }

        printCourseDocument(result->view());
        return true;
    }
    
    catch (const mongocxx::exception& e) {
        std::cout << "Failed to search for course: " << e.what() << std::endl;
        return false;
    }
}

// Prints entire database in alphanumeric order
void CourseRepository::PrintAllCourses() {
    
    try {
        mongocxx::options::find opts;
        opts.sort(bsoncxx::builder::stream::document{} << "courseId"
                << 1 << bsoncxx::builder::stream::finalize);

        auto cursor = collection.find({}, opts);

        int count = 0;
        for (auto&& doc : cursor) {
            printCourseDocument(doc);
            count++;
        }

        if (count == 0) {
            std::cout << "No courses found in the database." << std::endl;
        }
    }

    catch (const mongocxx::exception& e) {
        std::cout << "Failed to retrieve courses: " << e.what() << std::endl;
    }
}

// Helper method shared by displayCourseInfo and findAndPrintCourse.
// Prints a single course document in the standard 
// "id: title | Prerequisites: ..." format.
void CourseRepository::printCourseDocument(const bsoncxx::document::view& doc) {

    std::string courseId{doc["courseId"].get_string().value};
    std::string courseTitle{doc["courseTitle"].get_string().value};

    std::cout << courseId << ": " << courseTitle << " | Prerequisites: ";

    auto prereqArr = doc["preReqs"].get_array().value;
    size_t i = 0;
    size_t total = std::distance(prereqArr.begin(), prereqArr.end());

    for (auto&& p : prereqArr) {
        std::cout << p.get_string().value;
        if (++i < total) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
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

// Seeds the database from a CSV file.
// Not required, but allows a user to maintain 
// course data outside of the program.
void CourseRepository::SeedDbFromCSV(std::string& csvPath) {

    std::cout << "Loading CSV file, " << csvPath 
            << ", to seed the database.\n" << std::endl;

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

        std::cout << "Loading CSV file, " << csvPath 
                << ", to seed the database.\n" << std::endl;
    }

    // After 3 attempts are made, returns to main menu
    if (!file.is_open()) {
        std::cout << "Unable to open a valid CSV file after multiple attempts."
                    << std::endl;
        return;
    }

    // Maintenance variables for file
    std::vector<bsoncxx::document::value> docs;
    std::string line;
    std::string word;
    int wordCount;
    int rowCount = 0;

    // While file has a line to get
    while (getline(file, line)) {

        // Maintenance variables for each line
        Course course;
        std::stringstream str(line);

        // Used to find where prerequisites begin
        wordCount = 0;

        // First two inputs are courseId and courseTitle
        while (wordCount < 2) {

            getline(str, word, ',');
            word = sanitize(word);

            // Sets the first word as the courseId
            if (wordCount == 0) {
                course.courseId = ToUpper(word);
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

        // Pushes documents into a vector before adding to database
        docs.push_back(courseToDocument(course));
        rowCount++;
    }

    // Closes the open file
    file.close();

    if (docs.empty()) {
        std::cout << "No data found to seed." << std::endl;
        return;
    }

    // Skips inserting existing courses to avoid duplicates
    mongocxx::options::insert insertOpts{};
    insertOpts.ordered(false);

    // Tries to insert all documents from the docs vector
    try {
        auto result = collection.insert_many(docs, insertOpts);
        std::cout << "Seeded " << result->inserted_count()
                << " of " << rowCount << " course(s) into database." 
                << std::endl;
    }

    catch (const mongocxx::exception& e) {
        std::cout << "Database seeding failed: " << e.what() << std::endl;
    }
}

// Inserts or updates a course.
bool CourseRepository::UpsertCourse(const Course& course) {

    try {
        auto filter = bsoncxx::builder::stream::document{}
            << "courseId" << course.courseId
            << bsoncxx::builder::stream::finalize;

        mongocxx::options::replace replaceOpts{};
        replaceOpts.upsert(true);

        auto result = collection.
                        replace_one(filter.view(),
                                    courseToDocument(course), replaceOpts);

        if (result->upserted_id()) {
            std::cout << "Course " << course.courseId << " added." << std::endl;
        }
        
        else if (result->modified_count() > 0) {
            std::cout << "Course " << course.courseId << " updated." << std::endl;
        }
        
        else {
            std::cout << "Course " << course.courseId
                        << " already up to date (no changes)." << std::endl;
        }
        return true;
    }
    
    catch (const mongocxx::exception& e) {
        std::cout << "Failed to save course: " << e.what() << std::endl;
        return false;
    }
}

// Creates a line of 32 '*', used for top and bottom menu borders
void topBottomMenuBorder() {
    std::cout << std::setfill('*') << std::setw(32) << "" << std::endl;
}

// Prints the main user menu
void userMenu() {
    topBottomMenuBorder();
    std::cout << "Menu:" << std::endl;
    std::cout << "  1. Seed Database With Courses From CSV" << std::endl;
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
    // Creating a local instance of MongoDB
    mongocxx::instance instance{};
    mongocxx::uri uri("mongodb://localhost:27017"); 
    mongocxx::client client(uri);

    // Creates snhu_courses database w/ courses collection
    CourseRepository courseRepo(client["snhu_courses"]["courses"]);

    // Connectivity check
    try {
        auto ping = bsoncxx::builder::stream::document{} 
            << "ping" << 1 
            << bsoncxx::builder::stream::finalize;
        client["snhu_courses"].run_command(ping.view());
        std::cout << "Connected to MongoDB." << std::endl;
    }
    
    catch (const std::exception& e) {
        std::cout << "MongoDB connection failed: " << e.what() << std::endl;
    }

    // Ensures courseId is unique in the collection
    courseRepo.CreateUniqueIndex();

    // Displays all output in green for better contrast to improve visibility
    std::string greenTextOutput = "\033[32m";
    std::cout << greenTextOutput;

    int choice = 0;
    while (choice != 9) {

        userMenu();
        
        std::cin >> choice;
        std::cout << std::endl;
        
        // Handles alphabetic character entry 
        if (std::cin.fail()) {

            std::cin.clear();

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                       '\n');

            // Ask the user to enter a valid int only
            std::cout << "Invalid input, please enter 1, 2, 3, 4, 5, or 9." 
                        << std::endl;
        }

        switch (choice) {

            // Case 1 seeds database from CSV file parsed by user.
            // Does not allow duplicate entries.
            case 1:
            {
                std::string userFileInput;

                std::cout << "Please specify CSV file name." << std::endl;

                std::cin >> userFileInput;

                // Checks if input is a CSV file and safe to open
                if (isValidCsvPath(userFileInput)) {
                    courseRepo.SeedDbFromCSV(userFileInput);
                }

                else {
                    std::cout << "File is not a valid, safe CSV path."
                            << std::endl << std::endl;
                }
            }
                break;

            // Case 2 prints all courses in database.
            case 2:
            {
                std::cout << "Course List and Prerequisites:" << std::endl;

                courseRepo.PrintAllCourses();
            }
                break;

            // Case 3 searches for and prints a user specified course 
            // and its prereqs, if any.
            case 3:
            {
                std::string courseIdToFind;

                std::cout << "Please enter course ID:" << std::endl;

                std::cin >> courseIdToFind;
                courseIdToFind = sanitize(courseIdToFind);
                courseIdToFind = ToUpper(courseIdToFind);

                std::cout << std::endl;

                courseRepo.DisplayCourseInfo(courseIdToFind);
            }
                break;

            // Case 4 inserts a new course into the database.
            // Also used to update existing courses in database.
            case 4:
            {
                Course course;
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
                courseRepo.UpsertCourse(course);
            }
                break;

            case 5:
            {
                std::string deleteCourseID;
                std::cout << "Enter courseID to delete:" << std::endl;
                std::cin >> deleteCourseID;
                deleteCourseID = sanitize(deleteCourseID);
                deleteCourseID = ToUpper(deleteCourseID);

                courseRepo.DeleteCourse(deleteCourseID);
            }
                break;

            case 9:
            {
                std::cout << "Goodbye." << std::endl;
            }
                break;

            // Default case handles inappropriate input
            default:
            {
                std::cout << "Please enter a valid option." << std::endl 
                        << std::endl;
            }
        }
    }
    // Reverts to default terminal text style
    std::string defaultText = "\033[0m";
    std::cout << defaultText;
    return 0;
}
