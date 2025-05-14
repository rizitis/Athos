import os

# Define the directories and patterns to search for
directory = '/usr/include/linux'
patterns = ['__u64', 'uint64_t', 'long long']

# Run the search and return the results
search_results = {pattern: [] for pattern in patterns}

for pattern in patterns:
    # Search for each pattern in the directory
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.h'):  # Only interested in header files
                with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as f:
                    lines = f.readlines()
                    for line_num, line in enumerate(lines):
                        if pattern in line:
                            search_results[pattern].append({
                                'file': os.path.join(root, file),
                                'line': line_num + 1,
                                'text': line.strip()
                            })

# Print results in terminal
for pattern, matches in search_results.items():
    print(f"Matches for pattern: {pattern}")
    if matches:
        for match in matches:
            print(f"File: {match['file']}, Line: {match['line']}, Text: {match['text']}")
    else:
        print("No matches found.")
    print('-' * 50)
