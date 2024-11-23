import os

# Directory where the folders are located
root_dir = './REPO-AthOS'  # Replace with the root directory containing the folders

# Iterate through all subfolders
for root, dirs, files in os.walk(root_dir):
    for file in files:
        if file.endswith(".ini"):  # Only process .ini files
            ini_file_path = os.path.join(root, file)
            
            # Read the file content and check for issues
            with open(ini_file_path, 'r') as f:
                lines = f.readlines()
            
            # Check for lines that have a space after the backslash
            for line in lines:
                if ('-DCMAKE_INSTALL_DOCDIR' in line or '-DCMAKE_INSTALL_MANDIR' in line) and '\\ ' in line:
                    print(f"Issue found in: {ini_file_path}")
                    break  # Stop checking this file if issue is found, move to the next file
