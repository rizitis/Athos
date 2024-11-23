import os
import shutil

# Directory where the folders are located
root_dir = './REPO-AthOS'  # Replace with the root directory containing the folders
backup_dir = './REPO-AthOS.bak'  # Directory where backups will be stored

# Create a backup folder if it doesn't exist
if not os.path.exists(backup_dir):
    os.makedirs(backup_dir)

# Iterate through all subfolders
for root, dirs, files in os.walk(root_dir):
    for file in files:
        if file.endswith(".ini"):  # Only process .ini files
            ini_file_path = os.path.join(root, file)
            
            # Backup the original file
            backup_file_path = os.path.join(backup_dir, file)
            shutil.copy(ini_file_path, backup_file_path)
            
            # Read the file content
            with open(ini_file_path, 'r') as f:
                lines = f.readlines()
            
            # Write the corrected content back to the file
            with open(ini_file_path, 'w') as f:
                for line in lines:
                    # Fix lines with -DCMAKE_INSTALL_DOCDIR=/usr/doc \
                    # and -DCMAKE_INSTALL_MANDIR=/usr/man \
                    if '-DCMAKE_INSTALL_DOCDIR' in line or '-DCMAKE_INSTALL_MANDIR' in line:
                        # Remove the space after the backslash
                        line = line.replace('\\ ', '\\')
                    f.write(line)

print("Backup completed and fixing finished.")
