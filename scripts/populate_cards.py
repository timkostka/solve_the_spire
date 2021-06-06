"""
This script populates code within the PopulateCards function.

This scans the card definitions for all cards and ensures they are all added
to the index.

"""

import os

# relative path to card definition file
card_definition_path = "../SolveTheSpire/card.hpp"

# relative path to file containing PopulateCards function
function_path = "../SolveTheSpire/SolveTheSpire.cpp"

backup_function_path = "../SolveTheSpire/SolveTheSpire.backup_cpp"

# beginning line of PopulateCards function
function_starting_line = "void PopulateCards() {\n"

# beginning line of PopulateCards function
function_ending_line = "}\n"

# card definition prefix
card_definition_prefix = "const Card "

# card definition suffix
card_definition_suffix = " = "


if __name__ == "__main__":
    if not os.path.isfile(card_definition_path):
        print("ERROR: could not find card definition file")
        exit(1)
    if not os.path.isfile(function_path):
        print("ERROR: could not find function file")
        exit(1)
    # find variable names for all card definitions
    with open(card_definition_path) as f:
        lines = f.readlines()
    lines = [x[len(card_definition_prefix):]
             for x in lines
             if x.startswith(card_definition_prefix)]
    cards = [x[:x.index(card_definition_suffix)]
             for x in lines
             if card_definition_suffix in x]
    print(f"Found {len(cards)} card definitions")
    # now change the function definition
    with open(function_path) as f:
        lines = f.readlines()
    if function_starting_line not in lines:
        print("ERROR: could not start of PopulateCards definition")
        exit(1)
    before_lines = lines[:lines.index(function_starting_line) + 1]
    after_lines = lines[lines.index(function_starting_line):]
    if function_ending_line not in after_lines:
        print("ERROR: could not start of PopulateCards definition")
        exit(1)
    after_lines = after_lines[after_lines.index(function_ending_line):]
    middle_lines = ''.join('    %s.GetIndex();\n' % x for x in cards)
    # rename file
    os.rename(function_path, backup_function_path)
    # replace file
    with open(function_path, 'w') as f:
        f.write(''.join(before_lines))
        f.write(''.join(middle_lines))
        f.write(''.join(after_lines))
    print("File has been updated")
    print(f"Backup saved to {backup_function_path}")
