#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <vector>

using namespace std;

// Function to create a new file and write initial content
int Create_File()
{
    ofstream CreateFile("test.md");

    if (!CreateFile) 
    {
        cerr << "Error opening file for writing." << endl;
        return 1;
    }

    CreateFile << "# Hello, World!" << endl;
    CreateFile.close();
    return 0;
}


// Function to read the content of the file
int Read_File()
{
    ifstream OpenFile("test.md");

    if (!OpenFile) {
        cerr << "Error opening file for reading." << endl;
        return 1;
    }

    string line;
    while (getline(OpenFile, line)) {
        cout << line << endl;
    }

    OpenFile.close();
    return 0;
}

// Function to append content to the file
int Write_File()
{
    fstream WriteFile("test.md", ios::app);

    if (!WriteFile) {
        cerr << "Error opening file for appending." << endl;
        return 1;
    }

    // Prompt the user for content to append
    string line;
    cout << "Enter content to write to the file (type 'exit' to finish):" << endl;
    while (true) {
        getline(cin, line);
        if (line == "exit") 
        {
            break;
        }
        WriteFile << line << endl;
    }
    WriteFile.close();
    return 0;
}

// Function to edit a specific line in the file
int Edit_File()
{
    int choice;
    cout << "1. Append to file" << endl;
    cout << "2. Edit a file" << endl;
    cout << "3. Cancel" << endl;

    while (true)
    {
        cout << "Enter your choice (1-3): ";

        if (!(cin >> choice)) 
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid input. Please enter a number between 1 and 3." << endl;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer

        if (choice == 1) 
        {
            if (Write_File() == 0)
            {
                cout << "Content appended successfully." << endl;
            }
            return 0;
        } else if (choice == 2) 
        {
            ifstream InputFile("test.md");
            if (!InputFile) 
            {
                cerr << "Error opening file for reading." << endl;
                return 1;
            }

            vector<string> lines;
            string line;
            while (getline(InputFile, line)) 
            {
                lines.push_back(line);
            }

            InputFile.close();

            if (lines.empty()) 
            {
                cout << "The file is empty." << endl;
                return 0;
            }

            cout << "File content:" << endl;
            for (size_t i = 0; i < lines.size(); ++i) 
            {
                cout << i + 1 << ": " << lines[i] << endl;
            }

            size_t lineNumber;
            cout << "Enter the line number to edit (0 to cancel): ";
            if (!(cin >> lineNumber) || lineNumber > lines.size()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid line number." << endl;
                return 1;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (lineNumber == 0) {
                return 0;
            }

            cout << "Enter the replacement text: ";
            if (!getline(cin, lines[lineNumber - 1])) 
            {
                cerr << "Failed to read replacement text." << endl;
                return 1;
            }

            ofstream OutputFile("test.md");
            if (!OutputFile) 
            {
                cerr << "Error opening file for writing." << endl;
                return 1;
            }

            for (const string& editedLine : lines) 
            {
                OutputFile << editedLine << endl;
            }
            if (!OutputFile) {
                cerr << "Error writing to file." << endl;
                return 1;
            }

            cout << "File updated successfully." << endl;          
            return 0; // Proceed to edit a line

        } else if (choice == 3) 
        {
            return 0; // Cancel operation
        } else 
        {
            cerr << "Invalid choice. Please enter 1, 2, or 3: ";
        }
    }
}

// Function to delete the file
int Delete_File()
{
    if (remove("test.md") != 0) {
        cerr << "Error deleting file." << endl;
        return 1;
    }
    return 0;
}

// Main function to demonstrate file operations
int main()
{
    int choice;
    cout << "This is a simple file editor." << endl;
    cout << "Choose an option:" << endl;
    cout << "1. Create File" << endl;
    cout << "2. Read File" << endl;
    cout << "3. Edit a File" << endl;
    cout << "4. Delete File" << endl;
    cout << "5. Exit" << endl;

    while(true)
    {
        cout << "Enter your choice (1-5): ";

        if (!(cin >> choice)) 
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid input. Please enter a number between 1 and 5." << endl;
            continue;
        }
        
        switch (choice) 
        {
            case 1:
                if (Create_File() == 0)
                {
                    cout << "File created successfully." << endl;
                }
                break;
            case 2:
                cout << "File content:" << endl;
                Read_File();
                break;
            case 3:
                Edit_File();
                break;
            case 4:
                if (Delete_File() == 0)
                {
                    cout << "File deleted successfully." << endl;
                }
                break;
            case 5:
                cout << "Exiting..." << endl;
                return 0;
            default:
                cerr << "Invalid choice." << endl;
        }
    }

    return 0;
}